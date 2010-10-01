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

EnttecDMXUSBPro::EnttecDMXUSBPro(QObject* parent, const QString& name,
				 const QString& serial)
	: QObject(parent)
	, m_name(name)
	, m_serial(serial)
	, m_enttecSerial(serial)
{
	bzero(&m_context, sizeof(struct ftdi_context));
	ftdi_init(&m_context);
	extractEnttecSerial(); // Opens and closes the device by itself
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
	close();
	ftdi_deinit(&m_context);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBPro::open()
{
	if (isOpen() == false)
	{
		if (ftdi_usb_open_desc(&m_context, EnttecDMXUSBWidget::VID,
						   EnttecDMXUSBWidget::PID,
						   name().toAscii(),
						   serial().toAscii()) < 0)
		{
			qWarning() << "Unable to open" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return false;
		}

		if (ftdi_usb_reset(&m_context) < 0)
		{
			qWarning() << "Unable to reset" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			close();
			return false;
		}

		if (ftdi_set_line_property(&m_context, BITS_8, STOP_BIT_2, NONE) < 0)
		{
			qWarning() << "Unable to set 8N2 serial properties to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			close();
			return false;
		}

		if (ftdi_set_baudrate(&m_context, 250000) < 0)
		{
			qWarning() << "Unable to set 250kbps baudrate for"
				   << uniqueName() << ":"
				   << ":" << ftdi_get_error_string(&m_context);
			close();
			return false;
		}

		if (ftdi_setrts(&m_context, 0) < 0)
		{
			qWarning() << "Unable to set RTS line to 0 for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			close();
			return false;
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
		if (ftdi_usb_close(&m_context) < 0)
		{
			qWarning() << "Unable to close" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
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
	if (m_context.usb_dev == NULL)
		return false;
	else
		return true;
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

QString EnttecDMXUSBPro::enttecSerial() const
{
	return m_enttecSerial;
}

QString EnttecDMXUSBPro::uniqueName() const
{
	return QString("%1 (S/N: %2)").arg(name()).arg(enttecSerial());
}

bool EnttecDMXUSBPro::extractEnttecSerial()
{
	unsigned char request[] = { 0x7e, 0x0a, 0x00, 0x00, 0xe7 };
	unsigned char reply[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	/* Can't get the serial unless the widget is open */
	open();

	/* Write "Get Widget Serial Number Request" message */
	if (ftdi_write_data(&m_context, request, sizeof(request)) < 0)
	{
		qWarning() << "Unable to write serial request to" << name()
			   << ":" << ftdi_get_error_string(&m_context);
		close();
		return false;
	}

	/* Read "Get Widget Serial Number Reply" message */
	if (ftdi_read_data(&m_context, reply, sizeof(reply)) < 0)
	{
		qWarning() << "Unable to read serial reply from" << name()
			   << ":" << ftdi_get_error_string(&m_context);
		close();
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
			m_enttecSerial.sprintf("%x%.2x%.2x%.2x",
				reply[7], reply[6], reply[5], reply[4]);
			close();
			return true;
		}
		else
		{
			qWarning() << "Malformed serial reply from" << name();
			close();
			return false;
		}
	}
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBPro::sendDMX(const QByteArray& universe)
{
	/* Can't send DMX unless the widget is open */
	if (isOpen() == false)
		return false;

	QByteArray request(universe);
	request.prepend(char(0x00)); // DMX start code (Which constitutes the + 1 below)
	request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
	request.prepend((universe.size() + 1) & 0xff); // Data length LSB
	request.prepend(0x06); // Command
	request.prepend(0x7e); // Start byte
	request.append(0xe7); // Stop byte

	/* Write "Output Only Send DMX Packet Request" message */
	if (ftdi_write_data(&m_context, (unsigned char*) request.data(),
				request.size()) < 0)
	{
		qWarning() << "Unable to write DMX request to" << uniqueName()
			   << ":" << ftdi_get_error_string(&m_context);
		return false;
	}
	else
	{
		return true;
	}
}
