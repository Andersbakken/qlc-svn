/*
  Q Light Controller
  hidjsdevice.cpp

  Copyright (c) Heikki Junnila

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

#include <linux/joystick.h>
#include <linux/input.h>
#include <errno.h>

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hidjsdevice.h"
#include "hidinput.h"

HIDJsDevice::HIDJsDevice(HIDInput* parent, t_input line, const QString& path)
	: HIDDevice(parent, line, path)
{
	init();
}

HIDJsDevice::~HIDJsDevice()
{
	qobject_cast<HIDInput*> (parent())->removePollDevice(this);
}

void HIDJsDevice::init()
{
	if (open() == false)
		return;

	/* Device name */
	char name[128] = "Unknown";
	if (ioctl(m_file.handle(), JSIOCGNAME(sizeof(name)), name) < 0)
	{
		m_name = QString("HID Input %1: %2").arg(m_line + 1)
						    .arg(strerror(errno));
		qWarning() << "Unable to get joystick name:"
			   << strerror(errno);
	}
	else
	{
		m_name = QString("HID Input %1: %2").arg(m_line + 1).arg(name);
	}

	/* Number of axes */
	if (ioctl(m_file.handle(), JSIOCGAXES, &m_axes) < 0)
	{
		m_axes = 0;
		qWarning() << "Unable to get number of axes:"
			   << strerror(errno);
	}

	/* Number of buttons */
	if (ioctl(m_file.handle(), JSIOCGBUTTONS, &m_buttons) < 0)
	{
		m_buttons = 0;
		qWarning() << "Unable to get number of buttons:"
			   << strerror(errno);
	}

	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDJsDevice::open()
{
	bool result = false;

	result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
	if (result == false)
	{
		result = m_file.open(QIODevice::Unbuffered |
				     QIODevice::ReadOnly);
		if (result == false)
		{
			qWarning() << "Unable to open" << m_file.fileName()
				   << ":" << m_file.errorString();
		}
		else
		{
			qDebug() << "Opened" << m_file.fileName()
				 << "in read only mode";
		}
	}

	return result;
}

void HIDJsDevice::close()
{
	m_file.close();
}

QString HIDJsDevice::path() const
{
	return m_file.fileName();
}

bool HIDJsDevice::readEvent()
{
	struct js_event ev;
	HIDInputEvent* e;
	int r;

	r = read(m_file.handle(), &ev, sizeof(struct js_event));
	if (r > 0)
	{
		t_input_channel ch;
		t_input_value val;

		/* Get the event type */
		if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON)
		{
			if (ev.value != 0)
				val = 255;
			else
				val = 0;

			/* Map button channels to start after axes */
			ch = t_input_channel(m_axes + ev.number);

			/* Generate and post an event */
			e = new HIDInputEvent(this, m_line, ch, val, true);
	                QApplication::postEvent(parent(), e);
		}
		else if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
		{
			val = SCALE(double(ev.value),
				    double(-32767), double(32768),
				    double(0), double(255));
			ch = t_input_channel(ev.number);

			e = new HIDInputEvent(this, m_line, ch, val, true);
	                QApplication::postEvent(parent(), e);
		}
		else
		{
			/* Unknown event type */
		}

		return true;
	}
	else
	{
		/* This device seems to be dead */
		e = new HIDInputEvent(this, 0, 0, 0, false);
		QApplication::postEvent(parent(), e);

		return false;
	}
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDJsDevice::infoText()
{
	QString info;

	info += tr("<P><B>%1</B><BR>").arg(m_name);
	info += tr("Axes: %1<BR>").arg(m_axes);
	info += tr("Buttons: %1</P>").arg(m_buttons);

	return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDJsDevice::feedBack(t_input_channel channel, t_input_value value)
{
	/* HID devices don't (yet) support feedback */
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

