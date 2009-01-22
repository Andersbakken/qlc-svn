/*
  Q Light Controller
  outputmap.h

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

#ifndef OUTPUTMAP_H
#define OUTPUTMAP_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QList>
#include <QHash>

#include "common/qlctypes.h"

class QString;
class QDomDocument;
class QDomElement;

class QLCOutPlugin;
class OutputMap;
class OutputPatch;
class OutputMapEditor;
class OutputPatchEditor;

#define KOutputNone QObject::tr("None")
#define KXMLQLCOutputMap "OutputMap"

class OutputMap : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new OutputMap object
	 *
	 * @param universes Number of universes
	 */
	OutputMap(QObject* parent, int universes = KUniverseCount);

	/**
	 * Destroy a OutputMap object
	 */
	~OutputMap();

	/**
	 * Load all output plugins from the plugin directory.
	 */
	void load();

private:
	Q_DISABLE_COPY(OutputMap)

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
	 * Monitor timer
	 *********************************************************************/
public:
	/**
	 * Set the frequency for monitor timer. If frequency > 0, the timer is
	 * started and a changedValues() signal is emitted every frequency:th
	 * of a second. If frequency == 0, the timer is stopped and no signal
	 * emission occurs.
	 */
	void setMonitorTimerFrequency(unsigned int frequency);

	/**
	 * Get the monitor timer frequency.
	 */
	unsigned int monitorTimerFrequency() const;
	
protected slots:
	/** Slot that catches monitor timer signals */
	void slotMonitorTimeout();

signals:
	void changedValues(const QHash <t_channel,t_value>& values);

protected:
	QTimer m_monitorTimer;
	QHash <t_channel,t_value> m_monitorValues;
	
	/*********************************************************************
	 * Patch
	 *********************************************************************/
protected:
	/**
	 * Initialize the patching table
	 */
	void initPatch();

public:
	/**
	 * Get the total number of supported universes
	 *
	 * @return Universe count supported by QLC
	 */
	int universes() { return m_universes; }

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
	OutputPatch* patch(int universe);

protected:
	/** Vector containing all active plugins */
	QVector <OutputPatch*> m_patch;

	/*********************************************************************
	 * Plugins
	 *********************************************************************/
public:
	/**
	 * Get a list of available Output output plugins as a string list
	 * containing the plugins' names
	 *
	 * @return QStringList containing plugins' names
	 */
	QStringList pluginNames();

	/**
	 * Get the number of universes provided by the given plugin.
	 *
	 * @param pluginName Name of the plugin, whose output count to get
	 * @return A list of output names provided by the plugin.
	 */
	QStringList pluginOutputs(const QString& pluginName);

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
	 * @param output Plugin's output line for getting more specific info
	 */
	QString pluginStatus(const QString& pluginName = QString::null,
			     t_output output = KOutputInvalid);

	/**
	 * Append the given plugin to our list of plugins. Will fail if
	 * a plugin with the same name already exists.
	 *
	 * @param outputPlugin The output plugin to append
	 * @return true if successful, otherwise false
	 */
	bool appendPlugin(QLCOutPlugin* outputPlugin);

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
	 * Save OutputMap details into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param wksp_root A parent XML node to save to (workspace)
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/**
	 * Load OutputMap details from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param map_root A OutputMap root node to load from
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
