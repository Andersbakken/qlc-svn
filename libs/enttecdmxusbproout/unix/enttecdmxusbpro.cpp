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

#include <termios.h>
#include <unistd.h>
#include <QDebug>

#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(QObject* parent, const QString& path)
	: QObject(parent),
	m_file(path),
	m_serial("Unknown"),
	m_name("DMX USB Pro")
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
		if (m_file.open(QIODevice::ReadWrite |
				QIODevice::Unbuffered) == true)
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
			qWarning() << "Unable to open" << name() << ":"
				   << m_file.errorString();
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
		m_file.close();

	return true;
}

bool EnttecDMXUSBPro::isOpen() const
{
	return m_file.isOpen();
}

bool EnttecDMXUSBPro::initializePort()
{
	struct termios tio;

	bzero(&tio, sizeof(tio));

	tio.c_cflag = __MAX_BAUD | CRTSCTS | CS8 | CSTOPB | CLOCAL | CREAD;
	tio.c_iflag = IGNPAR | ICRNL;
	tio.c_oflag = 0;
	tio.c_lflag = 0;

	tio.c_cc[VINTR]    = 0; /* Ctrl-c */
	tio.c_cc[VQUIT]    = 0; /* Ctrl-\ */
	tio.c_cc[VERASE]   = 0; /* del */
	tio.c_cc[VKILL]    = 0; /* @ */
	tio.c_cc[VEOF]     = 4; /* Ctrl-d */
	tio.c_cc[VTIME]    = 0; /* inter-character timer unused */
	tio.c_cc[VMIN]     = 1; /* block read until 1 character arrives */
	tio.c_cc[VSWTC]    = 0; /* '\0' */
	tio.c_cc[VSTART]   = 0; /* Ctrl-q */
	tio.c_cc[VSTOP]    = 0; /* Ctrl-s */
	tio.c_cc[VSUSP]    = 0; /* Ctrl-z */
	tio.c_cc[VEOL]     = 0; /* '\0' */
	tio.c_cc[VREPRINT] = 0; /* Ctrl-r */
	tio.c_cc[VDISCARD] = 0; /* Ctrl-u */
	tio.c_cc[VWERASE]  = 0; /* Ctrl-w */
	tio.c_cc[VLNEXT]   = 0; /* Ctrl-v */
	tio.c_cc[VEOL2]    = 0; /* '\0' */

	tcflush(m_file.handle(), TCIFLUSH);
	tcsetattr(m_file.handle(), TCSANOW, &tio);

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
	/* Can't get the serial unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to get serial because widget is closed";
		return false;
	}

	/* Write "Get Widget Serial Number Request" message */
	QByteArray request;
	request.append(char(0x7e));
	request.append(char(0x0a));
	request.append(char(0x00));
	request.append(char(0x00));
	request.append(char(0xe7));
	qint64 written = m_file.write(request);
	if (written != request.size())
	{
		qWarning() << "Unable to write serial request to"
			   << name() << ": " << m_file.errorString();
		return false;
	}

	/* Wait for the dongle to settle */
	usleep(1000);

	/* Wait for "Get Widget Serial Number Reply" message to be available */
	QByteArray reply(m_file.read(9));

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
	/* Can't send DMX unless the widget is open */
	if (isOpen() == false)
	{
		qWarning() << "Unable to send DMX because widget is closed";
		return false;
	}

	QByteArray request(universe);
	request.prepend(char((universe.size() >> 8) & 0xff)); // Data length MSB
	request.prepend(char(universe.size() & 0xff)); // Data length LSB
	request.prepend(char(0x06)); // Command
	request.prepend(char(0x7e)); // Start byte
	request.append(char(0xe7));  // Stop byte

	qint64 written = m_file.write(request);
	if (written != request.size())
	{
		qWarning() << "Unable to write DMX data to" << name() << ":"
			   << m_file.errorString();
		return false;
	}
	else
	{
		return true;
	}
}
