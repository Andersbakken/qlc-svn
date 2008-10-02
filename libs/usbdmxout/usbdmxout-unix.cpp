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

#include <QApplication>
#include <QStringList>
#include <QPalette>
#include <QString>
#include <QColor>
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
	if (m_devices.contains(output) == true)
		m_devices[output]->open();
}

void USBDMXOut::close(t_output output)
{
	if (m_devices.contains(output) == true)
		m_devices[output]->close();
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

	nameFilters << "usbdmx*";
	QStringListIterator it(dir.entryList(nameFilters,
					     QDir::Files | QDir::System));
	while (it.hasNext() == true)
	{
		path = dir.absolutePath() + QDir::separator() + it.next();

		if (device(path) == NULL)
		{
			USBDMXDevice* device;
			device = new USBDMXDevice(this, path, output);
			Q_ASSERT(device != NULL);
			m_devices.insert(output, device);
		}
	}
}

USBDMXDevice* USBDMXOut::device(const QString& path)
{
	QMapIterator <t_output, USBDMXDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		Q_ASSERT(it.value() != NULL);
		if (it.value()->path() == path)
			return it.value();
	}

	return NULL;
}

QStringList USBDMXOut::outputs()
{
	QStringList list;

	QMapIterator <t_output, USBDMXDevice*> it(m_devices);
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

	/* Title */
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
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
	while (it.hasNext() == true)
	{
		it.next();
		info += QString("<TR>");
		info += QString("<TD>%1</TD>").arg(it.value()->output() + 1);
		info += QString("<TD>%1</TD>").arg(it.value()->name());
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

int USBDMXOut::writeChannel(t_channel channel, t_value value)
{
	t_output output = t_output(channel / 512);
	t_channel dmxChannel = channel % 512;

	if (m_devices.contains(output) == true)
		m_devices[output]->write(dmxChannel, value);

	return 0;
}

int USBDMXOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	/* TODO */
	return 0;
}

int USBDMXOut::readChannel(t_channel channel, t_value &value)
{
	t_output output = t_output(channel / 512);
	t_channel dmxChannel = channel % 512;

	if (m_devices.contains(output) == true)
		value = m_devices[output]->read(dmxChannel);
	
	return 0;
}

int USBDMXOut::readRange(t_channel address, t_value* values, t_channel num)
{
	/* TODO */
	return 0;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(usbdmxout, USBDMXOut)
