/*
  Q Light Controller
  usbdmxdevice-unix.cpp

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

#include <sys/ioctl.h>
#include <QDebug>
#include <QFile>

#include "usbdmxdevice-unix.h"

/*****************************************************************************
 * Peperoni Rodin interface macro definitions
 *****************************************************************************/

#define _DMX_RX  (1<<2) /* receiver commands */
#define _DMX_TX  (2<<2) /* transmitter commands */

#define _DMX_MEM       0
#define _DMX_STARTCODE 1
#define _DMX_SLOTS     2
#define _DMX_FRAMES    3

#define VALUE_BLOCKING      (0x0001)

/*****************************************************************************
 * USB-VendorRequests for the USBDMX driver
 *****************************************************************************/

#define DMX_TX_MEM       (_DMX_TX | _DMX_MEM)
#define DMX_TX_SLOTS     (_DMX_TX | _DMX_SLOTS)
#define DMX_TX_STARTCODE (_DMX_TX | _DMX_STARTCODE)
#define DMX_TX_FRAMES    (_DMX_TX | _DMX_FRAMES)
#define DMX_RX_MEM       (_DMX_RX | _DMX_MEM)
#define DMX_RX_SLOTS     (_DMX_RX | _DMX_SLOTS)
#define DMX_RX_STARTCODE (_DMX_RX | _DMX_STARTCODE)
#define DMX_RX_FRAMES    (_DMX_RX | _DMX_FRAMES)
#define DMX_SERIAL       (0x0c)

/*****************************************************************************
 * IOCTLs for client software
 *****************************************************************************/

/* define memory to access with next read()/write() calls
 * valid values are: "DMX_TX_MEM" or "DMX_TX_MEM" */
#define DMX_MEM_MAP_SET      1

/* get current memory map */
#define DMX_MEM_MAP_GET	     2

/* get transmitter slots, 2 bytes */
#define DMX_TX_SLOTS_GET     3

/* set transmitter slots, 2 bytes */
#define DMX_TX_SLOTS_SET     4

/* get transmitter startcode, 1 byte */
#define DMX_TX_STARTCODE_GET 5

/* set transmitter startcode, 1 byte */
#define DMX_TX_STARTCODE_SET 6

/* get transmitter frame counter, 4 bytes,
 * initialized to zero on interface power-up
 * this counter can overrun */
#define DMX_TX_FRAMES_GET    7

/* get number of slots received with the last complete frame, 2 bytes */
#define DMX_RX_SLOTS_GET     8

/* get receiver startcode, 1 byte, frames with other startcodes then
 * then this are ignored */
#define DMX_RX_STARTCODE_GET 9

/* set receiver startcode, 1 byte, only frames with this startcode are
 * received */
#define DMX_RX_STARTCODE_SET 10

/* get receiver frame counter, 4 bytes
 * initialized to zero on interface power-up
 * this counter can overrun */
#define DMX_RX_FRAMES_GET    11

/* read interfaces serial number */
#define DMX_SERIAL_GET       14

/* read if read()/write() operations are executed blocking */
#define DMX_BLOCKING_GET     16

/* set following read()/write() operations to blocking */
#define DMX_BLOCKING_SET     17

/* change USB configuration of the interface */
#define CONFIG_SET           18

/* get interface type (DeviceID), known values are listed below */
#define DMX_TYPE_GET         19

/*****************************************************************************
 * Inteface types (possible return values of DMX_TYPE_GET ioctl)
 *****************************************************************************/

#define XSWITCH      (0x01)
#define RODIN1       (0x02)
#define RODIN2       (0x03)
#define USBDMX21     (0x04)

/****************************************************************************
 * Initialization
 ****************************************************************************/

USBDMXDevice::USBDMXDevice(QObject* parent, const QString& path)
	: QObject(parent)
{
	Q_ASSERT(path.isEmpty() == false);
	Q_ASSERT(output != KOutputInvalid);

	m_path = path;
	m_file.setFileName(m_path);

	extractName();
}

USBDMXDevice::~USBDMXDevice()
{
	close();
}

void USBDMXDevice::extractName()
{
	bool needToClose = false;

	if (m_file.isOpen() == false)
	{
		needToClose = true;
		open();
	}

	if (m_file.isOpen() == false)
	{
		needToClose = false;
		m_name = QString("Nothing");
	}
	else
	{
		int type = ::ioctl(m_file.handle(), DMX_TYPE_GET, NULL);
		switch (type)
		{
		case XSWITCH:
			m_name = QString("X-Switch");
			break;
		case RODIN1:
			m_name = QString("Rodin 1");
			break;
		case RODIN2:
			m_name = QString("Rodin 2");
			break;
		case USBDMX21:
			m_name = QString("USBDMX21");
			break;
		default:
			m_name = QString("Unknown");
			break;
		}
	}

	if (needToClose == true)
		close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString USBDMXDevice::name() const
{
	return m_name;
}

QString USBDMXDevice::path() const
{
	return m_path;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void USBDMXDevice::open()
{
	m_file.unsetError();
	if (m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered) == true)
	{
		/* Set writing mode */
		::ioctl(m_file.handle(), DMX_MEM_MAP_SET, DMX_TX_MEM);

		/* Set DMX startcode */
		::ioctl(m_file.handle(), DMX_TX_STARTCODE_SET, 0);
	}
	else
	{
		qWarning() << QString("Unable to open %1: %2").arg(m_name)
				.arg(m_file.errorString());
	}
}

void USBDMXDevice::close()
{
	m_file.unsetError();
	m_file.close();

	if (m_file.error() != QFile::NoError)
		qWarning() << QString("Unable to close %1: %2").arg(m_name)
				.arg(m_file.errorString());
}

/****************************************************************************
 * Write
 ****************************************************************************/

void USBDMXDevice::writeRange(t_value* values, t_channel num)
{
	Q_UNUSED(num);

	m_file.seek(0);
	if (m_file.write((const char*) values, 512) == -1)
		qWarning() << "writeRange error:" << m_file.errorString();
}
