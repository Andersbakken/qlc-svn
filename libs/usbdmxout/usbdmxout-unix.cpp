/*
  Q Light Controller
  usbdmxout-unix.cpp

  Copyright (c) Christian Suehs
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

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QList>
#include <QDir>

#include "configureusbdmxout.h"
#include "usbdmxdevice-unix.h"
#include "usbdmxout-unix.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void USBDMXOut::init()
{
	rescanDevices();
}

void USBDMXOut::open(t_output output)
{
	if (output < m_devices.count())
		m_devices.at(output)->open();
}

void USBDMXOut::close(t_output output)
{
	if (output < m_devices.count())
		m_devices.at(output)->close();
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void USBDMXOut::rescanDevices()
{
	QStringList nameFilters;
	QDir dir("/dev/");
	t_output output;
	QString path;

	output = 0;

	QList <USBDMXDevice*> destroyList(m_devices);

	nameFilters << "usbdmx*";
	QStringListIterator it(dir.entryList(nameFilters,
					     QDir::Files | QDir::System));
	while (it.hasNext() == true)
	{
		USBDMXDevice* dev;

		path = dir.absolutePath() + QDir::separator() + it.next();

		dev = device(path);
		if (dev != NULL)
		{
			/* This device still exists. Don't destroy it. */
			destroyList.removeAll(dev);
		}
		else
		{
			dev = new USBDMXDevice(this, path);
			m_devices.append(dev);
		}
	}

	/* Destroy all devices that weren't found in the rescan */
	while (destroyList.isEmpty() == false)
	{
		USBDMXDevice* dev = destroyList.takeFirst();
		m_devices.removeAll(dev);
		delete dev;
	}
}

USBDMXDevice* USBDMXOut::device(const QString& path)
{
	QListIterator <USBDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		USBDMXDevice* dev = it.next();
		if (dev->path() == path)
			return dev;
	}

	return NULL;
}

QStringList USBDMXOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <USBDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->name());
	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString USBDMXOut::name()
{
	return QString("USB DMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void USBDMXOut::configure()
{
	ConfigureUSBDMXOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString USBDMXOut::infoText(t_output output)
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
		str += QString("devices manufactured by Peperoni Light: ");
		str += QString("Rodin 1, Rodin 2, Rodin T, X-Switch and ");
		str += QString("USBDMX21. See ");
		str += QString("<a href=\"http://www.peperoni-light.de\">");
		str += QString("http://www.peperoni-light.de</a> for more ");
		str += QString("information. ");
		str += QString("</P>");
	}
	else if (output < m_devices.count())
	{
		str += QString("<H3>%1</H3>").arg(outputs()[output]);
		str += QString("<P>");
		str += QString("Device is operating correctly.");
		str += QString("</P>");
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

void USBDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void USBDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
	Q_UNUSED(address);

	if (output < m_devices.count())
		m_devices.at(output)->writeRange(values, num);
}

void USBDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void USBDMXOut::readRange(t_output output, t_channel address, t_value* values,
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

Q_EXPORT_PLUGIN2(usbdmxout, USBDMXOut)
