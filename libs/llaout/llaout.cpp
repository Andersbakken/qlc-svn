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

#include <linux/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <QApplication>
#include <QString>
#include <QMutex>
#include <QFile>

#include <lla/LlaClient.h>

#include "common/qlcfile.h"
#include "configurellaout.h"
#include "llaout.h"

static QMutex _mutex;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void LlaOut::init()
{
	m_lla = NULL;

	for (t_channel i = 0; i < KChannelMax; i++)
		m_values[i] = 0;

	m_refCount = 0;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int LlaOut::open()
{
	/* Count the number of times open() has been called so that the devices
	   are opened only once. This is basically reference counting. */
	m_refCount++;
	if (m_refCount > 1)
		return 0;

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
	/* Count the number of times close() has been called so that the devices
	   are closed only after the last user closes this plugin. This is
	   basically reference counting. */
	m_refCount--;
	if (m_refCount > 0)
		return 0;
	Q_ASSERT(m_refCount == 0);

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
 * Name
 *****************************************************************************/

QString LlaOut::name()
{
	return QString("LLA Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int LlaOut::configure()
{
	int r;

	open();

	ConfigureLlaOut conf(NULL, this);
	r = conf.exec();

	close();

	return r;
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString LlaOut::infoText()
{
	QString t;
	QString info;

	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Title */
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

	if (open() != 0)
	{
		info += QString("<TR>");
		info += QString("<TD><B>Unable to open LLAOut</B></TD>");
		info += QString("<TD>");
		info += QString("No DMX output");
		info += QString("</TD>");
		info += QString("</TR>");
	}
	else
	{
		info += QString("<TR>");
		info += QString("<TD><B>Available outputs</B></TD>");
		t.sprintf("%d", KChannelMax / 512);
		info += QString("<TD>" + t + "</TD>");
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

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(llaout, LlaOut)
