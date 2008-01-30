/*
  Q Light Controller
  hideventdevice.cpp

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

#include <linux/input.h>

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>
#include <errno.h>

#include "hideventdevice.h"
#include "hidinput.h"

/**
 * This macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND 
 * operation with a byte that only has the relevant bit set. 
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit / 8] & (1 << (bit % 8)))

HIDEventDevice::HIDEventDevice(HIDInput* parent, const QString& path) 
	: HIDDevice(parent, "HIDEventDevice", path)
{
}

HIDEventDevice::~HIDEventDevice()
{
	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDEventDevice::open()
{
	bool result = false;

	qDebug("************************************************************");
	qDebug("* Device file:" + m_file.name());

	result = m_file.open(IO_Raw | IO_ReadWrite);
	if (result == false)
	{
		qWarning("Unable to open %s in R/W mode: %s.",
			 (const char*) m_file.name(),
			 (const char*) m_file.errorString());
		
		result = m_file.open(IO_Raw | IO_ReadOnly);
		if (result == false)
		{
			qWarning("Unable to open %s in R/O mode: %s.\n",
				 (const char*) m_file.name(),
				 (const char*) m_file.errorString());
		}
	}

	if (result == true)
	{
		/* An event device has been opened in RW or RO mode now.
		   Either way, these should be available. */

		/* Device name */
		char name[256] = "Unknown";
		if (ioctl(m_file.handle(), EVIOCGNAME(sizeof(name)), name) <= 0)
		{
			m_name = QString(strerror(errno));
			perror("ioctl EVIOCGNAME");
		}
		else
		{
			m_name = QString(name);
			qDebug("* Device name:" + m_name);
		}

		/* Device info */
		if (ioctl(m_file.handle(), EVIOCGID, &m_deviceInfo))
			perror("ioctl EVIOCGID");
		
		/* Supported event types */
		if (ioctl(m_file.handle(), EVIOCGBIT(0, sizeof(m_eventTypes)),
			  m_eventTypes) <= 0)
			perror("ioctl EVIOCGBIT");
		else
			getCapabilities();
	}

	return result;
}

void HIDEventDevice::getCapabilities()
{
	int i = 0;

	printf("* Supported event types:\n");

	for (i = 0; i < EV_MAX; i++)
	{
		if (test_bit(i, m_eventTypes))
		{
			/* this means that the bit is set in the
			   event types list */
			printf("\tEvent type 0x%02x ", i);
			switch (i)
			{
			case EV_KEY:
				printf("\t (Keys or Buttons)\n");
				break;
			case EV_ABS:
				printf("\t (Absolute Axes)\n");
				break;
			case EV_LED:
				printf("\t (LEDs)\n");
				break;
			case EV_REP:
				printf("\t (Repeat)\n");
				break;
			default:
				printf("\t (Unknown event type: 0x%04hx)\n", i);
			}
		}
	}
}

void HIDEventDevice::close()
{
	m_file.close();
}

QString HIDEventDevice::path() const
{
	return m_file.name();
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDEventDevice::infoText()
{
	QString info;
	QString str;
	int version;

	info += QString("<TR>");

	/* File name */
	info += QString("<TD>");
	info += m_file.name();
	info += QString("</TD>");

	if (m_file.isOpen() == true)
	{
		/* Name */
		info += QString("<TD>");
		info += m_name;
		info += QString("</TD>");

		/* Mode */
		info += QString("<TD>");
		if (m_file.isReadable() == true)
			info += QString("Read");
		if (m_file.isWritable() == true)
			info += QString("/Write");
		else
			info += QString(" Only");
		info += QString("</TD>");
	}
	else
	{
		/* File is not open, put an error here. */
		info += QString("<TD>");
		info += m_file.errorString();
		info += QString("</TD>");
		info += QString("<TD>");
		info += m_file.errorString();
		info += QString("</TD>");
	}

	info += QString("</TR>");

	return info;
}

t_input_channel HIDEventDevice::channels()
{
	return 0;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDEventDevice::feedBack(t_input_channel channel, t_input_value value)
{
}

