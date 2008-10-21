/*
  Q Light Controller
  serialdmx.cpp
  
  Copyright (c) Christopher Staite
  
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

#include <QApplication>
#include <QStringList>
#include <QPalette>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QList>
#include <QDir>

#include "configureserialdmx.h"
#include "serialdmx.h"
#include "serialdmxdevice.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void SerialDMXOut::init()
{
	rescanDevices();
}

void SerialDMXOut::open(t_output output)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->open();
}

void SerialDMXOut::close(t_output output)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->close();
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void SerialDMXOut::rescanDevices()
{
	QStringList nameFilters;
	QDir dir("/dev/");
	t_output output;
	QString path;

	output = 0;

	// What names could there be?
	// This should be cross-referenced with detectDeviceType
	nameFilters << "tty*";
	QStringListIterator it(dir.entryList(nameFilters,
					     QDir::Files | QDir::System));
	while (it.hasNext() == true)
	{
		path = dir.absolutePath() + QDir::separator() + it.next();

		if (device(path) == NULL)
		{
			SerialDMXDevice* device;
			device = new SerialDMXDevice(this, path, output);
			Q_ASSERT(device != NULL);
			m_devices.insert(output, device);
		}
	}
}

SerialDMXDevice* SerialDMXOut::device(const QString& path)
{
	QMapIterator <t_output, SerialDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		Q_ASSERT(it.value() != NULL);
		if (it.value()->path() == path)
			return it.value();
	}

	return NULL;
}

QStringList SerialDMXOut::outputs()
{
	QStringList list;

	QMapIterator <t_output, SerialDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		Q_ASSERT(it.value() != NULL);
		list << it.value()->name();
	}

	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString SerialDMXOut::name()
{
	return QString("Serial DMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void SerialDMXOut::configure()
{
	ConfigureSerialDMXOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString SerialDMXOut::infoText(t_output output)
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
		str += QString("any device which runs though a serial port ");
		str += QString("or emulated serial port (i.e. USB->Serial).  ");
		str += QString("Key examples are homebrew devices using the FTDI chip ");
		str += QString("see <address>http://www.telltronics.org/software/dmx/</address> for ");
		str += QString("more information. ");
		str += QString("</P>");
	}
	else if (m_devices.contains(output) == true)
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

void SerialDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->write(channel, value);
}

void SerialDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->writeRange(address, values, num);
}

void SerialDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->read(channel, value);
}

void SerialDMXOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->readRange(address, values, num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(serialdmxout, SerialDMXOut)
