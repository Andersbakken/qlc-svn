/*
  Q Light Controller
  udmxout.cpp

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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "libusb_dyn.h"
#else
#include <usb.h>
#endif

#include <QCoreApplication>
#include <QStringList>
#include <QSettings>
#include <QString>
#include <QDebug>

#include "configureudmxout.h"
#include "udmxdevice.h"
#include "udmxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

UDMXOut::~UDMXOut()
{
}

void UDMXOut::init()
{
	usb_init();
	rescanDevices();
}

void UDMXOut::open(t_output output)
{
	if (output < m_devices.count())
		m_devices.at(output)->open();
}

void UDMXOut::close(t_output output)
{
	if (output < m_devices.count())
		m_devices.at(output)->close();
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void UDMXOut::rescanDevices()
{
	struct usb_device* dev;
	struct usb_bus* bus;

	/* Treat all devices as dead first, until we find them again. Those
	   that aren't found, get destroyed at the end of this function. */
	QList <UDMXDevice*> destroyList(m_devices);

	usb_find_busses();
	usb_find_devices();

	/* Iterate thru all buses */
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next)
	{
		/* Iterate thru all devices in each bus */
		for (dev = bus->devices; dev != NULL; dev = dev->next)
		{
			UDMXDevice* udev;

			udev = device(dev);
			if (udev != NULL)
			{
				/* We already have this device and it's still
				   there. Remove from the destroy list and
				   continue iterating. */
				destroyList.removeAll(udev);
				continue;
			}
			else if (UDMXDevice::isUDMXDevice(dev) == true)
			{
				/* This is a new device. Create and append. */
				udev = new UDMXDevice(this, dev);
				m_devices.append(udev);
			}
		}
	}

	/* Destroy those devices that were no longer found. */
	while (destroyList.isEmpty() == false)
	{
		UDMXDevice* udev = destroyList.takeFirst();
		m_devices.removeAll(udev);
		delete udev;
	}
}

UDMXDevice* UDMXOut::device(struct usb_device* usbdev)
{
	QListIterator <UDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		UDMXDevice* udev = it.next();
		if (udev->device() == usbdev)
			return udev;
	}

	return NULL;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

QStringList UDMXOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <UDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->name());
	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString UDMXOut::name()
{
	return QString("uDMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void UDMXOut::configure()
{
	ConfigureUDMXOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString UDMXOut::infoText(t_output output)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("uDMX devices. See ");
		str += QString("<a href=\"http://www.anyma.ch/research/udmx\">");
		str += QString("http://www.anyma.ch</a> for more information.");
		str += QString("</P>");
	}
	else if (output < m_devices.count())
	{
		str += m_devices.at(output)->infoText();
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

void UDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void UDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
	Q_UNUSED(address);

	if (output < m_devices.count())
		m_devices.at(output)->writeRange(values, num);
}

void UDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void UDMXOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(udmxout, UDMXOut)
