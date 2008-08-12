/*
  Q Light Controller
  inputmap.h
  
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

#ifndef INPUTMAP_H
#define INPUTMAP_H

#include <QObject>
#include <QVector>
#include <QList>

#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QLCInPlugin;
class InputMap;

#define KInputNone QObject::tr("None")

/*****************************************************************************
 * InputPatch
 *****************************************************************************/

#define KXMLQLCInputPatch "Patch"
#define KXMLQLCInputPatchUniverse "Universe"
#define KXMLQLCInputPatchPlugin "Plugin"
#define KXMLQLCInputPatchInput "Input"
#define KXMLQLCInputPatchPluginNone "None"

/**
 * This is a simple container class that stores only the pointer to an
 * existing plugin and an input line provided by that plugin.
 */
class InputPatch
{
	friend class InputMap;
	friend class InputMapEditor;
	friend class InputPatchEditor;

public:
	InputPatch(QLCInPlugin* p, int i) { plugin = p; input = i; }
	virtual ~InputPatch() {}

protected:
	/**
	 * Save an InputPatch's properties into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param map_root An XML root node (InputMap) to save under
	 * @param universe The internal universe number that the patch is
	 *                 addressed to
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* map_root, int universe);

	/**
	 * Create and load an InputPatch's properties from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param root An XML node containing an InputPatch to load from
	 * @param inputMap InputMap object that contains the loaded patch
	 * @return true if successful, otherwise false
	 */
	static bool loader(QDomDocument* doc, QDomElement* root,
			   InputMap* inputMap);

	QLCInPlugin* plugin;
	int input;
};

/*****************************************************************************
 * InputMap
 *****************************************************************************/

#define KXMLQLCInputMap "InputMap"

class InputMap : public QObject
{
	Q_OBJECT

	friend class InputPatch;
	friend class InputMapEditor;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new InputMap object, with the given amount of input
	 * universes.
	 */
	InputMap(int universes = 4);

	/**
	 * Destroy an InputMap object
	 */
	virtual ~InputMap();

	/**
	 * Load all input plugins from the input plugin directory
	 */
	void load();

	/*********************************************************************
	 * Input data
	 *********************************************************************/
public slots:
	void slotValueChanged(QLCInPlugin* plugin, t_input input,
			      t_input_channel channel, t_input_value value);

	/*********************************************************************
	 * Patch
	 *********************************************************************/
public:
	/**
	 * Get the total number of supported input universes
	 *
	 * @return Input universe count supported by QLC
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
	 * @param universe The input universe to patch
	 * @param pluginName The name of the plugin to patch to the universe
	 * @param input An input universe provided by the plugin to patch to
	 * @return true if successful, otherwise false
	 */
	bool setPatch(unsigned int universe, const QString& pluginName,
		      unsigned int input = 0);

	/**
	 * Get mapping for an input universe.
	 *
	 * @param universe The internal input universe to get mapping for
	 */
	InputPatch* patch(int universe);

protected:
	/** Vector containing all active input plugins */
	QVector <InputPatch*> m_patch;

	/** Total number of supported input universes */
	int m_universes;

	/*********************************************************************
	 * Plugins
	 *********************************************************************/
public:
	/**
	 * Get a list of available input plugins as a string list
	 * containing the plugins' names
	 *
	 * @return QStringList containing plugins' names
	 */
	QStringList pluginNames();

	/**
	 * Get the number of input lines provided by the given plugin.
	 *
	 * @param pluginName Name of the plugin, whose input count to get
	 * @return Number of input lines provided by the plugin.
	 *         Zero is returned if pluginName is invalid.
	 */
	int pluginInputs(const QString& pluginName);

	/**
	 * Open a configuration dialog for the given plugin
	 *
	 * @param pluginName Name of the plugin to configure
	 */
	void configurePlugin(const QString& pluginName);

	/**
	 * Get a status text for the given plugin. 
	 *
	 * @param pluginName Name of the plugin, whose status to get
	 */
	QString pluginStatus(const QString& pluginName = QString::null);

	/**
	 * Append the given plugin to our list of plugins. Will fail if
	 * a plugin with the same name already exists.
	 *
	 * @param inputPlugin The input plugin to append
	 * @return true if successful, otherwise false
	 */
	bool appendPlugin(QLCInPlugin* inputPlugin);

protected:
	/**
	 * Get a plugin instance by the plugin's name
	 *
	 * @param name The name of the plugin to search for
	 * @return QLCInPlugin or NULL
	 */
	QLCInPlugin* plugin(const QString& name);

protected:
	/** List containing all available input plugins */
	QList <QLCInPlugin*> m_plugins;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/**
	 * Save InputMap details into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param wksp_root A parent XML node to save to (workspace)
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/**
	 * Load InputMap details from an XML document
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
	 * Load default settings for input mapper from QLC global settings
	 */
	void loadDefaults();

	/**
	 * Save default settings for input mapper into QLC global settings
	 */
	void saveDefaults();
};

#endif
