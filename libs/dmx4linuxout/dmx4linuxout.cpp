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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <linux/errno.h>

#include <qapplication.h>
#include <qstring.h>
#include <qthread.h>
#include <qfile.h>

#include "dmx4linuxout.h"
#include "configuredmx4linuxout.h"
#include "common/filehandler.h"

#define CONF_FILE "dmx4linuxout.conf"

static QMutex _mutex;

extern "C" OutputPlugin* create()
{
	return new DMX4LinuxOut();
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DMX4LinuxOut::DMX4LinuxOut() : OutputPlugin()
{
	m_name = QString("DMX4Linux Output");
	m_type = OutputType;
	m_version = 0x00010100;

	m_fd = -1;
	m_openError = 0;
	m_dmxInfoError = 0;

	for (t_channel i = 0; i < KChannelMax; i++)
		m_values[i] = 0;

	open();
}

DMX4LinuxOut::~DMX4LinuxOut()
{
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int DMX4LinuxOut::open()
{
	if (m_fd != -1)
		::close(m_fd);
	m_fd = -1;
	
	m_fd = ::open("/dev/dmx", O_WRONLY | O_NONBLOCK);
	m_openError = errno;
	if (m_openError < 0)
	{
		qDebug("DMX4Linux output is not available: %s",
		       strerror(m_openError));
		
	}
	else
	{
		/* Try to read DMX information */
		::ioctl(m_fd, DMX_IOCTL_GET_INFO, &m_dmxInfo);
		m_dmxInfoError = errno;
		if (m_dmxInfoError < 0)
		{
			qDebug("Unable to get DMX4Linux information: %s",
			       strerror(m_dmxInfoError));
		}
		else
		{
			int i = 0;

			m_dmxCaps = new struct 
				dmx_capabilities[m_dmxInfo.max_out_universes];

			/* Try to read DMX capabilities for universes */
			for (i = 0; i < m_dmxInfo.max_out_universes; i++)
			{
				m_dmxCaps[i].direction = 0;
				m_dmxCaps[i].universe = i;
				::ioctl(m_fd, DMX_IOCTL_GET_CAP, &m_dmxCaps[i]);
			}
		}
	}

	return errno;
}

int DMX4LinuxOut::close()
{
	int r = 0;
	r = ::close(m_fd);
	if (r == -1)
		perror("close");
	else
		m_fd = -1;

	return r;
}

int DMX4LinuxOut::outputs()
{
	return m_dmxInfo.used_out_universes;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int DMX4LinuxOut::configure(QWidget* parentWidget)
{
	ConfigureDMX4LinuxOut conf(parentWidget, this);
	conf.exec();
	return 0;
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

	/* Plugin version */
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD><B>Version</B></TD>");
	info += QString("<TD>");
	t.setNum((version() >> 16) & 0xff);
	info += t + QString(".");
	t.setNum((version() >> 8) & 0xff);
	info += t + QString(".");
	t.setNum(version() & 0xff);
	info += t + QString("</TD>");
	info += QString("</TR>");
	
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

	/*********************************************************************
	 * DMX4Linux information
	 *********************************************************************/
	if (m_openError == 0 && m_dmxInfoError == 0)
	{
		info += QString("<TR>");
		info += QString("<TD><B>DMX4Linux version</B></TD>");
		t.sprintf("%d.%d", m_dmxInfo.version_major, m_dmxInfo.version_minor);
		info += QString("<TD>" + t + "</TD>");
		info += QString("</TR>");

		info += QString("<TR>");
		info += QString("<TD><B>Available outputs</B></TD>");
		t.sprintf("%d", m_dmxInfo.max_out_universes);
		info += QString("<TD>" + t + "</TD>");
		info += QString("</TR>");

		info += QString("<TR>");
		info += QString("<TD><B>Used outputs</B></TD>");
		t.sprintf("%d", m_dmxInfo.used_out_universes);
		info += QString("<TD>" + t + "</TD>");
		info += QString("</TR>");
	}
	else
	{
		info += QString("<TR>");
		info += QString("<TD><B>Unable to read info:</B></TD>");
		info += QString("<TD>");
		info += QString(strerror(m_dmxInfoError));
		info += QString("</TD>");
		info += QString("</TR>");
	}
	info += QString("</TABLE>");

	/*********************************************************************
	 * DMX4Linux universe information
	 *********************************************************************/
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");

	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\">");
	info += QString("Universe");
	info += QString("</FONT>");
	info += QString("</TD>");

	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\">");
	info += QString("Driver");
	info += QString("</FONT>");
	info += QString("</TD>");

	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\">");
	info += QString("Channels");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	for (int i = 0; i < m_dmxInfo.used_out_universes; i++)
	{
		info += QString("<TR>");
		t.sprintf("%d", i + 1);
		info += QString("<TD>" + t + "</TD>");
		t.sprintf("%s", m_dmxCaps[i].driver);
		info += QString("<TD>" + t + "</TD>");
		t.sprintf("%d", m_dmxCaps[i].maxSlots);
		info += QString("<TD>" + t + "</TD>");
		info += QString("</TR>");
	}

	info += QString("</TABLE>");

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
