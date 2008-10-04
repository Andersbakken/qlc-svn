/*
  Q Light Controller
  usbdmxout-win32.cpp
  
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

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <windows.h>
#include <QPalette>
#include <QDebug>
#include <QString>
#include <QColor>

#include "configureusbdmxout.h"
#include "usbdmxdevice-win32.h"
#include "usbdmxout-win32.h"
#include "usbdmx-dynamic.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void USBDMXOut::init()
{
	/* Load usbdmx.dll */
	m_usbdmx = usbdmx_init();
	if (m_usbdmx == NULL)
	{
		qWarning() << "Loading USBDMX.DLL failed.";
	}
	else if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
	{
		/* verify USBDMX dll version */
		qWarning() << "USBDMX.DLL version does not match. Abort.";
		qWarning() << "Found" << m_usbdmx->version() << "but expected"
			   << USBDMX_DLL_VERSION;
	}
	else
	{
		qDebug() << "Using USBDMX.DLL version" << m_usbdmx->version();
	}
	
	rescanDevices();
}

void USBDMXOut::rescanDevices()
{
	for (t_output i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		HANDLE handle;
		if (m_usbdmx->open(i, &handle) == TRUE)
		{
			m_usbdmx->close(handle);
			if (m_devices.contains(i) == false)
			{
				USBDMXDevice* device;
				device = new USBDMXDevice(this, m_usbdmx, i);
				m_devices.insert(i, device);
			}
		}
	}
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

void USBDMXOut::open(t_output output)
{
	if (m_usbdmx == NULL)
		return;

	if (m_devices.contains(output) == true)
		m_devices[output]->open();
}

void USBDMXOut::close(t_output output)
{
	if (m_usbdmx == NULL)
		return;

	if (m_devices.contains(output) == true)
		m_devices[output]->close();
}

QStringList USBDMXOut::outputs()
{
	QStringList list;
	QMapIterator <t_output, USBDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next().value()->name();

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
		str += QString("<address>www.peperoni-light.de</address> for ");
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

void USBDMXOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->write(channel, value);
}

void USBDMXOut::writeRange(t_output output, t_channel address, t_value* values,
			   t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->writeRange(address, values, num);
}

void USBDMXOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->read(channel, value);
}

void USBDMXOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->readRange(address, values, num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(usbdmxout, USBDMXOut)
