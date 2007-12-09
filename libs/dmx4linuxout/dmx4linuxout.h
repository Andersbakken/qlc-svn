/*
  Q Light Controller
  dmx4linuxout.h
  
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

#ifndef DMX4LINUXOUT_H
#define DMX4LINUXOUT_H

#include <qptrlist.h>
#include <qstring.h>
#include <dmx/dmx.h>
#include <dmx/dmxioctl.h>

#include "common/outputplugin.h"
#include "common/types.h"

class ConfigureDMX4LinuxOut;

extern "C" OutputPlugin* create();

class DMX4LinuxOut : public OutputPlugin
{
	Q_OBJECT

	friend class ConfigureDMX4LinuxOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	DMX4LinuxOut();
	~DMX4LinuxOut();

	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	/** (Re-)open the plugin */
	int open();

	/** Close the plugin */
	int close();

	/** Number of output lines (universes) */
	int outputs();

protected:
	/** File handle for /dev/dmx */
	int m_fd;

	/** Error code for /dev/dmx open() */
	int m_openError;

	/** Generic information on DMX4Linux */
	struct dmx_info m_dmxInfo;

	/** Error code for DMX information ioctl() */
	int m_dmxInfoError;

	/** Capabilities for each output line (universe) */
	struct dmx_capabilities* m_dmxCaps;

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	/** Invoke a configuration dialog */
	int configure(QWidget* parentWidget);

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/** Get an information text blob to be displayed in plugin manager */
	QString infoText();

	/*********************************************************************
	 * Value read/write
	 *********************************************************************/
public:
	int writeChannel(t_channel channel, t_value value);
	int writeRange(t_channel address, t_value* values, t_channel num);

	int readChannel(t_channel channel, t_value &value);
	int readRange(t_channel address, t_value* values, t_channel num);

protected:
	t_value m_values[KChannelMax];
};

#endif
