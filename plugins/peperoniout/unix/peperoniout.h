/*
  Q Light Controller
  peperoniout.h

  Copyright (c)	Heikki Junnila

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

#ifndef PEPERONIOUT_H
#define PEPERONIOUT_H

#include <QStringList>
#include <QtPlugin>
#include <QList>

#include "qlcoutplugin.h"
#include "qlctypes.h"

class PeperoniDevice;
class QString;

/*****************************************************************************
 * USBDMXOut
 *****************************************************************************/

class PeperoniOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	virtual ~PeperoniOut();

	void init();
	void open(quint32 output);
	void close(quint32 output);

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	void rescanDevices();

protected:
	/** Get a PeperoniDevice entry by its usbdev struct */
	PeperoniDevice* device(struct usb_device* usbdev);

protected:
	/** List of available devices */
	QList <PeperoniDevice*> m_devices;

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
	QString infoText(quint32 output = KOutputInvalid);

	/*********************************************************************
	 * Value read/write methods
	 *********************************************************************/
public:
	/** \reimp */
	void outputDMX(quint32 output, const QByteArray& universe);
};

#endif
