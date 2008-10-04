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

USBDMXDevice::USBDMXDevice(QObject* parent, const QString &path,
			   t_output output) : QObject(parent)
{
	Q_ASSERT(path.isEmpty() == false);
	Q_ASSERT(output != KOutputInvalid);

	m_path = path;
	m_output = output;

	for (t_channel i = 0; i < sizeof(m_values); i++)
		m_values[i] = 0;

	m_file.setFileName(m_path);
	m_name = extractName();
}

USBDMXDevice::~USBDMXDevice()
{
	close();
}

QString USBDMXDevice::extractName()
{
	QString name;

	if (open() == false)
	{
		name = QString("%1: Nothing").arg(m_output + 1);
	}
	else
	{
		int type = ::ioctl(m_file.handle(), DMX_TYPE_GET, NULL);
		switch (type)
		{
		case XSWITCH:
			name = QString("%1: X-Switch").arg(m_output + 1);
			break;
		case RODIN1:
			name = QString("%1: Rodin 1").arg(m_output + 1);
			break;
		case RODIN2:
			name = QString("%1: Rodin 2").arg(m_output + 1);
			break;
		case USBDMX21:
			name = QString("%1: USBDMX21").arg(m_output + 1);
			break;
		default:
			name = QString("%1: Unknown").arg(m_output + 1);
			break;
		}
	}

	close();

	return name;
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

t_output USBDMXDevice::output() const
{
	return m_output;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool USBDMXDevice::open()
{
	m_file.unsetError();
	if (m_file.open(QIODevice::ReadWrite) == true)
	{
		/* Set writing mode */
		::ioctl(m_file.handle(), DMX_MEM_MAP_SET, DMX_TX_MEM);
		return true;
	}
	else
	{
		qWarning() << QString("Unable to open USBDMX %1: %2")
			.arg(m_output).arg(m_file.errorString());
		return false;
	}
}

bool USBDMXDevice::close()
{
	m_file.unsetError();
	m_file.close();
	if (m_file.error() != QFile::NoError)
	{
		qWarning() << QString("Unable to close USBDMX %1: %2")
			.arg(m_output).arg(m_file.errorString());
		return false;
	}
	else
	{
		return true;
	}
}

/****************************************************************************
 * Read & write
 ****************************************************************************/

void USBDMXDevice::write(t_channel channel, t_value value)
{
	m_mutex.lock();
	m_values[channel] = value;
	m_file.seek(channel);
	if (m_file.write((const char*) &value, 1) == -1)
		qWarning() << "USBDMX write error:"
			   << m_file.errorString();
	m_mutex.unlock();
}

void USBDMXDevice::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(m_values + address, values, num);
	m_file.seek(address);
	if (m_file.write((const char*) values, num) == -1)
		qWarning() << "USBDMX writeRange error:"
			   << m_file.errorString();
	m_mutex.unlock();
}

void USBDMXDevice::read(t_channel channel, t_value* value)
{
	m_mutex.lock();
	*value = m_values[channel];
	m_mutex.unlock();
}

void USBDMXDevice::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(values, m_values + address, num);
	m_mutex.unlock();
}
