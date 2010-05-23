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

#include <QDBusConnection>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "../unix/enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbout.h"
#include "ftdi.hpp"

/****************************************************************************
 * Initialization
 ****************************************************************************/

void EnttecDMXUSBOut::init()
{
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

/****************************************************************************
 * Devices (ENTTEC calls them "widgets" and so shall we)
 ****************************************************************************/

bool EnttecDMXUSBOut::rescanWidgets()
{
	// Treat all widgets initially as not present
	QList <EnttecDMXUSBWidget*> destroyList(m_widgets);

	Ftdi::List* list = Ftdi::List::find_all(EnttecDMXUSBWidget::VID,
						EnttecDMXUSBWidget::PID);
	for (Ftdi::List::iterator it = list->begin(); it != list->end(); it++)
	{
		Ftdi::Context ctx(*it);

		QString vendor = QString::fromStdString(ctx.vendor());
		QString description = QString::fromStdString(ctx.description());
		QString serial = QString::fromStdString(ctx.serial());

                if (vendor.toUpper() == QString("FTDI"))
		{
			// DMX USB Open
			EnttecDMXUSBWidget* w = widget(serial);
			if (w != NULL)
				destroyList.removeAll(w);
			else
				m_widgets.append(new EnttecDMXUSBOpen(this, ctx));
		}
		else if (vendor.toUpper() == QString("ENTTEC"))
		{
			// DMX USB Pro is ignored by this plugin for now
			EnttecDMXUSBWidget* w = widget(serial);
			if (w != NULL)
				destroyList.removeAll(w);
			else
				m_widgets.append(new EnttecDMXUSBPro(this, ctx));
		}
        }

	list->clear();

	// Destroy all widgets that are no longer present
	foreach (EnttecDMXUSBWidget* w, destroyList)
	{
		m_widgets.removeAll(w);
		delete w;
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
			str += QString("<P>");
			str += QString("<B>No devices available/detected</B>. ");
			str += QString("</P>");
		}

		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("ENTTEC Open DMX USB, using the open source libftdi ");
		str += QString("library. See <a href=\"http://www.enttec.com\">");
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

