/*
  Q Light Controller
  qlcoutplugin.h

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

#ifndef QLCOUTPLUGIN_H
#define QLCOUTPLUGIN_H

#include <QtPlugin>

#include "qlctypes.h"

class QLCOutPlugin
{
public:
	/**
	 * De-initialize the plugin. This is the last thing that is called
	 * for the plugin so make sure nothing is lingering in the twilight
	 * after this call.
	 */
	virtual ~QLCOutPlugin() {}

	/**
	 * Initialize the plugin. Since plugins cannot have a user-defined
	 * constructor, any initialization prior to opening any HW must be
	 * done thru this second-stage initialization method. OutputMap calls
	 * this function for all plugins exactly once after loading, before
	 * calling any other method from the plugin.
	 *
	 * This is a pure virtual function that must be implemented in all
	 * plugins.
	 */
	virtual void init() = 0;

	/**
	 * Open the specified input line so that the plugin can start
	 * sending output data thru that line.
	 *
	 * @param output The output line to open
	 */
	virtual void open(t_output output = 0) = 0;

	/**
	 * Close the specified output line so that the plugin can stop
	 * sending output data thru that line.
	 *
	 * @param output The output line to close
	 */
	virtual void close(t_output output = 0) = 0;

	/**
	 * Invoke a configuration dialog for the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual void configure() = 0;

	/**
	 * Provide an information text to be displayed in the output manager.
	 * If @output is KOutputInvalid, the info text contains info regarding
	 * the whole plugin. Otherwise it contains info on the specific output.
	 * This information is meant to help users during output mapping.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 *
	 * @param output The output to get info from
	 */
	virtual QString infoText(t_output output = KOutputInvalid) = 0;

	/**
	 * Get the plugin's name
	 */
	virtual QString name() = 0;

	/*********************************************************************
	 * Outputs
	 *********************************************************************/
public:
	/**
	 * Get a list of output lines' names. The names must be always in the
	 * same order i.e. the first name is the name of output line number 0,
	 * the next one is output line number 1, etc..
	 *
	 * @return Number of outputs provided by the plugin
	 */
	virtual QStringList outputs() = 0;

	/**
	 * Write the value of one channel. Channel numbers 0-511 are
	 * for the first universe, 512-1023 for the second, etc...
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param output The output (universe) to write to
	 * @param channel The channel within that universe to write to
	 * @param value The value to write (0-255)
	 */
	virtual void writeChannel(t_output output, t_channel channel,
				  t_value value) = 0;

	/**
	 * Write the values of a number of channels
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param output The output (universe) to write to
	 * @param address The starting address in that universe
	 * @param values An array of values that are written to all
	 *               consequent channels starting from address
	 * @param num The size of values array
	 */
	virtual void writeRange(t_output output, t_channel address,
				t_value* values, t_channel num) = 0;

	/**
	 * Get the value of one channel. Channel numbers 0-511 are
	 * for the first universe, 512-1023 for the second, etc...
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param output The output (universe) to read from
	 * @param channel The channel to read
	 * @param value A pointer to hold the read value
	 */
	virtual void readChannel(t_output output, t_channel channel,
				 t_value* value) = 0;

	/**
	 * Read the values of a number of channels.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param output The output (universe) to read from
	 * @param address The starting address to start reading from
	 * @param values An array that holds the read values
	 * @param num The size of values array
	 */
	virtual void readRange(t_output output, t_channel address,
			       t_value* values, t_channel num) = 0;
};

Q_DECLARE_INTERFACE(QLCOutPlugin, "QLCOutPlugin")

#endif
