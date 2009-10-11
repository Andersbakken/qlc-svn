/*
  Q Light Controller
  enttecdmxusbproout.cpp

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

#ifndef __APPLE__
#include <QDBusConnection>
#endif

#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

#include "enttecdmxusbproout.h"
#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

void EnttecDMXUSBProOut::init()
{
#ifndef __APPLE__
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

void EnttecDMXUSBProOut::open(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->open();
}

void EnttecDMXUSBProOut::close(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->close();
}

#ifndef __APPLE__
void EnttecDMXUSBProOut::slotDeviceAdded(const QString& name)
{
	if (name.contains("/org/freedesktop/Hal/devices/usb_device_403_6001_"))
		rescanWidgets();
}

void EnttecDMXUSBProOut::slotDeviceRemoved(const QString& name)
{
	if (name.contains("/org/freedesktop/Hal/devices/usb_device_403_6001_"))
		rescanWidgets();
}
#endif

/****************************************************************************
 * Devices (ENTTEC calls them "widgets" and so shall we)
 ****************************************************************************/

void EnttecDMXUSBProOut::rescanWidgets()
{
	QStringList nameFilters;
	QDir dir("/dev/");
	t_output output;
	QString path;

	output = 0;

	QList <EnttecDMXUSBPro*> destroyList(m_widgets);

#ifdef __APPLE__
	nameFilters << "tty.usbserial*";
#else
	nameFilters << "ttyUSB*";
#endif
	QStringListIterator it(dir.entryList(nameFilters,
					     QDir::Files | QDir::System));
	while (it.hasNext() == true)
	{
		path = dir.absolutePath() + QDir::separator() + it.next();

		EnttecDMXUSBPro* w(widget(path));
		if (w != NULL)
		{
			/* This device still exists. Don't destroy it. */
			destroyList.removeAll(w);
		}
		else
		{
			w = new EnttecDMXUSBPro(this, path);
			qDebug() << "Enttec DMX USB Pro at" << path << "added.";
			m_widgets.append(w);
		}
	}

	/* Destroy all devices that weren't found in the rescan */
	while (destroyList.isEmpty() == false)
	{
		EnttecDMXUSBPro* w = destroyList.takeFirst();
		qDebug() << "Enttec DMX USB Pro at" << w->path() << "removed.";
		m_widgets.removeAll(w);
		delete w;
	}
}

EnttecDMXUSBPro* EnttecDMXUSBProOut::widget(const QString& path) const
{
	QListIterator <EnttecDMXUSBPro*> it(m_widgets);
	while (it.hasNext() == true)
	{
		EnttecDMXUSBPro* w(it.next());
		if (w->path() == path)
			return w;
	}

	return NULL;
}

QStringList EnttecDMXUSBProOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <EnttecDMXUSBPro*> it(m_widgets);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
	return list;
}

/****************************************************************************
 * Name
 ****************************************************************************/

QString EnttecDMXUSBProOut::name()
{
	return QString("Enttec DMX USB Pro Output");
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void EnttecDMXUSBProOut::configure()
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

QString EnttecDMXUSBProOut::infoText(t_output output)
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
			str += QString("<B>No devices available</B>. Make ");
			str += QString("sure you have the Enttec DMX USB Pro ");
			str += QString("plugged in and the <I>ftdi_sio</I> ");
			str += QString("kernel module loaded. You need to ");
			str += QString("uninstall the <I>FTDI</I> and ");
			str += QString("<I>Enttec Open DMX USB</I> plugins ");
			str += QString("because the <I>D2XX</I> interface ");
			str += QString("used by them overrides the VCP ");
			str += QString("(Virtual Communications Port) ");
			str += QString("interface used by this plugin.");
			str += QString("</P>");
		}

		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("Enttec DMX USB Pro widgets using the VCP ");
		str += QString("(Virtual Communications Port) driver.");
		str += QString("See <a href=\"http://www.enttec.com\">");
		str += QString("http://www.enttec.com</a> for more ");
		str += QString("information.");
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

void EnttecDMXUSBProOut::writeChannel(t_output output, t_channel channel,
				   t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void EnttecDMXUSBProOut::writeRange(t_output output, t_channel address,
				 t_value* values, t_channel num)
{
	Q_UNUSED(address);

	if (output < m_widgets.size())
	{
		/* Convert the value buffer into a QByteArray */
		QByteArray ba((const char*)values, num);
		m_widgets.at(output)->sendDMX(ba);
	}
}

void EnttecDMXUSBProOut::readChannel(t_output output, t_channel channel,
				  t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void EnttecDMXUSBProOut::readRange(t_output output, t_channel address,
				t_value* values, t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}


/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusbproout, EnttecDMXUSBProOut)

