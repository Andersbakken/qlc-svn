/*
  Q Light Controller
  enttecdmxusbout.cpp

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

#ifdef DBUS_ENABLED
#include <QDBusConnection>
#endif

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "../unix/enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbout.h"
#include "ftdi.h"

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
	QRegExp re("/org/freedesktop/Hal/devices/usb_device_403_*_*_if*_serial_usb_*");
	re.setPatternSyntax(QRegExp::Wildcard);
	if (name.contains(re) == true)
		rescanWidgets();
}

void EnttecDMXUSBOut::slotDeviceRemoved(const QString& name)
{
	QRegExp re("/org/freedesktop/Hal/devices/usb_device_403_*_*_if*_serial_usb_*");
	re.setPatternSyntax(QRegExp::Wildcard);
	if (name.contains(re) == true)
		rescanWidgets();
}
#endif

/****************************************************************************
 * Devices (ENTTEC calls them "widgets" and so shall we)
 ****************************************************************************/

bool EnttecDMXUSBOut::rescanWidgets()
{
	QStringList openList;

	// Construct a list of lines that were open before rescan so that
	// they can be re-opened again after the scan.
	while (m_widgets.isEmpty() == false)
	{
		EnttecDMXUSBWidget* w = m_widgets.takeFirst();
		Q_ASSERT(w != NULL);
		if (w->isOpen() == true)
			openList << w->serial();
		delete w;
	}

	struct ftdi_device_list* list = 0;
	struct ftdi_context ftdi;
	ftdi_init(&ftdi);
	ftdi_usb_find_all(&ftdi, &list, EnttecDMXUSBWidget::VID,
					EnttecDMXUSBWidget::PID);
	while (list != NULL)
	{
		struct usb_device* dev = list->dev;
		Q_ASSERT(dev != NULL);

		char serial[256];
		char name[256];
		char vendor[256];

		ftdi_usb_get_strings(&ftdi, dev,
				     vendor, sizeof(vendor),
				     name, sizeof(name),
				     serial, sizeof(serial));

		if (QString(vendor).toUpper() == QString("ENTTEC"))
		{
			m_widgets.append(new EnttecDMXUSBPro(this,
					 QString(name), QString(serial)));
		}
		else
		{
			m_widgets.append(new EnttecDMXUSBOpen(this,
					 QString(name), QString(serial)));
		}

		list = list->next;
	}

	ftdi_deinit(&ftdi);

	// Re-open lines that were open before rescan
	while (openList.isEmpty() == false)
	{
		EnttecDMXUSBWidget* w = widget(openList.takeFirst());
		if (w != NULL)
			w->open();
	}

	return true;
}

EnttecDMXUSBWidget* EnttecDMXUSBOut::widget(const QString& serial) const
{
	foreach (EnttecDMXUSBWidget* w, m_widgets)
	{
		if (w->serial() == serial)
			return w;
	}

	return NULL;
}

QStringList EnttecDMXUSBOut::outputs()
{
	QStringList list;
	int i = 1;

	foreach (EnttecDMXUSBWidget* w, m_widgets)
	{
		Q_ASSERT(w != NULL);
		list << QString("%1: %2").arg(i++).arg(w->uniqueName());
	}
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
			str += QString("<P><B>");
			str += tr("No devices available.");
			str += QString("</B></P>");
		}

		str += QString("<P>");
		str += tr("This plugin provides DMX output support for "
			  "ENTTEC Open/Pro DMX USB, using the open source "
			  "libftdi library. See %1 for more information.")
			  .arg("<a href=\"http://www.enttec.com\">"
			       "http://www.enttec.com</a>");
		str += QString("</P>");
	}
	else if (output < m_widgets.size())
	{
		str += QString("<H3>%1</H3>").arg(outputs()[output]);
		str += QString("<P>");
		str += tr("Device is operating correctly.");
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

