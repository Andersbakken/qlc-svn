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
#include <QSettings>

#include "configureftdidmx.h"
#include "ftdidmx.h"
#include "ftdidmxdevice.h"
#include "ftd2xx.h"

static struct FTDIDevice known_devices[] =
{
	{"EntTec Open DMX USB", 0x0403, 0x6001, 0},
	{"Homebrew USB -> DMX", 0x0403, 0xEC70, 0},
	{"EntTec DMX USB Pro",  0x0403, 0x6001, 1}
};

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void FTDIDMXOut::init()
{
	QSettings settings;
	QVariant devTypes = settings.value("/ftdidmx/types/number", 0);
	m_number_device_types = sizeof(known_devices) / sizeof(FTDIDevice);
	if (devTypes.type() == QVariant::Int) {
		int i = devTypes.toInt();
		m_device_types = (FTDIDevice*)malloc(sizeof(FTDIDevice) * (m_number_device_types + i));
		
		while (i > 0) {
			i--;
			QVariant vid = settings.value(QString("/ftdidmx/types/vid%1").arg(i), QVariant(0));
			QVariant pid = settings.value(QString("/ftdidmx/types/pid%1").arg(i), QVariant(0));
			QVariant interface = settings.value(QString("/ftdidmx/types/interface%1").arg(i), QVariant(0));
			QVariant name = settings.value(QString("/ftdidmx/types/name%1").arg(i), QVariant(""));
			if (vid.type() == QVariant::Int &&
				pid.type() == QVariant::Int &&
				interface.type() == QVariant::Int &&
				name.type() == QVariant::String) {
				FTDIDevice d = m_device_types[m_number_device_types + i];
				d.vid = vid.toInt();
				d.pid = pid.toInt();
				d.type = interface.toInt();
				d.name = name.toString().toAscii();
			}
		}
		
		m_number_device_types += i;
	} else {
		m_device_types = (FTDIDevice*)malloc(sizeof(FTDIDevice) * m_number_device_types);
	}
	
	for (unsigned int i = 0; i < sizeof(known_devices) / sizeof(FTDIDevice); i++) {
		m_device_types[i].vid = known_devices[i].vid;
		m_device_types[i].pid = known_devices[i].pid;
		m_device_types[i].type = known_devices[i].type;
		m_device_types[i].name = known_devices[i].name;
	}
}

FTDIDMXOut::FTDIDMXOut()
{
	m_device_types = NULL;
}

FTDIDMXOut::~FTDIDMXOut()
{
	if (m_device_types != NULL) {
		free(m_device_types);
	}
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

FTDIDMXDevice* FTDIDMXOut::device(const QString& path)
{
	QSettings settings;
	int output = path.toInt();
	
	if (m_devices.contains(output)) {
		return m_devices.value(output);
	} else {
		int loadedDevices = settings.value("/ftdidmx/devices/number", QVariant(0)).toInt();
		if (loadedDevices <= output) {
			return NULL;
		}
		QString serial = settings.value(QString("/ftdidmx/devices/serial%1").arg(output)).toString();
		int dType = settings.value(QString("/ftdidmx/devices/type%1").arg(output)).toInt();
		int pid = settings.value(QString("/ftdidmx/types/pid%1").arg(dType)).toInt();
		int vid = settings.value(QString("/ftdidmx/types/vid%1").arg(dType)).toInt();
		int type = settings.value(QString("/ftdidmx/types/interface%1").arg(dType)).toInt();
		
		FTDIDMXDevice *device = new FTDIDMXDevice(this, vid, pid, type, serial.toAscii().data(), output);
		if (device != NULL) {
			m_devices.insert(output, device);
			return device;
		}
	}

	return NULL;
}

QStringList FTDIDMXOut::outputs()
{
	QStringList list;
	QSettings settings;

	int loadedDevices = settings.value("/ftdidmx/devices/number", QVariant(0)).toInt();
	for (int i = 0; i < loadedDevices; i++) {
		list << settings.value(QString("/ftdidmx/devices/serial%1").arg(i)).toString();
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

Q_EXPORT_PLUGIN2(ftdidmxout, FTDIDMXOut)
