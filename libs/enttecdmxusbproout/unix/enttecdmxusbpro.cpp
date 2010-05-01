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

#include <sys/ioctl.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <QDebug>

#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(QObject* parent, const QString& path)
	: QObject(parent),
	m_file(-1),
	m_serial("Unknown"),
	m_name("DMX USB Pro"),
	m_path(path)
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
		/* Attempt to open the device */
		m_file = ::open(m_path.toUtf8(),
				O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (m_file != -1)
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
			qWarning() << "Error opening " << m_path << ":"
				   << strerror(errno) << "(" << errno << ").";
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
		::close(m_file);
		m_file = -1;
	}

	return true;
}

bool EnttecDMXUSBPro::isOpen() const
{
	if (m_file != -1)
		return true;
	else
		return false;
}

bool EnttecDMXUSBPro::initializePort()
{
	struct termios tio;

	/* Nothing to do if the port isn't open */
	if (isOpen() == false)
		return false;

	::bzero(&tio, sizeof(tio));

	/* Disable all input & output processing so \0 won't be understood as
	   a terminating character. */
	::cfmakeraw(&tio);

	/* Set R/W timeout to 1 sec */
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 10;

	/* Set exclusive access */
	if (::ioctl(m_file, TIOCEXCL) == -1)
	{
		qWarning() << "Error setting TIOCEXCL on" << m_path << ":"
			   << strerror(errno) << "(" << errno << ").";
		return false;
        }

	/* Lose non-blocking R/W mode */
	if (::fcntl(m_file, F_SETFL, 0) == -1)
	{
		qWarning() << "Error clearing O_NONBLOCK on" << m_path << ":"
			   << strerror(errno) << "(" << errno << ").";
		return false;
        }

	/* 8N2 - 8bits, no parity, 2 stop bits, maximum speed */
#ifdef __APPLE__
	::cfsetspeed(&tio, B230400);
	tio.c_cflag |= CRTSCTS | CS8 | CSTOPB | CLOCAL | CREAD;
#else
	tio.c_cflag = __MAX_BAUD | CRTSCTS | CS8 | CSTOPB | CLOCAL | CREAD;
#endif
	::tcflush(m_file, TCIFLUSH);
	if (::tcsetattr(m_file, TCSANOW, &tio) == -1)
	{
		qWarning() << "Error setting tty attributes to" << m_path << ":"
			   << strerror(errno) << "(" << errno << ").";
		return false;
	}

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
	int writtenBytes;
	int readBytes;

	/* Can't get the serial unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to get serial because widget is closed";
		return false;
	}

	/* Write "Get Widget Serial Number Request" message */
	char request[] = { 0x7e, 0x0a, 0x00, 0x00, 0xe7 };
	writtenBytes = ::write(m_file, request, sizeof(request));
	if (writtenBytes != sizeof(request))
	{
		qWarning() << "Unable to write serial number request to"
			   << m_path << ":" << strerror(errno)
			   << "(" << errno << ").";
		return false;
	}

	/* Wait for the dongle to settle */
	usleep(1000);

	/* Wait for "Get Widget Serial Number Reply" message to be available */
	char reply[9];
	bzero(reply, sizeof(reply));
	readBytes = ::read(m_file, reply, 9);
	if (readBytes != 9)
	{
		qWarning() << "Unable to read serial number reply from"
			   << m_path << ":" << strerror(errno)
			   << "(" << errno << ").";
		return false;
	}

	/* Reply message is:
	   { 0x7E 0x0A 0x04 0x00 0xNN, 0xNN, 0xNN, 0xNN 0xE7 }
	   Where 0xNN represent widget's unique serial number in BCD */
	if (reply[0] == char(0x7e) && reply[1] == char(0x0a) &&
	    reply[2] == char(0x04) && reply[3] == char(0x00) &&
	    reply[8] == char(0xe7))
	{
		m_serial.sprintf("%x%.2x%.2x%.2x", char(reply[7]),
			char(reply[6]), char(reply[5]), char(reply[4]));
		return true;
	}
	else
	{
		qWarning() << "Malformed serial reply from" << name();
		return false;
	}
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBPro::sendDMX(const QByteArray& universe)
{
	int writtenBytes;

	/* Can't send DMX unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to send DMX because widget is closed";
		return false;
	}

	QByteArray request(universe);
	request.prepend(char(0x00)); // DMX start code (which constitutes the + 1 below)
	request.prepend(char(((universe.size() + 1) >> 8) & 0xff)); // Data length MSB
	request.prepend(char((universe.size() + 1) & 0xff)); // Data length LSB
	request.prepend(char(0x06)); // Command
	request.prepend(char(0x7e)); // Start byte
	request.append(char(0xe7));  // Stop byte

	writtenBytes = ::write(m_file, request.data(), request.size());
	if (writtenBytes != request.size())
	{
		qWarning() << "Unable to write DMX data to"
			   << m_path << ":" << strerror(errno)
			   << "(" << errno << ").";
		return false;
	}
	else
	{
		return true;
	}
}
