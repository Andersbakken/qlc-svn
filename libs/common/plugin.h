/*
  Q Light Controller
  plugin.h

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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qmainwindow.h>
#include <limits.h>

typedef unsigned short t_plugin_id;

const t_plugin_id KPluginID    = 0;
const t_plugin_id KPluginIDMin = 1;
const t_plugin_id KPluginIDMax = USHRT_MAX;

class Plugin : public QObject
{
	Q_OBJECT

public:
	/**
	 * Construct a new plugin
	 */
	Plugin();

	/**
	 * Destroy the plugin
	 */
	virtual ~Plugin();
	
	/**
	 * Plugin type
	 */
	enum PluginType
	{
		OutputType,
		InputType
	};
	
	/**
	 * Open (initialize for operation) the plugin.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 */
	virtual int open() = 0;

	/**
	 * Close (de-initialize) the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual int close() = 0;

	/**
	 * Invoke a configuration dialog for the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 * @param parentWidget A parent QWidget for the configuration dialog
	 */
	virtual int configure(QWidget* parentWidget) = 0;

	/**
	 * Provide an information text to be displayed in the plugin manager
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual QString infoText() = 0;

	/*********************************************************************
	 * Standard functions that should not be overwritten
	 *********************************************************************/
	
	/**
	 * Get the plugin's name
	 */
	QString name();

	/**
	 * Get the plugin's version (possibly obsolete)
	 */
	unsigned long version();

	/**
	 * Get the plugin's type
	 */
	PluginType type();

	/**
	 * Set the plugin's library handle (returned by dlopen())
	 */
	void setHandle(void* handle);

	/**
	 * Get the plugin's library handle
	 */
	void* handle();

	/**
	 * @todo
	 */
	void setEventReceiver(QObject* parent);

	/**
	 * @todo
	 */
	QObject* eventReceiver() const;

protected:
	QString m_name;
	PluginType m_type;
	unsigned long m_version;
	void* m_handle;
	QObject* m_eventReceiver;
};

#endif
