/*
  Q Light Controller
  qlcinplugin.h

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

#ifndef QLCINPLUGIN_H
#define QLCINPLUGIN_H

#include <QStringList>
#include <QtPlugin>
#include <QString>

#include "qlctypes.h"

class QObject;

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class QLCInPlugin
{
public:
	/**
	 * De-initialize the plugin. This is the last thing that is called
	 * for the plugin so make sure nothing is lingering in the twilight
	 * after this call.
	 */
	virtual ~QLCInPlugin() {}

	/**
	 * Initialize the plugin. Since plugins cannot have a user-defined
	 * constructor, any initialization prior to opening any HW must be
	 * done thru this second-stage initialization method. InputMap calls
	 * this function for all plugins exactly once after loading, before
	 * calling any other method from the plugin.
	 *
	 * This is a pure virtual function that must be implemented in all
	 * plugins.
	 */
	virtual void init() = 0;

	/**
	 * Open the specified input line so that the plugin can start
	 * sending input data from that line.
	 *
	 * @param input The input line to open
	 */
	virtual void open(t_input input = 0) = 0;

	/**
	 * Close the specified input line so that the plugin can stop
	 * sending input data from that line.
	 *
	 * @param input The input line to close
	 */
	virtual void close(t_input input = 0) = 0;

	/**
	 * Invoke a configuration dialog for the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual void configure() = 0;

	/**
	 * Provide an information text to be displayed in the plugin manager
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 * @param input If specified, information for the given input line is
	 *              expected. Otherwise provides information for the plugin
	 */
	virtual QString infoText(t_input input = KInputInvalid) = 0;

	/**
	 * Get the plugin's name
	 */
	virtual QString name() = 0;

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	/**
	 * Get a list of input lines' names. The names must be always in the
	 * same order i.e. the first name is the name of input line number 0,
	 * the next one is input line number 1, etc..
	 *
	 * @return Number of inputs provided by the plugin
	 */
	virtual QStringList inputs() = 0;

	/*********************************************************************
	 * Input data listener
	 *********************************************************************/
public:
	/**
	 * Normally, if QLCInPlugin were a QObject, one would just define a
	 * signal to QLCInPlugin that is emitted when input data is available.
	 * It seems, however, that plugins' base interface classes cannot be
	 * QObjects, so the basic signal-slot approach cannot be used.
	 *
	 * Each input plugin implementation must:
	 *    1. Inherit QObject and QLCInPlugin
	 *    2. Re-implement this method and connect their plugin's signal:
	 *
	 *         valueChanged(QLCInPlugin* plugin, t_input line,
	 *                      t_input_channel ch, t_input_value val);
	 *
	 * to the listener object's slot:
	 *
	 *         slotValueChanged(QLCInPlugin* plugin, t_input line,
	 *                          t_input_channel ch, t_input_value val);
	 *
	 * With this approach, input plugins can use the signal-slot mechanism
	 * just as if QLCInPlugin inherited QObject directly.
	 */
	virtual void connectInputData(QObject* listener) = 0;

	/*********************************************************************
	 * Feedback
	 *********************************************************************/
public:
	/**
	 * Send a value back to an input line's channel. This method can be
	 * used for example to move motorized sliders with QLC sliders.
	 *
	 * @param input The input line to send feedback to
	 * @param channel A channel in the input line to send feedback to
	 * @param value An input value to send back to the input channel
	 */
	virtual void feedBack(t_input input, t_input_channel channel,
			      t_input_value value) = 0;
};

Q_DECLARE_INTERFACE(QLCInPlugin, "QLCInPlugin")

#endif
