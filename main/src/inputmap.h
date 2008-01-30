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

#include <qptrvector.h>
#include <qptrlist.h>
#include <qobject.h>

#include "common/types.h"

class InputPlugin;
class PluginLoader;

class InputMap : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	InputMap();
	virtual ~InputMap();

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
	bool appendPlugin(InputPlugin* inputPlugin);

protected:
	/**
	 * Get a plugin instance by the plugin's name
	 *
	 * @param name The name of the plugin to search for
	 * @return InputPlugin or NULL
	 */
	InputPlugin* plugin(const QString& name);

protected:
	/** List containing all available plugins */
	QPtrList<InputPlugin> m_plugins;

	/*********************************************************************
	 * Defaults
	 *********************************************************************/
public:
	/**
	 * Load default settings for input mapper from QLC global settings
	 *
	 * @param path Domain path of global settings (KApplicationNameLong)
	 */
	void loadDefaults(const QString& path);

	/**
	 * Save default settings for input mapper into QLC global settings
	 *
	 * @param path Domain path of global settings (KApplicationNameLong)
	 */
	void saveDefaults(const QString& path);
};

#endif
