/*
  Q Light Controller
  outputmap.cpp
  
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
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "app.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputMap::OutputMap(QObject* parent, int universes) : QObject(parent)
{
	m_universes = universes;
	m_dummyOut = NULL;

	m_blackout = false;
	m_blackoutStore = NULL;

	initPatch();

	load();
	loadDefaults();
}

OutputMap::~OutputMap()
{
	for (int i = 0; i < m_universes; i++)
		delete m_patch[i];

	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();

	m_dummyOut = NULL;
}

void OutputMap::load()
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

bool OutputMap::toggleBlackout()
{
	if (m_blackout == true)
		setBlackout(false);
	else
		setBlackout(true);

	return m_blackout;
}

void OutputMap::setBlackout(bool state)
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
		   OutputMap::setValueRange() doesn't write the zeros to
		   m_blackoutstore */
		m_blackout = true;
	}
	else
	{
		Q_ASSERT(m_blackoutStore != NULL);

		/* Toggle blackout off BEFORE the operation so that
		   OutputMap::setValueRange() writes the blackoutstore contents
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

bool OutputMap::blackout()
{
	return m_blackout;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

t_value OutputMap::getValue(t_channel channel)
{
	OutputPatch* outputPatch;
	t_channel universe;
	t_value value = 0;
	
	Q_ASSERT(channel < (m_universes * 512));

	if (m_blackout == true)
		return m_blackoutStore[channel];

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	universe = static_cast<t_channel> (channel / 512);
	if (universe >= m_universes)
	{
		qWarning() << "Unable to get value. Invalid universe number:"
			   << universe;
		return 0;
	}

	/* Get the plugin that is assigned to this universe */
	outputPatch = m_patch[universe];
	Q_ASSERT(outputPatch != NULL);
	Q_ASSERT(outputPatch->plugin() != NULL);

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	outputPatch->plugin()->readChannel(outputPatch->output(),
					   (channel % 512), &value);

	return value;
}

bool OutputMap::getValueRange(t_channel address, t_value* values, t_channel num)
{
	OutputPatch* outputPatch;
	t_channel universe;

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
	universe = static_cast<t_channel> (address / 512);
	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return 0;
	}

	/* Get the plugin that is assigned to this universe */
	outputPatch = m_patch[universe];
	Q_ASSERT(outputPatch != NULL);
	Q_ASSERT(outputPatch->plugin() != NULL);

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	outputPatch->plugin()->readRange(outputPatch->output(),
					 (address % 512), values, num);

	return true;
}

void OutputMap::setValue(t_channel channel, t_value value)
{
	OutputPatch* outputPatch;
	t_channel universe;

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
	universe = static_cast<t_channel> (channel / 512);
	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return;
	}

	/* Get the plugin that is assigned to this universe */
	outputPatch = m_patch[universe];
	Q_ASSERT(outputPatch != NULL);
	Q_ASSERT(outputPatch->plugin() != NULL);

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	outputPatch->plugin()->writeChannel(outputPatch->output(),
					    (channel % 512), value);
}

void OutputMap::setValueRange(t_channel address, t_value* values, t_channel num)
{
	OutputPatch* outputPatch;
	t_channel universe;

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
	universe = static_cast<t_channel> (address / 512);
	if (universe >= m_universes)
	{
		qWarning() << "Unable to set values. Invalid universe number:"
			   << universe;
		return;
	}

	/* Get the plugin that is assigned to this universe */
	outputPatch = m_patch[universe];
	Q_ASSERT(outputPatch != NULL);
	Q_ASSERT(outputPatch->plugin() != NULL);

	/* Isolate just the channel number (0-511) and remap it to
	   the universe output selected for this patch */
	outputPatch->plugin()->writeRange(outputPatch->output(),
					  (address % 512), values, num);

}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void OutputMap::initPatch()
{
	/* Create a dummy output plugin and put it to the plugins list */
	m_dummyOut = new DummyOutPlugin();
	m_dummyOut->init();
	appendPlugin(m_dummyOut);

	for (int i = 0; i < m_universes; i++)
	{
		OutputPatch* outputPatch;
		/* The dummy output plugin provides always as many outputs
		   as QLC has supported universes. So, assign each of these
		   outputs, by default, to each universe */
		outputPatch = new OutputPatch(this);
		outputPatch->set(m_dummyOut, i);
		m_patch.insert(i, outputPatch);
	}
}

bool OutputMap::setPatch(unsigned int universe, const QString& pluginName,
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
	
	m_patch[universe]->set(outputPlugin, output);

	return true;
}

OutputPatch* OutputMap::patch(int universe)
{
	Q_ASSERT(universe >= 0 && universe < KUniverseCount);
	return m_patch[universe];
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList OutputMap::pluginNames()
{
	QListIterator <QLCOutPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());
	
	return list;
}

QStringList OutputMap::pluginOutputs(const QString& pluginName)
{
	QLCOutPlugin* op = NULL;

	op = plugin(pluginName);
	if (op == NULL)
		return QStringList();
	else
		return op->outputs();
}

void OutputMap::configurePlugin(const QString& pluginName)
{
	QLCOutPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
		QMessageBox::warning(_app, tr("Unable to configure plugin"),
				     tr("%1 not found").arg(pluginName));
	else
		outputPlugin->configure();
}

QString OutputMap::pluginStatus(const QString& pluginName, t_output output)
{
	QLCOutPlugin* outputPlugin = NULL;
	QString info;

	if (pluginName != QString::null)
		outputPlugin = plugin(pluginName);

	if (outputPlugin != NULL)
		info = outputPlugin->infoText(output);
	else
		info = tr("No information");

	return info;
}

bool OutputMap::appendPlugin(QLCOutPlugin* outputPlugin)
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

QLCOutPlugin* OutputMap::plugin(const QString& name)
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

bool OutputMap::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* OutputMap entry */
	root = doc->createElement(KXMLQLCOutputMap);
	wksp_root->appendChild(root);

	/* Patches */
	for (i = 0; i < m_patch.size(); i++)
		m_patch[i]->saveXML(doc, &root, i);

	return true;
}

bool OutputMap::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCOutputMap)
	{
		qWarning() << "OutputMap node not found!";
		return false;
	}

	/* Patches */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCOutputPatch)
		{
			OutputPatch::loader(doc, &tag, this);
		}
		else
		{
			qWarning() << "Unknown OutputMap tag:" << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void OutputMap::loadDefaults()
{
	QSettings settings;
	QString plugin;
	QString output;
	QString key;

	for (int i = 0; i < KUniverseCount; i++)
	{
		/* Plugin name */
		key = QString("/outputmap/universe%2/plugin/").arg(i);
		plugin = settings.value(key).toString();

		/* Plugin output */
		key = QString("/outputmap/universe%2/output/").arg(i);
		output = settings.value(key).toString();

		if (plugin.length() > 0 && output.length() > 0)
			setPatch(i, plugin, output.toInt());
	}
}

void OutputMap::saveDefaults()
{
	QSettings settings;
	QString key;
	QString str;

	for (int i = 0; i < KUniverseCount; i++)
	{
		OutputPatch* outputPatch = patch(i);
		Q_ASSERT(outputPatch != NULL);
		Q_ASSERT(outputPatch->plugin() != NULL);

		/* Plugin name */
		key = QString("/outputmap/universe%2/plugin/").arg(i);
		settings.setValue(key, outputPatch->pluginName());

		/* Plugin output */
		key = QString("/outputmap/universe%2/output/").arg(i);
		settings.setValue(key, str.setNum(outputPatch->output()));
	}
}
