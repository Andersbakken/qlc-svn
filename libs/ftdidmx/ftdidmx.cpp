/*
  Q Light Controller
  ftdidmx.cpp
  
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

#include "configureftdidmx.h"
#include "ftdidmx.h"
#include "ftdidmxdevice.h"
#include "ftd2xx.h"

#define MAX_DEVICES 16

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void FTDIDMXOut::init()
{
	rescanDevices();
}

void FTDIDMXOut::open(t_output output)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->open();
}

void FTDIDMXOut::close(t_output output)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->close();
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void FTDIDMXOut::rescanDevices()
{
	char devString[MAX_DEVICES][64];
	char *devStringPtr[MAX_DEVICES + 1];
	int devices;

	for (int i = 0; i < MAX_DEVICES; i++)
		devStringPtr[i] = devString[i];
	devStringPtr[MAX_DEVICES] = NULL;

	FT_STATUS st = FT_ListDevices(devString, &devices, FT_LIST_ALL | FT_OPEN_BY_DESCRIPTION);
	if (st == FT_OK) {
		t_output output = 0;
		while (devices > 0) {
			devices--;
			FTDIDMXDevice *device = new FTDIDMXDevice(this, devString[devices], devices, output);
			Q_ASSERT(device != NULL);
			m_devices.insert(output, device);
		}
	}
}

FTDIDMXDevice* FTDIDMXOut::device(const QString& path)
{
	QMapIterator <t_output, FTDIDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		Q_ASSERT(it.value() != NULL);
		if (it.value()->path() == path)
			return it.value();
	}

	return NULL;
}

QStringList FTDIDMXOut::outputs()
{
	QStringList list;

	QMapIterator <t_output, FTDIDMXDevice*> it(m_devices);
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

QString FTDIDMXOut::name()
{
	return QString("FTDI DMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void FTDIDMXOut::configure()
{
	ConfigureFTDIDMXOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString FTDIDMXOut::infoText(t_output output)
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
		str += QString("any device which utilises the FTDI chip hardware.");
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

void FTDIDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->write(channel, value);
}

void FTDIDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->writeRange(address, values, num);
}

void FTDIDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->read(channel, value);
}

void FTDIDMXOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->readRange(address, values, num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(FTDIDMXout, FTDIDMXOut)
