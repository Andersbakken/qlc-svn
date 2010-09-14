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

EnttecDMXUSBPro::EnttecDMXUSBPro(QObject* parent,
				 const FT_DEVICE_LIST_INFO_NODE& info,
				 DWORD id)
	: QObject(parent)
{
	m_name = QString(info.Description);
	m_handle = NULL;
	m_id = id;

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
		/* Attempt to open the device */
		FT_STATUS status = FT_Open(m_id, &m_handle);
		if (status == FT_OK)
		{
			if (initializePort() == false)
			{
				qWarning() << "Unable to initialize port."
					   << "Closing widget.";
				close();
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			qWarning() << "Unable to open" << name()
				   << ". Error:" << status;
			return false;
		}
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
		FT_STATUS status = FT_Close(m_handle);
		if (status == FT_OK)
		{
			m_handle = NULL;
			return true;
		}
		else
		{
			qWarning() << "Unable to close" << name()
				   << ". Error:" << status;
			return false;
		}
	}
	else
	{
		return true;
	}
}

bool EnttecDMXUSBPro::isOpen()
{
	if (m_handle != NULL)
		return true;
	else
		return false;
}

bool EnttecDMXUSBPro::initializePort()
{
	FT_STATUS status = FT_OK;

	/* Can't get the serial unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to initialize because widget is closed";
		return false;
	}

	/* Reset the widget */
	status = FT_ResetDevice(m_handle);
	if (status != FT_OK)
	{
		qWarning() << "FT_ResetDevice:" << status;
		return false;
	}

	/* Set the baud rate. 12 will give us 250Kbits */
	status = FT_SetDivisor(m_handle, 12);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetDivisor:" << status;
		return false;
	}

	/* Set data characteristics */
	status = FT_SetDataCharacteristics(m_handle, FT_BITS_8,
					   FT_STOP_BITS_2, FT_PARITY_NONE);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetDataCharacteristics:" << status;
		return false;
	}

	/* Set flow control */
	status = FT_SetFlowControl(m_handle, FT_FLOW_NONE, 0, 0);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetFlowControl:" << status;
		return false;
	}

	/* Set RS485 for sending */
	FT_ClrRts(m_handle);

	/* Clear TX RX buffers */
	FT_Purge(m_handle, FT_PURGE_TX | FT_PURGE_RX);

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

QString EnttecDMXUSBPro::uniqueName() const
{
	return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

bool EnttecDMXUSBPro::extractSerial()
{
	unsigned char request[] = { 0x7e, 0x0a, 0x00, 0x00, 0xe7 };
	unsigned char reply[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	DWORD written = 0;
	DWORD read = 0;
	FT_STATUS status = FT_OK;
	QString serial("Unknown");

	/* Can't get the serial unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to get serial because widget is closed";
		return false;
	}

	/* Write "Get Widget Serial Number Request" message */
	status = FT_Write(m_handle, request, sizeof(request), &written);
	if (status != FT_OK)
	{
		qWarning() << "Unable to write serial request to"
			   << name() << ". Error:" << status;
		return false;
	}

	/* Read "Get Widget Serial Number Reply" message */
	status = FT_Read(m_handle, reply, sizeof(reply), &read);
	if (status == FT_OK)
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
	else
	{
		qWarning() << "Unable to read serial reply from"
			   << name() << ". Error:" << status;
		return false;
	}

	return true;
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBPro::sendDMX(const QByteArray& universe)
{
	DWORD written = 0;
	FT_STATUS status = FT_OK;

	/* Can't send DMX unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to send DMX because widget is closed";
		return false;
	}

	QByteArray request(universe);
	request.prepend(char(0x00)); // DMX start code (Which constitutes the + 1 below)
	request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
	request.prepend((universe.size() + 1) & 0xff); // Data length LSB
	request.prepend(0x06); // Command
	request.prepend(0x7e); // Start byte
	request.append(0xe7); // Stop byte

	/* Write "Output Only Send DMX Packet Request" message */
	status = FT_Write(m_handle, request.data(), request.size(),
			  &written);
	if (status != FT_OK)
	{
		qWarning() << "Unable to write DMX request to"
			   << name() << ". Error:" << status;
		return false;
	}
	else
	{
		return true;
	}
}
