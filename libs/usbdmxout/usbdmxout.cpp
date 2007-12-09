/*
  Q Light Controller
  usbdmxout.cpp
  
  Copyright (c) Christian Suehs
                Stefan Krumm
		Heikki Junnila
  
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <qptrlist.h>
#include <qapplication.h>
#include <qthread.h>
#include <qstring.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qdir.h>

#include "usbdmxout.h"
#include "configureusbdmxout.h"
#include "common/filehandler.h"

static QMutex _mutex;

#define CONF_FILE "usbdmxout.conf"
#define DEVICE_DIR "/dev/"

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

/*****************************************************************************
 * UsbDmxOut plugin implementation
 *****************************************************************************/

extern "C" OutputPlugin* create()
{
	return new USBDMXOut();
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

USBDMXOut::USBDMXOut() : OutputPlugin()
{
	m_name = QString("USB DMX Output");
	m_type = OutputType;
	m_version = 0x00010100;

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
		m_devices[i] = -1;

	for (t_channel ch = 0; ch < MAX_USBDMX_DEVICES * 512; ch++)
		m_values[ch] = 0;

	open();
}

USBDMXOut::~USBDMXOut()
{
	close();
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

int USBDMXOut::open()
{
	QString path;

	/* Close all connections first */
	close();

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		path.sprintf("/dev/usbdmx%d", i);
		m_devices[i] = ::open(static_cast<const char*> (path), O_RDWR);
		if (m_devices[i] >= 0)
			qDebug(QString("Found USB2DMX device from ") + path);
	}

	return 0;
}

int USBDMXOut::close()
{
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		if (m_devices[i] != -1)
			::close(m_devices[i]);
		m_devices[i] = -1;
	}

	return 0;
}

int USBDMXOut::outputs()
{
	return MAX_USBDMX_DEVICES;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int USBDMXOut::configure(QWidget* parentWidget)
{
	ConfigureUSBDMXOut conf(parentWidget, this);
	conf.exec();
	return 0;
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString USBDMXOut::infoText()
{
	QString info;
	QString s;

	/* HTML page Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/* Version information */
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD><B>Version</B></TD>");
	info += QString("<TD>");
	s.setNum((version() >> 16) & 0xff);
	info += s + QString(".");
	s.setNum((version() >> 8) & 0xff);
	info += s + QString(".");
	s.setNum(version() & 0xff);
	info += s + QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * Outputs
	 *********************************************************************/

	/* Title */
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += QString("Output line");
	info += QString("</FONT>");
	info += QString("</TD>");

	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += QString("Device name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	/* Output lines */
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		if (m_devices[i] >= 0)
		{
			info += QString("<TR>");
			s.sprintf("%d", i + 1);
			info += QString("<TD>" + s + "</TD>");
			s.sprintf("/dev/usbdmx%d", i);
			info += QString("<TD>" + s + "</TD>");
			info += QString("</TR>");
		}
	}
	info += QString("</TABLE>");

	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

int USBDMXOut::writeChannel(t_channel channel, t_value value)
{
	int ifaceNo = int(channel / 512);
	int channelNo = channel % 512;
	int r = 0;
	
	_mutex.lock();

	m_values[channel] = value;
	
	if (m_devices[ifaceNo] != -1)
	{
		::ioctl(m_devices[ifaceNo], DMX_MEM_MAP_SET, DMX_TX_MEM);
		::lseek(m_devices[ifaceNo], channelNo, SEEK_SET);

		r = ::write(m_devices[ifaceNo], &value, 1);
		if (r == -1)
			::perror("USBDMXOut::writeChannel");
	}

	_mutex.unlock();

	return r;
}

int USBDMXOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	int ifaceNo = int(address / 512);
	int firstChannel = address % 512;
	int r = 0;
	
	_mutex.lock();

	memcpy(m_values + address, values, num * sizeof(t_value));

	if (m_devices[ifaceNo] != -1)
	{
		::ioctl(m_devices[ifaceNo], DMX_MEM_MAP_SET, DMX_TX_MEM);
		::lseek(m_devices[ifaceNo], firstChannel, SEEK_SET);

		r = ::write(m_devices[ifaceNo], values, num);
		if (r == -1)
			perror("USBDMXOut::writeRange");
	}
  
	_mutex.unlock();

	return r;
}

int USBDMXOut::readChannel(t_channel channel, t_value &value)
{
	_mutex.lock();
	value = m_values[channel];
	_mutex.unlock();

	return 0;
}

int USBDMXOut::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	_mutex.unlock();

	return 0;
}
