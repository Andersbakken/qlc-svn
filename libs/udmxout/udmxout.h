/*
  Q Light Controller
  udmxout.h

  Copyright (c)	Lutz Hillebrand
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

#ifndef UDMXOUT_H
#define UDMXOUT_H

#include <QStringList>
#include <QtPlugin>
#include <QList>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class ConfigureUDmxOut;
class UDMXDevice;
class QString;

/*****************************************************************************
 * USBDMXOut
 *****************************************************************************/

class UDMXOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	friend class ConfigureUDMXOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	virtual ~UDMXOut();

	void init();
	void open(t_output output);
	void close(t_output output);

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	void rescanDevices();

protected:
	/** Get a UDMXDevice entry by its usbdev struct */
	UDMXDevice* device(struct usb_device* usbdev);

protected:
	/** List of available devices */
	QList <UDMXDevice*> m_devices;

	/*********************************************************************
	 * Outputs
	 *********************************************************************/
public:
	/** Get the names of outputs provided by this plugin */
	QStringList outputs();

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Plugin status
	 *********************************************************************/
public:
	QString infoText(t_output output = KOutputInvalid);

	/*********************************************************************
	 * Value read/write methods
	 *********************************************************************/
public:
	void writeChannel(t_output output, t_channel channel, t_value value);
	void writeRange(t_output output, t_channel address, t_value* values,
			t_channel num);

	void readChannel(t_output output, t_channel channel, t_value* value);
	void readRange(t_output output, t_channel address, t_value* values,
		       t_channel num);
};

#endif
