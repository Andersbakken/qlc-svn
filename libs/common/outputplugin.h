/*
  Q Light Controller
  outputplugin.h

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

#ifndef OUTPUTPLUGIN_H
#define OUTPUTPLUGIN_H

#include "common/plugin.h"
#include "common/types.h"

class OutputPlugin : public Plugin
{
	Q_OBJECT
		
public:
	/**
	 * Create a new output plugin
	 */
	OutputPlugin();

	/**
	 * Destroy an output plugin
	 */
	virtual ~OutputPlugin();
	
	/**
	 * Get the number of outputs provided by the plugin.
	 * Default implementation provides one output line.

	 * One might call these outputs also universes, but because "universe"
	 * is already used quite widely in QLC, "output" was chosen here
	 * instead to prevent confusion.
	 */
	virtual int outputs() { return 1; }
	
	/**
	 * Write the value of one channel. Channel numbers 0-511 are
	 * for the first universe, 512-1023 for the second, etc...
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param channel The channel (and universe) to write to
	 * @param value The value to write (0-255)
	 */
	virtual int writeChannel(t_channel channel, t_value value) = 0;

	/**
	 * Write the values of a number of channels
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param address The starting address to start writing from
	 * @param values An array of values that are written to all
	 *               consequent channels starting from address.
	 * @param num The size of values array
	 */
	virtual int writeRange(t_channel address, t_value* values,
			       t_channel num) = 0;
	
	/**
	 * Get the value of one channel. Channel numbers 0-511 are
	 * for the first universe, 512-1023 for the second, etc...
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param channel The channel (and universe) to read from
	 * @param value A reference to a t_value holding the read value
	 */
	virtual int readChannel(t_channel channel, t_value &value) = 0;

	/**
	 * Read the values of a number of channels.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all output plugins.
	 *
	 * @param address The starting address to start reading from
	 * @param values An array that holds the read values
	 * @param num The size of values array
	 */
	virtual int readRange(t_channel address, t_value* values,
			      t_channel num) = 0;
};

#endif
