/*
  Q Light Controller
  dmx4linuxout.cpp
  
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

#include <linux/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <QApplication>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QFile>

#include "common/qlcfile.h"

#include "configuredmx4linuxout.h"
#include "dmx4linuxout.h"

static QMutex _mutex;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void DMX4LinuxOut::init()
{
	m_fd = 0;
	m_openError = 0;
	m_refCount = 0;

	for (t_channel i = 0; i < MAX_DMX4LINUX_DEVICES * 512; i++)
		m_values[i] = 0;

}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int DMX4LinuxOut::open()
{
	/* Count the number of times open() has been called so that the devices
	   are opened only once. This is basically reference counting. */
	m_refCount++;
	if (m_refCount > 1)
		return 0;

	m_fd = ::open("/dev/dmx", O_WRONLY | O_NONBLOCK);
	if (m_fd < 0)
	{
		m_openError = errno;
		qWarning() << "DMX4Linux output is not available:"
			   << strerror(m_openError);
	}
	else
	{
		m_openError = 0;
	}

	return errno;
}

int DMX4LinuxOut::close()
{
	/* Count the number of times close() has been called so that the devices
	   are closed only after the last user closes this plugin. This is
	   basically reference counting. */
	m_refCount--;
	if (m_refCount > 0)
		return 0;
	Q_ASSERT(m_refCount == 0);

	int r = ::close(m_fd);
	if (r == -1)
		perror("close");
	else
		m_fd = -1;
	m_openError = 0;

	return r;
}

int DMX4LinuxOut::outputs()
{
	return MAX_DMX4LINUX_DEVICES;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString DMX4LinuxOut::name()
{
	return QString("DMX4Linux Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int DMX4LinuxOut::configure()
{
	int r;

	open();

	ConfigureDMX4LinuxOut conf(NULL, this);
	r = conf.exec();

	close();

	return r;
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DMX4LinuxOut::infoText()
{
	QString info = QString::null;
	QString t;

	/* HTML Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * DMX4Linux information
	 *********************************************************************/

	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");

	open();
	
	/* Print error if /dev/dmx cannot be opened */
	if (m_openError != 0)
	{
		info += QString("<TR>");
		info += QString("<TD><B>Unable to open /dev/dmx:</B></TD>");
		info += QString("<TD>");
		info += QString(strerror(m_openError));
		info += QString("</TD>");
		info += QString("</TR>");
	}

	if (m_openError == 0)
	{
		info += QString("<TR>");
		info += QString("<TD><B>Available outputs</B></TD>");
		t.sprintf("%d", MAX_DMX4LINUX_DEVICES);
		info += QString("<TD>" + t + "</TD>");
		info += QString("</TR>");
	}
	else
	{
		info += QString("<TR>");
		info += QString("<TD><B>Unable to read info:</B></TD>");
		info += QString("<TD>");
		info += QString(strerror(m_openError));
		info += QString("</TD>");
		info += QString("</TR>");
	}
	info += QString("</TABLE>");

	close();

	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

int DMX4LinuxOut::writeChannel(t_channel channel, t_value value)
{
	int r = 0;

	_mutex.lock();

	m_values[channel] = value;

	lseek(m_fd, channel, SEEK_SET);
	r = write(m_fd, &value, 1);
	if (r == -1)
		perror("write");

	_mutex.unlock();

	return r;
}

int DMX4LinuxOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	int r = 0;

	_mutex.lock();

	memcpy(m_values + address, values, num * sizeof(t_value));

	lseek(m_fd, address, SEEK_SET);
	r = write(m_fd, values, num);
	if (r == -1)
		perror("write");

	_mutex.unlock();

	return r;
}

int DMX4LinuxOut::readChannel(t_channel channel, t_value &value)
{
	_mutex.lock();
	value = m_values[channel];
	_mutex.unlock();

	return 0;
}

int DMX4LinuxOut::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);

	_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	_mutex.unlock();

	return 0;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(dmx4linuxout, DMX4LinuxOut)
