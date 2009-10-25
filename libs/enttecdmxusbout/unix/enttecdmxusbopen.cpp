/*
  Q Light Controller
  enttecdmxusbopen.cpp

  Copyright (C) Heikki Junnila
		Christopher Staite

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
#include "enttecdmxusbopen.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(QObject* parent,
				   const FT_DEVICE_LIST_INFO_NODE& info)
	: QThread(parent),
	m_info(info),
	m_running(false),
	m_universe(QByteArray(512, 0))
{
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
	close();
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open()
{
	if (isOpen() == false)
	{
		/* Attempt to open the device */
		FT_STATUS status = FT_Open(0, &m_info.ftHandle);
		if (status == FT_OK)
		{
			if (initializePort() == false)
			{
				qWarning() << "Unable to initialize port."
					   << "Closing widget.";
				close();
				return false;
			}

			if (isRunning() == false)
				start();

			return true;
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

bool EnttecDMXUSBOpen::close()
{
	if (isOpen() == true)
	{
		/* Stop the writer thread */
		if (isRunning() == true)
			stop();

		FT_STATUS status = FT_Close(m_info.ftHandle);
		if (status == FT_OK)
		{
			m_info.ftHandle = 0;
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

bool EnttecDMXUSBOpen::isOpen() const
{
	if (m_info.ftHandle != 0)
		return true;
	else
		return false;
}

bool EnttecDMXUSBOpen::initializePort()
{
	FT_STATUS status = FT_OK;

	/* Reset the widget */
	status = FT_ResetDevice(m_info.ftHandle);
	if (status != FT_OK)
	{
		qWarning() << "FT_ResetDevice:" << status;
		return false;
	}

	/* Set the baud rate. 12 will give us 250Kbits */
	status = FT_SetDivisor(m_info.ftHandle, 12);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetDivisor:" << status;
		return false;
	}

	/* Set data characteristics */
	status = FT_SetDataCharacteristics(m_info.ftHandle, FT_BITS_8,
					   FT_STOP_BITS_2, FT_PARITY_NONE);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetDataCharacteristics:" << status;
		return false;
	}

	/* Set flow control */
	status = FT_SetFlowControl(m_info.ftHandle, FT_FLOW_NONE, 0, 0);
	if (status != FT_OK)
	{
		qWarning() << "FT_SetFlowControl:" << status;
		return false;
	}

	/* Set RS485 for sending */
	FT_ClrRts(m_info.ftHandle);

	/* Clear TX RX buffers */
	FT_Purge(m_info.ftHandle, FT_PURGE_TX | FT_PURGE_RX);

	return true;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBOpen::name() const
{
	return m_info.Description;
}

QString EnttecDMXUSBOpen::serial() const
{
	return m_serial;
}

QString EnttecDMXUSBOpen::uniqueName() const
{
	return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBOpen::sendDMX(const QByteArray& universe)
{
	Q_ASSERT(universe.size() == 512);

	/* Can't send DMX unless the widget is open */
	if (isOpen() == false || isRunning() == false)
	{
		qWarning() << "Unable to send DMX because widget is closed.";
		return false;
	}
	else
	{
		m_universe = universe;
		return true;
	}
}

/****************************************************************************
 * Thread
 ****************************************************************************/

void EnttecDMXUSBOpen::stop()
{
	if (isRunning() == true)
	{
		m_running = false;
		wait();
	}
}

void EnttecDMXUSBOpen::run()
{
	ULONG written = 0;
	FT_STATUS status = FT_OK;
	unsigned char startByte = 0;

	/* Wait for device to settle if the port was opened just recently */
	usleep(1000);

	while (m_running == true)
	{
		if (isOpen() == false)
		{
			qWarning() << "Writer thread terminated."
				   << "Port closed unexpectedly.";
			m_running = false;
			return;
		}

		status = FT_SetBreakOn(m_info.ftHandle);
		if (status != FT_OK)
		{
			qWarning() << "FT_SetBreakOn:" << status;
			goto framesleep;
		}

		usleep(88);

		status = FT_SetBreakOff(m_info.ftHandle);
		if (status != FT_OK)
		{
			qWarning() << "FT_SetBreakOff:" << status;
			goto framesleep;
		}

		usleep(8);

		status = FT_Write(m_info.ftHandle, &startByte, 1, &written);
		if (status != FT_OK)
		{
			qWarning() << "FT_Write startbyte:" << status;
			goto framesleep;
		}

		status = FT_Write(m_info.ftHandle, m_universe.data(),
				  m_universe.size(), &written);
		if (status != FT_OK)
		{
			qWarning() << "FT_Write universe:" << status;
			goto framesleep;
		}

framesleep:
		usleep(24000);
	}
}

