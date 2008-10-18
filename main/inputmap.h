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

#include "common/qlcinputdevice.h"
#include "common/qlctypes.h"

class QLCInPlugin;
class InputPatch;
class InputMap;

class QDomDocument;
class QDomElement;

#define KInputNone QObject::tr("None")
#define KXMLQLCInputMap "InputMap"

class InputMap : public QObject
{
	Q_OBJECT

	friend class InputPatch;
	friend class InputMapEditor;
	friend class InputPatchEditor;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new InputMap object, with the given amount of input
	 * universes.
	 */
	InputMap(QObject* parent,
		 t_input_universe universes = KInputUniverseCount);

	/**
	 * Destroy an InputMap object
	 */
	virtual ~InputMap();

	/*********************************************************************
	 * Input data
	 *********************************************************************/
public slots:
	void slotValueChanged(QLCInPlugin* plugin, t_input input,
			      t_input_channel channel, t_input_value value);

signals:
	void inputValueChanged(t_input_universe universe,
			       t_input_channel channel, t_input_value value);

	/*********************************************************************
	 * Patch
	 *********************************************************************/
public:
	/**
	 * Get the number of supported input universes
	 */
	t_input_universe universes();

	/**
	 * Get the assigned input line names for supported input universes, to
	 * be used in UI controls in the form "x: name", with universe numbers
	 * start from 1 instead of 0.
	 */
	QStringList universeNames();

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
	 * @param templateName The name of a template that describes the mapped
	 *                     device
	 * @return true if successful, otherwise false
	 */
	bool setPatch(t_input_universe universe,
		      const QString& pluginName, t_input input,
		      const QString& templateName = QString::null);

	/**
	 * Get mapping for an input universe.
	 *
	 * @param universe The internal input universe to get mapping for
	 */
	InputPatch* patch(t_input_universe universe);

protected:
	/** Vector containing all active input plugins and the internal
	    universes that they are associated to. */
	QVector <InputPatch*> m_patch;

	/** Total number of supported input universes */
	t_input_universe m_universes;

	/*********************************************************************
	 * Plugins
	 *********************************************************************/
public:
	/** Get the directory used to store input plugins */
	QString pluginPath() const;

protected:
	/**
	 * Load all input plugins from the input plugin directory
	 */
	void loadPlugins();

public:
	/**
	 * Get a list of available input plugins as a string list
	 * containing the plugins' names
	 *
	 * @return QStringList containing plugins' names
	 */
	QStringList pluginNames();

	/**
	 * Get the names of all input lines provided by the given plugin.
	 *
	 * @param pluginName Name of the plugin, whose input count to get
	 * @return A QStringList containing the names of each input line
	 *
	 */
	QStringList pluginInputs(const QString& pluginName);

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
	 * @param input A specific input identifier
	 */
	QString pluginStatus(const QString& pluginName = QString::null,
			     t_input input = KInputInvalid);

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
	 * Device templates
	 *********************************************************************/
public:
	/** Get the directory used to store input template files */
	QString templatePath() const;

protected:
	/** Load all available device templates */
	void loadTemplates();

public:
	/** Get a list of available device template names */
	QStringList deviceTemplateNames();

	/** Get a device template by its name */
	QLCInputDevice* deviceTemplate(const QString& name);

	/** Add a new device template */
	void addDeviceTemplate(QLCInputDevice* deviceTemplate);

	/** Remove an existing device template by its name and delete it */
	void removeDeviceTemplate(const QString& name);
	
protected:
	/** List that contains all available device templates */
	QList <QLCInputDevice*> m_deviceTemplates;

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
	 * @param map_root An OutputMap root node to load from
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
