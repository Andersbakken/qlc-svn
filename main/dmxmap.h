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

#include <QObject>
#include <QVector>
#include <QList>
#include "common/qlctypes.h"

class QString;
class QDomDocument;
class QDomElement;

class QLCOutPlugin;
class DMXMap;
class DMXPatch;
class DMXMapEditor;
class DMXPatchEditor;

/*****************************************************************************
 * DMXPatch
 *****************************************************************************/

#define KXMLQLCDMXPatch "Patch"
#define KXMLQLCDMXPatchUniverse "Universe"
#define KXMLQLCDMXPatchPlugin "Plugin"
#define KXMLQLCDMXPatchOutput "Output"

/**
 * This is a simple container class that stores only the pointer to an
 * existing plugin and an output line provided by that plugin.
 */
class DMXPatch
{
	friend class DMXMap;
	friend class DMXMapEditor;
	friend class DMXPatchEditor;

public:
	DMXPatch(QLCOutPlugin* p, int o) { plugin = p; output = o; }
	virtual ~DMXPatch() {}

protected:
	/**
	 * Save a DMXPatch's properties into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param map_root An XML root node (DMXMap) to save under
	 * @param universe The internal universe number that the patch is
	 *                 addressed to
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* map_root, int universe);

	/**
	 * Create and load a DMXPatch's properties from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param root An XML node containing a DMXPatch to load from
	 * @param universe The universe number that the DMXPatch is addressed to
	 * @return true if successful, otherwise false
	 */
	static bool loader(QDomDocument* doc, QDomElement* root, DMXMap* dmxMap);

	QLCOutPlugin* plugin;
	int output;
};

/*****************************************************************************
 * DMXMap
 *****************************************************************************/

#define KXMLQLCDMXMap "DMXMap"

class DMXMap : public QObject
{
	Q_OBJECT

	friend class DMXPatch;
	friend class DMXMapEditor;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new DMXMap object
	 *
	 * @param universes Number of universes
	 */
	DMXMap(int universes = KUniverseCount);

	/**
	 * Destroy a DMXMap object
	 */
	~DMXMap();

	/**
	 * Load all output plugins from the plugin directory.
	 */
	void load();

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

signals:
	/**
	 * Signal that is sent when blackout state is changed. 
	 *
	 * @param state true if blackout has been turned on, otherwise false
	 */
	void blackoutChanged(bool state);

protected:
	/** Current blackout state */
	bool m_blackout;

	/** A temp place to store values to during blackout */
	t_value* m_blackoutStore;

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
	 * You should limit the range to one universe. For example, don't
	 * try to get values for channels 510-515. In any case, there are no
	 * such fixtures in the world that could exist in two universes.
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
	 * You should limit the range to one universe. For example, don't
	 * try to set values for channels 510-515. In any case, there are no
	 * such fixtures in the world that could exist in two universes.
	 *
	 * @param address The address of the first channel
	 * @param values The values to set
	 * @param num Size of values array
	 */
	void setValueRange(t_channel address, t_value* values, t_channel num);

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
	 * @param output A universe provided by the plugin to patch to
	 * @return true if successful, otherwise false
	 */
	bool setPatch(unsigned int universe, const QString& pluginName,
		      unsigned int output = 0);

	/**
	 * Get the output mapping for a QLC universe.
	 *
	 * @param universe The internal universe to get mapping for
	 */
	DMXPatch* patch(int universe);

protected:
	/** Vector containing all active plugins */
	QVector <DMXPatch*> m_patch;

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

	/**
	 * Open a configuration dialog for the given plugin
	 *
	 * @param pluginName Name of the plugin to configure
	 */
	void configurePlugin(const QString& pluginName);

	/**
	 * Get a status text for the given plugin. If no plugin name is
	 * given, an overall mapping status of all universes is returned.
	 *
	 * @param pluginName Name of the plugin, whose status to get
	 */
	QString pluginStatus(const QString& pluginName = QString::null);

	/**
	 * Append the given plugin to our list of plugins. Will fail if
	 * a plugin with the same name already exists.
	 *
	 * @param outputPlugin The output plugin to append
	 * @return true if successful, otherwise false
	 */
	bool appendPlugin(QLCOutPlugin* outputPlugin);

protected:
	/**
	 * Get a plugin instance by the plugin's name
	 *
	 * @param name The name of the plugin to search for
	 * @return QLCOutPlugin or NULL
	 */
	QLCOutPlugin* plugin(const QString& name);

protected:
	/** The dummy out plugin that is used for unused universes */
	QLCOutPlugin* m_dummyOut;

	/** List containing all available plugins */
	QList <QLCOutPlugin*> m_plugins;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/**
	 * Save DMXMap details into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param wksp_root A parent XML node to save to (workspace)
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/**
	 * Load DMXMap details from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param map_root A DMXMap root node to load from
	 * @return true if successful, otherwise false
	 */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * Defaults
	 *********************************************************************/
public:
	/**
	 * Load default settings for output mapper from QLC global settings
	 */
	void loadDefaults();

	/**
	 * Save default settings for output mapper into QLC global settings
	 */
	void saveDefaults();
};

#endif
