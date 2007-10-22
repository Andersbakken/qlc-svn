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
#include <qdir.h>

#include "common/plugin.h"
#include "common/outputplugin.h"
#include "dummyoutplugin.h"

#include "dmxmap.h"
#include "dmxmapeditor.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DMXMap::DMXMap(int universes)
{
	m_blackout = false;
	m_universes = universes;
	m_dummyOut = NULL;

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
	m_blackout = state;

	if (state == true)
	{
		/* TODO: Store real values somewhere during blackout */
		/* TODO: Write zeros to all universes */

		// t_value tmpValues[KChannelMax] = { 0 };
		// m_outputPlugin->writeRange(0, tmpValues, KChannelMax);

		// Below old stuff that was too slow for the USB2DMX code
		//      for (t_channel ch = 0; ch < KChannelMax; ch++)
		//	{
		//	  m_outputPlugin->writeChannel(ch, 0);
		//	}
	}
	else
	{
		/* TODO: Write real values to all universes */
	}
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
	if (m_blackout == true)
		return 0;

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (channel / 511);
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
	dmxPatch->plugin->readChannel((channel % 511) * 
				      (dmxPatch->output + 1), value);

	return value;
}

bool DMXMap::getValueRange(t_channel address, t_value* values, t_channel num)
{
	if (m_blackout == true)
	{
		for (int i = 0; i < num; i++)
			values[i] = 0;
		return true;
	}

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (address / 511);
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
	return dmxPatch->plugin->readRange((address % 511) *
					   (dmxPatch->output + 1), values, num);

}

void DMXMap::setValue(t_channel channel, t_value value)
{
	if (m_blackout == true)
		return;

	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (channel / 511);
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
	dmxPatch->plugin->writeChannel((channel % 511) * 
				       (dmxPatch->output + 1), value);
}

void DMXMap::setValueRange(t_channel address, t_value* values, t_channel num)
{
	if (m_blackout == true)
		return;
	
	/* Calculate universe from the channel number.
	   0-511 are universe 1, 512-1022 are universe 2... */
	t_channel universe = static_cast<t_channel> (address / 511);
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
	dmxPatch->plugin->writeRange((address % 511) * 
				     (dmxPatch->output + 1), values, num);

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
	m_dummyOut = new DummyOutPlugin(0);
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
		if (appendPlugin(outputPlugin) == true)
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
	typedef Plugin* create_t(int);
	create_t* create = NULL;

	void* pluginHandle = NULL;
	Plugin* plugin = NULL;

	/* Load the (presumed) shared object into memory. Don't resolve
	   symbols until they're needed */
	pluginHandle = ::dlopen(static_cast<const char*> (path), RTLD_LAZY);
	if (pluginHandle == NULL)
	{
		qDebug("Unable to open %s with dlopen(): %s\n", 
		       (const char*) path, dlerror());
		return NULL;
	}

	/* Attempt to resolve "create" symbol from the shared object */
	create = (create_t*) ::dlsym(pluginHandle, "create");
	if (create == NULL)
	{
		dlclose(pluginHandle);
		qDebug("Unable to resolve symbols for %s. dlsym(): %s", 
		       (const char*) path, dlerror());
		return NULL;
	}

	/* Attempt to use the "create" symbol to create a Plugin instance */
	plugin = create(0);
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
	plugin->setConfigDirectory(QString(getenv("HOME")) + QString("/.qlc/"));
	plugin->loadSettings();

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
