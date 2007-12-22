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

#include <dlfcn.h>
#include <stdlib.h>

#include <qstring.h>
#include <qptrvector.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qdir.h>
#include <qdom.h>

#include "common/plugin.h"
#include "common/outputplugin.h"
#include "dummyoutplugin.h"

#include "app.h"
#include "dmxmap.h"
#include "dmxmapeditor.h"

extern App* _app;

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

bool DMXPatch::loader(QDomDocument* doc, QDomElement* root, DMXMap* dmxMap)
{
	OutputPlugin* plugin = NULL;
	QDomNode node;
	QDomElement tag;
	QString str;
	QString pluginName;
	int output = 0;
	int universe = 0;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(dmxMap != NULL);

	if (root->tagName() != KXMLQLCDMXPatch)
	{
		qWarning("Patch node not found!");
		return false;
	}

	/* QLC universe that this patch has been made for */
	universe = root->attribute(KXMLQLCDMXPatchUniverse).toInt();
	universe = CLAMP(universe, 0, KUniverseCount);

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
			qWarning("Unknown Patch tag: %s",
				 (const char*) tag.tagName());
		
		node = node.nextSibling();
	}

	dmxMap->setPatch(universe, pluginName, output);

	return true;
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
	loadPlugins(QString(PLUGINS));
}

DMXMap::~DMXMap()
{
	delete m_dummyOut;
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
		qDebug("Unable to get value. Invalid universe %d", universe);
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
	t_value value = 0;

	if (universe >= m_universes)
	{
		qDebug("Unable to get values. Invalid universe %d", universe);
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
		qDebug("Unable to set value. Invalid universe %d", universe);
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
	t_value value = 0;

	if (universe >= m_universes)
	{
		qDebug("Unable to set values. Invalid universe %d", universe);
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
 * Editor UI
 *****************************************************************************/

void DMXMap::openEditor(QWidget* parent)
{
	DMXMapEditor* editor = NULL;

	editor = new DMXMapEditor(parent, this);
	editor->init();
	editor->exec();
	delete editor;
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
	appendPlugin(m_dummyOut);

	m_patch.resize(m_universes);

	for (i = 0; i < m_universes; i++)
	{
		/* The dummy output plugin provides always as many outputs
		   as QLC has supported universes. So, assign each of these
		   outputs, by default, to each universe */
		dmxPatch = new DMXPatch(m_dummyOut, i);
		m_patch.insert(i, dmxPatch);
	}
}

bool DMXMap::setPatch(int universe, const QString& pluginName,
		      int pluginUniverse)
{
	OutputPlugin* outputPlugin = NULL;
	DMXPatch* dmxPatch = NULL;

	if (universe < 0 || universe > m_patch.size())
	{
		qDebug("Unable to patch universe %d. Value is out of bounds.",
			universe);
		return false;
	}

	outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
	{
		qDebug("Unable to patch universe %d. Plugin %s not found.",
		       universe, (const char*) pluginName);
		return false;
	}
	else
	{
		/* Get rid of the old patch */
		delete m_patch[universe];

		/* Create a new patch */
		dmxPatch = new DMXPatch(outputPlugin, pluginUniverse);
		m_patch.insert(universe, dmxPatch);
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
	QPtrListIterator<OutputPlugin> it(m_plugins);
	QStringList list;

	while (it.current() != NULL)
	{
		list.append((*it)->name());
		++it;
	}	
	
	return list;
}

int DMXMap::pluginOutputs(const QString& pluginName)
{
	OutputPlugin* op = NULL;

	op = plugin(pluginName);
	if (op == NULL)
		return 0;
	else
		return op->outputs();
}

void DMXMap::configurePlugin(const QString& pluginName)
{
	OutputPlugin* outputPlugin = plugin(pluginName);
	if (outputPlugin == NULL)
		QMessageBox::warning(_app,
				     "Unable to configure plugin",
				     pluginName + " not found!");
	else
		outputPlugin->configure(_app);
}

QString DMXMap::pluginStatus(const QString& pluginName)
{
	OutputPlugin* outputPlugin = NULL;
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
		info += _app->colorGroup().highlight().name();
		info += QString("\">");
		info += QString("<FONT COLOR=\"");
		info += _app->colorGroup().highlightedText().name();
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
				info += _app->colorGroup().midlight().name();
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

void DMXMap::loadPlugins(const QString& pluginPath)
{
	OutputPlugin* outputPlugin = NULL;

	QDir pluginDir(pluginPath);
	pluginDir.setFilter(QDir::Files);
	pluginDir.setNameFilter("*.so");

	/* Check that we can access the directory */
	if (pluginDir.exists() == false)
	{
		qDebug(pluginPath + QString(" doesn't exist"));
		return;
	}
	else if (pluginDir.isReadable() == false)
	{
		qDebug(pluginPath + QString(" is not accessible"));
		return;
	}

	/* Loop thru all files in the directory */
	QStringList dirlist(pluginDir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		outputPlugin = createPlugin(pluginPath + QString("/") + *it);
		if (outputPlugin != NULL && appendPlugin(outputPlugin) == true)
		{
			qDebug("DMX output available thru %s (%d outputs)",
			       (const char*) outputPlugin->name(),
			       outputPlugin->outputs());
		}
		else
		{
			qDebug("%s doesn't contain an output plugin\n",
			       (const char*) (pluginPath + *it));
		}
	}
}

OutputPlugin* DMXMap::createPlugin(const QString& path)
{
	QLCPluginCreateFunction create;

	void* pluginHandle = NULL;
	Plugin* plugin = NULL;

	/* Load the (presumed) shared object into memory. Don't resolve
	   symbols until they're needed */
	pluginHandle = ::dlopen(static_cast<const char*> (path), RTLD_LAZY);
	if (pluginHandle == NULL)
	{
		qDebug("Unable to open %s with dlopen(): %s\n", 
		       (const char*) path, ::dlerror());
		return NULL;
	}

	/* Attempt to resolve "create" symbol from the shared object */
	create = (QLCPluginCreateFunction) ::dlsym(pluginHandle, "create");
	if (create == NULL)
	{
		::dlclose(pluginHandle);
		qDebug("Unable to resolve symbols for %s. dlsym(): %s", 
		       (const char*) path, ::dlerror());
		return NULL;
	}

	/* Attempt to use the "create" symbol to create a Plugin instance */
	plugin = create();
	Q_ASSERT(plugin != NULL);

	/* We accept only output plugins here */
	if (plugin->type() != Plugin::OutputType)
	{
		delete plugin;
		dlclose(pluginHandle);
		return NULL;
	}

	/* Setup the plugin */
	plugin->setHandle(pluginHandle);

	return static_cast<OutputPlugin*> (plugin);
}

bool DMXMap::appendPlugin(OutputPlugin* outputPlugin)
{
	Q_ASSERT(outputPlugin);

	if (plugin(outputPlugin->name()) == NULL)
	{
		m_plugins.append(outputPlugin);
		return true;
	}
	else
	{
		qDebug(outputPlugin->name() + " is already loaded!");
		return false;
	}
}

OutputPlugin* DMXMap::plugin(const QString& name)
{
	QPtrListIterator<OutputPlugin> it(m_plugins);

	while ( it.current() != NULL )
	{
		if (it.current()->name() == name)
			return it.current();
		++it;
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
	for (i = 0; i < m_patch.count(); i++)
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
		qWarning("DMXMap node not found!");
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
			qWarning("Unknown DMXMap tag: %s",
				 (const char*) tag.tagName());
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void DMXMap::loadDefaults(const QString& path)
{
	QSettings settings;
	QString key;
	QString plugin;
	QString output;

	settings.setPath("qlc.sf.net", "qlc");

	for (int i = 0; i < KUniverseCount; i++)
	{
		/* Plugin name */
		key.sprintf("%s/dmxmap/universe%d/plugin/",
			    (const char*) path, i);
		plugin = settings.readEntry(key);

		/* Plugin output */
		key.sprintf("%s/dmxmap/universe%d/output/",
			    (const char*) path, i);
		output = settings.readEntry(key);
		
		if (plugin.length() > 0 && output.length() > 0)
			setPatch(i, plugin, output.toInt());
	}
}

void DMXMap::saveDefaults(const QString& path)
{
	DMXPatch* dmxPatch = NULL;
	QSettings settings;
	QString key;
	QString str;

	settings.setPath("qlc.sf.net", "qlc");

	for (int i = 0; i < KUniverseCount; i++)
	{
		dmxPatch = patch(i);
		Q_ASSERT(dmxPatch != NULL);
		Q_ASSERT(dmxPatch->plugin != NULL);

		/* Plugin name */
		key.sprintf("%s/dmxmap/universe%d/plugin/",
			    (const char*) path, i);
		settings.writeEntry(key, dmxPatch->plugin->name());

		/* Plugin output */
		key.sprintf("%s/dmxmap/universe%d/output/",
			    (const char*) path, i);
		settings.writeEntry(key, str.setNum(dmxPatch->output));
	}
}
