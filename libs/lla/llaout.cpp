/*
  Q Light Controller
  llaout.cpp
  
  Copyright (c) Simon Newton
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
#include <linux/errno.h>

#include <qapplication.h>
#include <qthread.h>
#include <qstring.h>
#include <qfile.h>

#include <lla/LlaClient.h>

#include "common/filehandler.h"
#include "llaout.h"
#include "configurellaout.h"

static QMutex _mutex;

extern "C" OutputPlugin* create()
{
	return new LlaOut();
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

LlaOut::LlaOut() : OutputPlugin()
{
	m_lla = NULL;
	m_name = QString("LLA Output");
	m_type = OutputType;
	m_version = 0x00010100;
	m_configDir = QString("~/.qlc/");
	
	for (t_channel i = 0; i < KChannelMax; i++)
		m_values[i] = 0;

	open();
}

LlaOut::~LlaOut()
{
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int LlaOut::open()
{
	close();

	m_lla = new LlaClient();
	
	if (m_lla == NULL)
		return -1;
	
	if (m_lla->start() < 0)
	{
		delete m_lla;
		m_lla = NULL;
		return -1;
	}

	return 0;
}

int LlaOut::close()
{
	if (m_lla != NULL)
	{
		m_lla->stop();
		delete m_lla;
		m_lla = NULL;
	}
	
	return 0;
}

int LlaOut::outputs()
{
	return 1;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int LlaOut::configure(QWidget* parentWidget)
{
	ConfigureLlaOut conf(parentWidget, this);
	conf.exec();
	return 0;
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString LlaOut::infoText()
{
	QString t;
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>Plugin Info</TITLE>");
	str += QString("</HEAD>");
	str += QString("<BODY>");

	/* Title */
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD BGCOLOR=\"");
	str += QApplication::palette().active().highlight().name();
	str += QString("\">");
	str += QString("<FONT COLOR=\"");
	str += QApplication::palette().active().highlightedText().name();
	str += QString("\" SIZE=\"5\">");
	str += name();
	str += QString("</FONT>");
	str += QString("</TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	/* Version */
	str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD><B>Version</B></TD>");
	t.sprintf("%d.%d.%d", (version() >> 16) & 0xff,
		  (version() >> 8) & 0xff, version() & 0xff);
	str += QString("<TD>" + t + "</TD>");
	str += QString("</TR>");
	
	str += QString("</TABLE>");
	str += QString("</BODY>");
	str += QString("</HTML>");
	
	return str;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

int LlaOut::writeChannel(t_channel channel, t_value value)
{
	int r = 0;

	_mutex.lock();
	
	m_values[channel] = value;
	
	//which interface should we write to?
	int uniNo = int(channel / 512);
	
	if (m_lla != NULL)
	{
		m_lla->send_dmx(uniNo + 1, &m_values[uniNo * 512], 512);
		m_lla->fd_action(0);
	}
	
	_mutex.unlock();
	
	return r;
}

int LlaOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	int r = 0;
	QString txt;
	
	Q_ASSERT(values != NULL);

	_mutex.lock();
	
	// which one is the first universe to write to?
	int uni = int(address / 512);
	
	// how many universes?
	int lastUni = (address + num) / 512;
	
	memcpy(m_values + address, values, num * sizeof(t_value));
	
	if (m_lla != NULL)
	{
		for(int i = uni; i <= lastUni; i++)
			m_lla->send_dmx(i + 1, &m_values[i * 512], 512);
	}
	
	m_lla->fd_action(0);
	_mutex.unlock();
	
	return r;
}

int LlaOut::readChannel(t_channel channel, t_value &value)
{
	_mutex.lock();
	value = m_values[channel];
	_mutex.unlock();
	
	return 0;
}

int LlaOut::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(values != NULL);
	
	_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	_mutex.unlock();
	
	return 0;
}
