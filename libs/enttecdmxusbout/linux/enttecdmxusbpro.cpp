/*
  Q Light Controller
  enttecdmxusbpro.cpp

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

#include <QDebug>
#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(QObject* parent, Ftdi::Context context)
	: QObject(parent)
	, m_context(context)
	, m_name(QString::fromStdString(context.description()))
{
	open();
	extractSerial();
	close();
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
	close();
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBPro::open()
{
	if (isOpen() == false)
	{
		int r;

		r = m_context.open();
		if (r < 0)
		{
			qWarning() << "Unable to open" << uniqueName()
				   << ":" << r;
			return false;
		}

		r = m_context.reset();
		if (r < 0)
		{
			qWarning() << "Unable to reset" << uniqueName()
				   << ":" << r;
			return close();
		}

		r = m_context.set_baud_rate(250000);
		if (r < 0)
		{
			qWarning() << "Unable to set 250kbps baudrate for"
				   << uniqueName() << ":" << r;
			return close();
		}

		r = m_context.set_line_property(BITS_8, STOP_BIT_2, NONE);
		if (r < 0)
		{
			qWarning() << "Unable to set 8N2 serial properties to"
				   << uniqueName() << ":" << r;
			return close();
		}

		r = m_context.set_rts(false);
		if (r < 0)
		{
			qWarning() << "Unable to set RTS line to 0 for"
				   << uniqueName() << ":" << r;
			return close();
		}

		return true;
	}
	else
	{
		/* Already open */
		return true;
	}
}

bool EnttecDMXUSBPro::close()
{
	if (isOpen() == true)
	{
		int r = m_context.close();
		if (r < 0)
		{
			qWarning() << "Unable to close" << uniqueName()
				   << ":" << r;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool EnttecDMXUSBPro::isOpen()
{
	return m_context.is_open();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBPro::name() const
{
	return m_name;
}

QString EnttecDMXUSBPro::serial() const
{
	return m_serial;
}

QString EnttecDMXUSBPro::uniqueName() const
{
	return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

bool EnttecDMXUSBPro::extractSerial()
{
	unsigned char request[] = { 0x7e, 0x0a, 0x00, 0x00, 0xe7 };
	unsigned char reply[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	QString serial("Unknown");
	int r;

	/* Can't get the serial unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to get serial because widget is closed";
		return false;
	}

	/* Write "Get Widget Serial Number Request" message */
	r = m_context.write(request, sizeof(request));
	if (r < 0)
	{
		qWarning() << "Unable to write serial request to"
			   << name() << ":" << r;
		return false;
	}

	/* Read "Get Widget Serial Number Reply" message */
	r = m_context.read(reply, sizeof(reply));
	if (r < 0)
	{
		qWarning() << "Unable to read serial reply from"
			   << name() << ":" << r;
		return false;
	}
	else
	{
		/* Reply message is:
		   { 0x7E 0x0A 0x04 0x00 0xNN, 0xNN, 0xNN, 0xNN 0xE7 }
		   Where 0xNN represent widget's unique serial number in BCD */
		if (reply[0] == 0x7e && reply[1] == 0x0a &&
		    reply[2] == 0x04 && reply[3] == 0x00 &&
		    reply[8] == 0xe7)
		{
			m_serial.sprintf("%x%.2x%.2x%.2x",
				reply[7], reply[6], reply[5], reply[4]);
		}
		else
		{
			qWarning() << "Malformed serial reply from" << name();
			return false;
		}
	}

	return true;
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBPro::sendDMX(const QByteArray& universe)
{
	/* Can't send DMX unless the widget is open */
	if (isOpen() == false)
		open();

	QByteArray request(universe);
	request.prepend(char(0x00)); // DMX start code (Which constitutes the + 1 below)
	request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
	request.prepend((universe.size() + 1) & 0xff); // Data length LSB
	request.prepend(0x06); // Command
	request.prepend(0x7e); // Start byte
	request.append(0xe7); // Stop byte

	/* Write "Output Only Send DMX Packet Request" message */
	int r = m_context.write((unsigned char*) request.data(), request.size());
	if (r < 0)
	{
		qWarning() << "Unable to write DMX request to"
			   << uniqueName() << ":" << r;
		return false;
	}
	else
	{
		return true;
	}
}
