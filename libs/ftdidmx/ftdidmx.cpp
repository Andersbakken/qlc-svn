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

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QList>
#include <QDir>

#include "configureftdidmx.h"
#include "ftdidmx.h"
#include "ftdidmxdevice.h"
#include "ftd2xx.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void FTDIDMXOut::init()
{
	// TODO: this should be read from the configuration
#ifndef WIN32
	// On Windows the users must ensure the ftdi driver is correct
	// for their hardware, *NIX users can just dynamically
	// alter the hardware settings in QLC
	setVIDPID(known_devices[0].vid, known_devices[0].pid);
#endif
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

void FTDIDMXOut::setVIDPID(int vid, int pid)
{
	m_vidpid_mutex.lock();
	m_scan_vid = vid;
	m_scan_pid = pid;
	m_vidpid_mutex.unlock();
	rescanDevices();
}

void FTDIDMXOut::rescanDevices()
{
	DWORD devices;
	// Make sure we have a static vid/pid throughout the scanning process
	m_vidpid_mutex.lock();
	int vid = m_scan_vid;
	int pid = m_scan_pid;
	m_vidpid_mutex.unlock();

#ifndef WIN32
	FT_SetVIDPID(vid, pid);
#endif
	if (FT_CreateDeviceInfoList(&devices) != FT_OK)
		devices = MAX_NUM_DEVICES;

	// Array to store serial numbers in
	char devString[devices][64];
	char *devStringPtr[devices + 1];

	for (unsigned int i = 0; i < devices; i++)
		devStringPtr[i] = devString[i];
	devStringPtr[devices] = NULL;

	FT_STATUS st = FT_ListDevices(devStringPtr, &devices,
				      FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
	if (st == FT_OK)
	{
		t_output output = 0;
		while (devices > 0)
		{
			devices--;
			FTDIDMXDevice* device;
			device = new FTDIDMXDevice(this, vid, pid,
						   devString[devices], output);
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
		str += QString("any device which utilises the FTDI chip ");
		str += QString("hardware.");
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

