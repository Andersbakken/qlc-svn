/*
  Q Light Controller
  enttecdmxusb.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

/* HAL DBus service is not available on Apple & Windows */
#ifdef DBUS_ENABLED
#include <QDBusConnection>
#endif

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbout.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

void EnttecDMXUSBOut::init()
{
#ifdef DBUS_ENABLED
	/* Listen to device additions and removals thru DBus system bus */
	QDBusConnection::systemBus().connect(QString(),
				QString("/org/freedesktop/Hal/Manager"),
				QString("org.freedesktop.Hal.Manager"),
				QString("DeviceAdded"),
				this, SLOT(slotDeviceAdded(const QString&)));
	QDBusConnection::systemBus().connect(QString(),
				QString("/org/freedesktop/Hal/Manager"),
				QString("org.freedesktop.Hal.Manager"),
				QString("DeviceRemoved"),
				this, SLOT(slotDeviceRemoved(const QString&)));
#endif

	/* Search for new widgets */
	rescanWidgets();
}

void EnttecDMXUSBOut::open(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->open();
}

void EnttecDMXUSBOut::close(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->close();
}

#ifdef DBUS_ENABLED
void EnttecDMXUSBOut::slotDeviceAdded(const QString& name)
{
	if (name.contains("/org/freedesktop/Hal/devices/usb_device_403_"))
		rescanWidgets();
}

void EnttecDMXUSBOut::slotDeviceRemoved(const QString& name)
{
	if (name.contains("/org/freedesktop/Hal/devices/usb_device_403_"))
		rescanWidgets();
}
#endif

/****************************************************************************
 * Devices (ENTTEC calls them "widgets" and so shall we)
 ****************************************************************************/

bool EnttecDMXUSBOut::rescanWidgets()
{
	FT_DEVICE_LIST_INFO_NODE* devInfo = NULL;
	FT_STATUS status = FT_OK;
	DWORD num = 0;

	while (m_widgets.isEmpty() == false)
		delete m_widgets.takeFirst();

	/* Find out the number of FTDI devices present */
	status = FT_CreateDeviceInfoList(&num);
	if (status != FT_OK)
	{
		qWarning() << "CreateDeviceInfoList failed:" << status;
		return false;
	}
	else if (num <= 0)
	{
		return true;
	}

	// Allocate storage for list based on numDevices
	devInfo = new FT_DEVICE_LIST_INFO_NODE[num];

	// Get the device information list
	if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
	{
		for (DWORD i = 0; i < num; i++)
		{
			/* Get the device description field so that it can be
			   used to determine the device type (Pro/Open) */
			QString desc(devInfo[i].Description);

			if (desc.contains("pro", Qt::CaseInsensitive) == true)
			{
				/* This is a DMX USB Pro widget */
				EnttecDMXUSBPro* w;
				w = new EnttecDMXUSBPro(this, devInfo[i], i);
				m_widgets.append(w);
			}
			else
			{
				/* This is an Open DMX USB widget */
				EnttecDMXUSBOpen* w;
				w = new EnttecDMXUSBOpen(this, devInfo[i], i);
				Q_ASSERT(w != NULL);
				//w->setSerial(QString("%1").arg(i));
				m_widgets.append(w);
			}
		}
	}

	delete [] devInfo;

	return true;
}

QStringList EnttecDMXUSBOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <EnttecDMXUSBWidget*> it(m_widgets);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
	return list;
}

/****************************************************************************
 * Name
 ****************************************************************************/

QString EnttecDMXUSBOut::name()
{
	return QString("Enttec DMX USB Output");
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void EnttecDMXUSBOut::configure()
{
	int r = QMessageBox::question(NULL, name(),
				tr("Do you wish to re-scan your hardware?"),
				QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
		rescanWidgets();
}

/****************************************************************************
 * Plugin status
 ****************************************************************************/

QString EnttecDMXUSBOut::infoText(t_output output)
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
		if (m_widgets.size() == 0)
		{
			str += QString("<P>");
			str += QString("<B>No devices available</B>. ");
#ifdef WIN32
			str += QString("Make sure you have your Enttec hardware ");
			str += QString("plugged in and the <I>D2XX</I> ");
			str += QString("drivers installed from <a href=\"http://www.ftdichip.com/Drivers/D2XX.htm\">");
			str += QString("http://www.ftdichip.com/Drivers/D2XX.htm</a>. ");
			str += QString("Note that the VCP interface used by Enttec DMX USB Pro ");
			str += QString("is not supported by this plugin.");
#endif
			str += QString("</P>");
		}

		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("ENTTEC DMX USB Pro and Open DMX USB, using the ");
		str += QString("proprietary D2XX interface. See <a href=\"http://www.enttec.com\">");
		str += QString("http://www.enttec.com</a> for more information.");
		str += QString("</P>");
	}
	else if (output < m_widgets.size())
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

void EnttecDMXUSBOut::outputDMX(t_output output, const QByteArray& universe)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->sendDMX(universe);
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusbout, EnttecDMXUSBOut)

