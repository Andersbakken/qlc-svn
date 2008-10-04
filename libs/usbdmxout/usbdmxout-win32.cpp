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
	QString info;
	QString s;

	/* HTML page Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * Outputs
	 *********************************************************************/

	/* Output */
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Output");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Device name */
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Device name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	
	/* Output lines */
	QMapIterator <t_output, USBDMXDevice*> it(m_devices);
	bool atLeastOne = false;
	while (it.hasNext() == true)
	{
		USBDMXDevice* device = it.next().value();
		Q_ASSERT(device != NULL);

		info += QString("<TR>");
		info += QString("<TD>%1</TD>").arg(device->output());
		info += QString("<TD>%1</TD>").arg(device->name());
		info += QString("</TR>");
		atLeastOne = true;
	}
	
	if (atLeastOne == false)
	{
		info += QString("<TR>");
		info += QString("<TD COLSPAN=\"2\">No devices available</TD>");
		info += QString("</TR>");
	}
	
	info += QString("</TABLE>");

	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
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
