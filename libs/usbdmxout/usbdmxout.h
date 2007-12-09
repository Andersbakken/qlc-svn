/*
  Q Light Controller
  usbdmxout.h
  
  Copyright (c) Christian Sühs
		Stefan Krumm
		Heikki Junnila
  
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

#ifndef USBDMXOUT_H
#define USBDMXOUT_H

#include <qptrlist.h>
#include <qstring.h>
#include <qvaluevector.h>

#include "common/outputplugin.h"
#include "common/types.h"

class ConfigureUsbDmxOut;
class QPoint;

extern "C" OutputPlugin* create();

/* Maximum number of supported device entries (/dev/usbdmx*) */
#define MAX_USBDMX_DEVICES 8

/*****************************************************************************
 * USBDMXOut
 *****************************************************************************/

class USBDMXOut : public OutputPlugin
{
	Q_OBJECT

	friend class ConfigureUSBDMXOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	USBDMXOut();
	virtual ~USBDMXOut();

	/*********************************************************************
	 * Plugin open/close
	 *********************************************************************/
public:
	int open();
	int close();
	int outputs();

protected:
	int m_devices[MAX_USBDMX_DEVICES];

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	int configure(QWidget* parentWidget);

	/*********************************************************************
	 * Plugin status
	 *********************************************************************/
public:
	QString infoText();

	/*********************************************************************
	 * Value read/write methods
	 *********************************************************************/
public:
	int writeChannel(t_channel channel, t_value value);
	int writeRange(t_channel address, t_value* values, t_channel num);

	int readChannel(t_channel channel, t_value &value);
	int readRange(t_channel address, t_value* values, t_channel num);

protected:
	t_value m_values[MAX_USBDMX_DEVICES * 512];
};

#endif
