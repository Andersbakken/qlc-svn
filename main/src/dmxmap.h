/*
  Q Light Controller
  dmxmap.h
  
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

#ifndef DMXMAP_H
#define DMXMAP_H

#include <qptrvector.h>
#include <qptrlist.h>
#include "common/types.h"

class QString;
class OutputPlugin;
class DMXPatch;
class DMXMapEditor;

/*****************************************************************************
 * DMXPatch
 *****************************************************************************/

/**
 * This is a simple container class that stores only the pointer to an
 * existing plugin and an output line provided by that plugin.
 */
class DMXPatch
{
public:
	DMXPatch(OutputPlugin* p, int o) { plugin = p; output = o; }
	virtual ~DMXPatch() {}

	OutputPlugin* plugin;
	int output;
};

/*****************************************************************************
 * DMXMap
 *****************************************************************************/

class DMXMap
{
	friend class DMXMapEditor;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new DMXMap object
	 *
	 * @param universes Number of universes
	 * @param channels Number of channels per universe
	 */
	DMXMap(int universes = KUniverseCount);

	/**
	 * Destroy a DMXMap object
	 */
	~DMXMap();

protected:
	/** Total number of supported universes */
	int m_universes;

	/*********************************************************************
	 * Blackout
	 *********************************************************************/
public:
	/**
	 * Toggle blackout between on and off.
	 *
	 * @return New blackout state (i.e. after toggling)
	 */
	bool toggleBlackout();

	/**
	 * Set blackout on or off
	 *
	 * @param blackout If true, set blackout ON, otherwise OFF
	 */
	void setBlackout(bool state);

	/**
	 * Get blackout state
	 *
	 * @return true if blackout is ON, otherwise false
	 */
	bool blackout();

protected:
	/** Current blackout state */
	bool m_blackout;

	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	/**
	 * Get the value of one channel. Channels 0-511 are for the first
	 * universe, 512-1023 for the second etc..
	 *
	 * @param channel The channel whose value to get
	 * @return The value of the channel
	 */
	t_value getValue(t_channel channel);

	/**
	 * Get the value of a number of channels. Channels 0-511 are for the
	 * first universe, 512-1023 for the second etc..
	 *
	 * @param address The address of the first channel
	 * @param values An array that should contain the values
	 * @param num Size of values array
	 */
	bool getValueRange(t_channel address, t_value* values, t_channel num);

	/**
	 * Set the value of one channel. Channels 0-511 are for the first
	 * universe, 512-1023 for the second etc..
	 *
	 * @param channel The channel whose value to set
	 * @param value The value to set
	 */
	void setValue(t_channel channel, t_value value);

	/**
	 * Set the value of a number of channels. Channels 0-511 are for the
	 * first universe, 512-1023 for the second etc..
	 *
	 * @param address The address of the first channel
	 * @param values The values to set
	 * @param num Size of values array
	 */
	void setValueRange(t_channel address, t_value* values, t_channel num);

	/*********************************************************************
	 * Editor UI
	 *********************************************************************/
public:
	void openEditor(QWidget* parent);

	/*********************************************************************
	 * Patch
	 *********************************************************************/
public:
	/**
	 * Get the total number of supported universes
	 *
	 * @return Universe count supported by QLC
	 */
	int universes() { return m_universes; }

protected:
	/**
	 * Initialize the patching table
	 */
	void initPatch();

	/**
	 * Patch the given universe to go thru the given plugin
	 *
	 * @param universe The universe to patch
	 * @param pluginName The name of the plugin to patch to the universe
	 * @param pluginUniverse A universe provided by the plugin to patch to
	 * @return true if successful, otherwise false
	 */
	bool setPatch(int universe, const QString& pluginName,
		      int pluginUniverse = 0);

	/**
	 * Get the output mapping for a QLC universe.
	 *
	 * @param universe The internal universe to get mapping for
	 */
	DMXPatch* patch(int universe);

protected:
	/** Vector containing all active plugins */
	QPtrVector<DMXPatch> m_patch;

	/*********************************************************************
	 * Plugins
	 *********************************************************************/
public:
	/**
	 * Get a list of available DMX output plugins as a string list
	 * containing the plugins' names
	 *
	 * @return QStringList containing plugins' names
	 */
	QStringList pluginNames();

	/**
	 * Get the number of universes provided by the given plugin.
	 *
	 * @param pluginName Name of the plugin, whose output count to get
	 * @return Number of DMX outputs provided by the plugin.
	 *         Zero is returned if pluginName is invalid.
	 */
	int pluginOutputs(const QString& pluginName);

protected:
	/**
	 * Load output plugins from the given directory
	 *
	 * @param pluginPath A path pointing to QLC plugin directory
	 */
	void loadPlugins(const QString& pluginPath);
	
	/**
	 * Attempt to create a plugin from the given filename
	 *
	 * @param path A filename to a shared object plugin
	 */
	OutputPlugin* createPlugin(const QString& path);

	/**
	 * Append the given plugin to our list of plugins. Will fail if
	 * a plugin with the same name already exists.
	 *
	 * @param outputPlugin The output plugin to append
	 * @return true if successful, otherwise false
	 */
	bool appendPlugin(OutputPlugin* outputPlugin);

	/**
	 * Get a plugin instance by the plugin's name
	 *
	 * @param name The name of the plugin to search for
	 * @return OutputPlugin or NULL
	 */
	OutputPlugin* plugin(const QString& name);

protected:
	/** The dummy out plugin that is used for unused universes */
	OutputPlugin* m_dummyOut;

	/** List containing all available plugins */
	QPtrList<OutputPlugin> m_plugins;
};

#endif
