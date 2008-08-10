/*
  Q Light Controller
  dmxmap.cpp
  
  Copyright (c) Heikki Junnila
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QPluginLoader>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

#include "dummyoutplugin.h"
#include "dmxmapeditor.h"
#include "dmxmap.h"
#include "app.h"

extern App* _app;

using namespace std;

/*****************************************************************************
 * DMXPatch
 *****************************************************************************/

bool DMXPatch::saveXML(QDomDocument* doc, QDomElement* map_root, int universe)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(this->plugin != NULL);

	/* Patch entry */
	root = doc->createElement(KXMLQLCDMXPatch);
	map_root->appendChild(root);

	/* Universe */
	str.setNum(universe);
	root.setAttribute(KXMLQLCDMXPatchUniverse, str);

	/* Plugin */
	tag = doc->createElement(KXMLQLCDMXPatchPlugin);
	root.appendChild(tag);
	text = doc->createTextNode(this->plugin->name());
	tag.appendChild(text);

	/* Output */
	tag = doc->createElement(KXMLQLCDMXPatchOutput);
	root.appendChild(tag);
	str.setNum(this->output);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	return true;
}

bool DMXPatch::loader(QDomDocument*, QDomElement* root, DMXMap* dmxMap)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	QString pluginName;
	int output = 0;
	int universe = 0;
	
	Q_ASSERT(root != NULL);
	Q_ASSERT(dmxMap != NULL);

	if (root->tagName() != KXMLQLCDMXPatch)
	{
		qWarning() << "Patch node not found!";
		return false;
	}

	/* QLC universe that this patch has been made for */
	universe = root->attribute(KXMLQLCDMXPatchUniverse).toInt();

	/* Load patch contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCDMXPatchPlugin)
		{
			/* Plugin name */
			pluginName = tag.text();
		}
		else if (tag.tagName() == KXMLQLCDMXPatchOutput)
		{
			/* Plugin output */
			output = tag.text().toInt();
		}
		else
		{
			qWarning() << "Unknown Patch tag: " << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return dmxMap->setPatch(universe, pluginName, output);
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DMXMap::DMXMap(int universes) : QObject()
{
	m_universes = universes;
	m_dummyOut = NULL;

	m_blackout = false;
	m_blackoutStore = NULL;

	initPatch();

	load();
}

DMXMap::~DMXMap()
{
	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();

	m_dummyOut = NULL;
}

void DMXMap::load()
{
	QSettings s;
	
	QString pluginPath = s.value("directories/plugins").toString();
	if (pluginPath.isEmpty() == true)
	{
#ifdef WIN32
		pluginPath = "C:\\QLC\\Plugins";
#else
		pluginPath = "/usr/lib/qlc";
#endif
		s.setValue("directories/plugins", pluginPath);
	}

#ifdef WIN32
	QDir dir(pluginPath + "\\Output", "*.dll", QDir::Name, QDir::Files);
#else
	QDir dir(pluginPath + "/output", "*.so", QDir::Name, QDir::Files);
#endif

	/* Check that we can access the directory */
	if (dir.exists() == false)
	{
		qWarning() << "Output plugin path" << dir.absolutePath()
			   << "doesn't exist.";
		return;
	}
	else if (dir.isReadable() == false)
	{
		qWarning() << "Output plugin path" << dir.absolutePath()
			   << "is not accessible";
		return;
	}

	/* Loop thru all files in the directory */
	QStringList dirlist(dir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		QPluginLoader loader(dir.absoluteFilePath(*it), this);
		QLCOutPlugin* plugin;

		plugin = qobject_cast<QLCOutPlugin*> (loader.instance());
		if (plugin == NULL)
		{
			qWarning() << "Unable to find an output plugin in"
				   << dir.absoluteFilePath(*it);
		}
		else
		{
			plugin->init();
			appendPlugin(plugin);
		}
	}
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

bool DMXMap::toggleBlackout()
{
	if (m_blackout == true)
		setBlackout(false);
	else
		setBlackout(true);

	return m_blackout;
}

void DMXMap::setBlackout(bool state)
{
	/* Don't do blackout twice */
	if (m_blackout == state)
		return;

	if (state == true)
	{
		t_value zeros[512] = { 0 };

		Q_ASSERT(m_blackoutStore == NULL);
		m_blackoutStore = new t_value[m_universes * 512];

		/* Read the current values from all plugins */
		for (int i = 0; i < m_universes; i++)
		{
			/* Get the whole universe into the blackout store */
			getValueRange(i * 512,
				      m_blackoutStore + (i * 512),
				      512);
			
			/* Set all plugin outputs to zeros */
			setValueRange(i * 512, zeros, 512);
		}

		/* Set blackout AFTER the zero write operation so that
		   DMXMap::setValueRange() doesn't write the zeros to
		   m_blackoutstore */
		m_blackout = true;
	}
	else
	{
		Q_ASSERT(m_blackoutStore != NULL);

		/* Toggle blackout off BEFORE the operation so that
		   DMXMap::setValueRange() writes the blackoutstore contents
		   back to universes. */
		m_blackout = false;

		/* Write the values from the blackout store to all plugins */
		for (int i = 0; i < m_universes; i++)
		{
			/* Set values from the blackout store back to
			   plugin outputs */
			setValueRange(i * 512,
				      m_blackoutStore + (i * 512),
				      512);
		}

		delete [] m_blackoutStore;
		m_blackoutStore = NULL;
	}

	emit blackoutChanged(m_blackout);
}

bool DMXMap::blackout()
{
	return m_blackout;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

t_value DMXMap::getValue(t_channel channel)
{
	Q_ASSERT(channel < (m_universes * 512));

	if (m_blackout == true)
		return m_blackoutStore[channel];

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (channel / 512);
	DMXPatch* dmxPatch = NULL;
	t_value value = 0;

	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return 0;
	}

	/* Get the plugin that is assigned to this universe */
	dmxPatch = m_patch[universe];

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	dmxPatch->plugin->readChannel((channel % 512) +
				      (dmxPatch->output * 512), value);

	return value;
}

bool DMXMap::getValueRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address < (m_universes * 512));
	Q_ASSERT((address + num - 1) < (m_universes * 512));

	if (m_blackout == true)
	{
		/* Get the values from the temporary store when in blackout */
		memcpy(values, m_blackoutStore + address,
		       num * sizeof(t_value));
		return true;
	}

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (address / 512);
	DMXPatch* dmxPatch = NULL;

	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return 0;
	}

	/* Get the plugin that is assigned to this universe */
	dmxPatch = m_patch[universe];

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	return dmxPatch->plugin->readRange((address % 512) +
					   (dmxPatch->output * 512),
					   values, num);

}

void DMXMap::setValue(t_channel channel, t_value value)
{
	if (channel == KChannelInvalid)
		return;

	Q_ASSERT(channel < (m_universes * 512));

	if (m_blackout == true)
	{
		/* Just store the values when in blackout */
		m_blackoutStore[channel] = value;
		return;
	}

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (channel / 512);
	DMXPatch* dmxPatch = NULL;

	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return;
	}

	/* Get the plugin that is assigned to this universe */
	dmxPatch = m_patch[universe];

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	dmxPatch->plugin->writeChannel((channel % 512) + 
				       (dmxPatch->output * 512), value);
}

void DMXMap::setValueRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address < (m_universes * 512));
	Q_ASSERT((address + num - 1) < (m_universes * 512));

	if (m_blackout == true)
	{
		memcpy(m_blackoutStore + address, values, 
		       num * sizeof(t_value));
		return;
	}
	
	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (address / 512);
	DMXPatch* dmxPatch = NULL;

	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return;
	}

	/* Get the plugin that is assigned to this universe */
	dmxPatch = m_patch[universe];

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	dmxPatch->plugin->writeRange((address % 512) + 
				     (dmxPatch->output * 512), values, num);

}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void DMXMap::initPatch()
{
	DMXPatch* dmxPatch = NULL;
	int i = 0;

	/* Create a dummy output plugin and put it to the plugins list */
	m_dummyOut = new DummyOutPlugin();
	m_dummyOut->init();
	appendPlugin(m_dummyOut);

	for (i = 0; i < m_universes; i++)
	{
		/* The dummy output plugin provides always as many outputs
		   as QLC has supported universes. So, assign each of these
		   outputs, by default, to each universe */
		dmxPatch = new DMXPatch(m_dummyOut, i);
		m_patch.insert(i, dmxPatch);
	}
}

bool DMXMap::setPatch(unsigned int universe, const QString& pluginName,
		      unsigned int output)
{
	if (int(universe) >= m_patch.size())
	{
		qWarning() << "Universe number out of bounds:" << universe
			   << "Unable to set patch.";
		return false;
	}

	QLCOutPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
	{
		qWarning() << "Plugin" << pluginName << "for universe number"
			   << universe << "not found.";
		return false;
	}
	else
	{
		m_patch[universe]->plugin->close();
		m_patch[universe]->plugin = outputPlugin;
		m_patch[universe]->output = output;
		m_patch[universe]->plugin->open();

		return true;
	}
}

DMXPatch* DMXMap::patch(int universe)
{
	Q_ASSERT(universe >= 0 && universe < KUniverseCount);
	return m_patch[universe];
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList DMXMap::pluginNames()
{
	QListIterator <QLCOutPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());
	
	return list;
}

int DMXMap::pluginOutputs(const QString& pluginName)
{
	QLCOutPlugin* op = NULL;

	op = plugin(pluginName);
	if (op == NULL)
		return 0;
	else
		return op->outputs();
}

void DMXMap::configurePlugin(const QString& pluginName)
{
	QLCOutPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
		QMessageBox::warning(_app, tr("Unable to configure plugin"),
				     tr("%1 not found").arg(pluginName));
	else
		outputPlugin->configure();
}

QString DMXMap::pluginStatus(const QString& pluginName)
{
	QLCOutPlugin* outputPlugin = NULL;
	QString info;

	if (pluginName != QString::null)
		outputPlugin = plugin(pluginName);

	if (outputPlugin == NULL)
	{
		DMXPatch* dmxPatch = NULL;
		int i = 0;
		QString str;

		// HTML header
		info += QString("<HTML>");
		info += QString("<HEAD>");
		info += QString("<TITLE>Universe mapping status</TITLE>");
		info += QString("</HEAD>");
		info += QString("<BODY>");

		// Mapping status title
		info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
		info += QString("<TR>");
		info += QString("<TD BGCOLOR=\"");
		info += QApplication::palette()
			.color(QPalette::Highlight).name();
		info += QString("\">");
		info += QString("<FONT COLOR=\"");
		info += QApplication::palette()
			.color(QPalette::HighlightedText).name();
		info += QString("\" SIZE=\"5\">");
		info += QString("Universe mapping status");
		info += QString("</FONT>");
		info += QString("</TD>");
		info += QString("</TR>");
		info += QString("</TABLE>");

		// Universe mappings
		info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");

		for (i = 0; i < KUniverseCount; i++)
		{
			dmxPatch = patch(i);
			Q_ASSERT(dmxPatch != NULL && dmxPatch->plugin != NULL);

			if (i % 2 == 0)
				info += QString("<TR>");
			else
			{
				info += QString("<TR BGCOLOR=\"");
				info += QApplication::palette()
					.color(QPalette::Midlight).name();
				info += QString("\">");
			}

			info += QString("<TD>");
			str.setNum(i + 1);
			info += QString("<B>Universe " + str + "</B>");
			info += QString("</TD>");

			info += QString("<TD>");
			info += dmxPatch->plugin->name();
			info += QString("</TD>");

			info += QString("<TD>");
			str.sprintf("Output %d", dmxPatch->output + 1);
			info += str;
			info += QString("</TD>");

			info += QString("</TR>");
		}

		info += QString("</TABLE>");
	}
	else
	{
		info = outputPlugin->infoText();
	}

	return info;
}

bool DMXMap::appendPlugin(QLCOutPlugin* outputPlugin)
{
	Q_ASSERT(outputPlugin != NULL);

	if (plugin(outputPlugin->name()) == NULL)
	{
		qDebug() << "Found an output plugin:" << outputPlugin->name();
		m_plugins.append(outputPlugin);
		return true;
	}
	else
	{
		qWarning() << "Output plugin" << outputPlugin->name()
			   << "is already loaded. Skipping";
		return false;
	}
}

QLCOutPlugin* DMXMap::plugin(const QString& name)
{
	QListIterator <QLCOutPlugin*> it(m_plugins);

	while (it.hasNext() == true)
	{
		QLCOutPlugin* plugin = it.next();
		if (plugin->name() == name)
			return plugin;
	}

	return NULL;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool DMXMap::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* DMXMap entry */
	root = doc->createElement(KXMLQLCDMXMap);
	wksp_root->appendChild(root);

	/* Patches */
	for (i = 0; i < m_patch.size(); i++)
		m_patch[i]->saveXML(doc, &root, i);

	return true;
}

bool DMXMap::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCDMXMap)
	{
		qWarning() << "DMXMap node not found!";
		return false;
	}

	/* Patches */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCDMXPatch)
		{
			DMXPatch::loader(doc, &tag, this);
		}
		else
		{
			qWarning() << "Unknown DMXMap tag:" << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void DMXMap::loadDefaults()
{
	QSettings settings;
	QString plugin;
	QString output;
	QString key;

	for (int i = 0; i < KUniverseCount; i++)
	{
		/* Plugin name */
		key = QString("/dmxmap/universe%2/plugin/").arg(i);
		plugin = settings.value(key).toString();

		/* Plugin output */
		key = QString("/dmxmap/universe%2/output/").arg(i);
		output = settings.value(key).toString();

		if (plugin.length() > 0 && output.length() > 0)
			setPatch(i, plugin, output.toInt());
	}
}

void DMXMap::saveDefaults()
{
	QSettings settings;
	QString key;
	QString str;

	for (int i = 0; i < KUniverseCount; i++)
	{
		DMXPatch* dmxPatch = patch(i);
		Q_ASSERT(dmxPatch != NULL);
		Q_ASSERT(dmxPatch->plugin != NULL);

		/* Plugin name */
		key = QString("/dmxmap/universe%2/plugin/").arg(i);
		settings.setValue(key, dmxPatch->plugin->name());

		/* Plugin output */
		key = QString("/dmxmap/universe%2/output/").arg(i);
		settings.setValue(key, str.setNum(dmxPatch->output));
	}
}
