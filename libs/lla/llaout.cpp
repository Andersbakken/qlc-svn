/*
  Q Light Controller
  llaout.cpp
  
  Copyright (C) Simon Newton, Heikki Junnila
  
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
#include <qthread.h>
#include <qstring.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <assert.h>
#include <errno.h>
#include <linux/errno.h>

#include "common/filehandler.h"
#include "llaout.h"
#include "configurellaout.h"

#define CONF_FILE "llaout.conf"

#define ID_CONFIGURE      10
#define ID_ACTIVATE       20

static QMutex _mutex;

//
// Exported functions
//
extern "C" OutputPlugin* create(t_plugin_id id)
{
	return new LlaOut(id);
}

extern "C" void destroy(OutputPlugin* object)
{
	delete object;
}

//
// Class implementation
//
LlaOut::LlaOut(t_plugin_id id) : OutputPlugin(id)
{
	m_lla = NULL;
	m_name = QString("LLA Output");
	m_type = OutputType;
	m_version = 0x00010000;
	m_firstUni = 1;
	m_configDir = QString("~/.qlc/");
	
	for (t_channel i = 0; i < KChannelMax; i++)
	{
		m_values[i] = 0;
	}
}

LlaOut::~LlaOut()
{
}

/* Open a connection to the lla daemon */
int LlaOut::open()
{
	m_lla = new LlaClient();
	
	if (m_lla == NULL)
		return -1;
	
	if (m_lla->start() < 0)
	{
		delete m_lla;
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
	}
	
	return 0;
}

bool LlaOut::isOpen()
{
	if (m_lla == NULL)
		return false;
	else
		return true;
}

int LlaOut::configure()
{
	ConfigureLlaOut* conf = new ConfigureLlaOut(this);
	
	if (conf->exec() == QDialog::Accepted)
	{
		m_firstUni = conf->firstDeviceID();
		saveSettings();
	}
	
	delete conf;
	
	return 0;
}

QString LlaOut::infoText()
{
	QString t;
	QString str = QString::null;
	str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
	str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
	str += name() + QString("</FONT></TD></TR></TABLE>");
	str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	str += QString("<TR>\n");
	str += QString("<TD><B>Version</B></TD>");
	str += QString("<TD>");
	t.setNum((version() >> 16) & 0xff);
	str += t + QString(".");
	t.setNum((version() >> 8) & 0xff);
	str += t + QString(".");
	t.setNum(version() & 0xff);
	str += t + QString("</TD>");
	str += QString("</TR>");
	
	str += QString("<TR>\n");
	str += QString("<TD><B>Status</B></TD>");
	str += QString("<TD>");
	if (isOpen() == true)
	{
		str += QString("<I>Active</I></TD>");
	}
	else
	{
		str += QString("Not Active</TD>");
	}
	str += QString("</TR>");
	
	str += QString("</TR>");
	str += QString("</TABLE>");
	str += QString("</BODY></HTML>");
	
	return str;
}

int LlaOut::setConfigDirectory(QString dir)
{
	m_configDir = dir;
	return 0;
}

int LlaOut::saveSettings()
{
	QString s;
	QString t;
	
	QString fileName = m_configDir + QString(CONF_FILE);
	qDebug(fileName);
	QFile file(fileName);
	
	if (file.open(IO_WriteOnly))
	{
		// Comment
		s = QString("# LlaOut Plugin Configuration\n");
		file.writeBlock((const char*) s, s.length());
		
		// Entry type
		s = QString("Entry = ") + name() + QString("\n");
		file.writeBlock((const char*) s, s.length());
		
		t.sprintf("%d",m_firstUni);
		s = QString("FirstUni = ") + t + QString("\n");
		file.writeBlock((const char*) s, s.length());
		
		file.close();
	}
	else
	{
		perror("file.open");
		qDebug("Unable to save LlaOut configuration");
	}
	
	return errno;
}

int LlaOut::loadSettings()
{
	QString fileName;
	QPtrList <QString> list;
	
	fileName = m_configDir + QString(CONF_FILE);
	
	FileHandler::readFileToList(fileName, list);
	
	for (QString* s = list.first(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.next();
			if (*s == name())
			{
				createContents(list);
			}
		}
	}
	
	return 0;
}


void LlaOut::createContents(QPtrList <QString> &list)
{
	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else if (*s == QString("FirstUni"))
		{
			m_firstUni = list.next()->toInt();
		}
	}
}

void LlaOut::contextMenu(QPoint pos)
{
	QPopupMenu* menu = new QPopupMenu();
	menu->insertItem("Configure...", ID_CONFIGURE);
	menu->insertSeparator();
	menu->insertItem("Activate", ID_ACTIVATE);
	
	connect(menu, SIGNAL(activated(int)), 
		this, SLOT(slotContextMenuCallback(int)));
	menu->exec(pos, 0);
	delete menu;
}

void LlaOut::slotContextMenuCallback(int item)
{
	switch(item)
	{
	case ID_CONFIGURE:
		configure();
		break;
		
	case ID_ACTIVATE:
		activate();
		break;
		
	default:
		break;
	}
}

void LlaOut::activate()
{
	emit activated(this);
}

//
// Write a value to a channel
//
int LlaOut::writeChannel(t_channel channel, t_value value)
{
	int r = 0;
	_mutex.lock();
	
	m_values[channel] = value;
	
	//which interface should we write to?
	int uniNo = int(channel / 512);
	
	if (m_lla != NULL) {
		m_lla->send_dmx(uniNo + m_firstUni, &m_values[uniNo*512], 512);
		m_lla->fd_action(0);
	}
	
	_mutex.unlock();
	
	return r;
}

//
// Write num values starting from address
//
int LlaOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	int r = 0;
	QString txt;
	
	assert(values);

	_mutex.lock();
	
	// which one is the first universe to write to?
	int uni = int(address / 512);
	
	// how many universes?
	int lastUni = (address + num) / 512;
	
	memcpy(m_values + address, values, num * sizeof(t_value));
	
	if (m_lla != NULL)
	{
		for(int i = uni; i <= lastUni; i++)
		{
			m_lla->send_dmx(i + m_firstUni, &m_values[i*512], 512);
		}
	}
	
	m_lla->fd_action(0);
	_mutex.unlock();
	
	return r;
}

//
// Read a channel's value
//
int LlaOut::readChannel(t_channel channel, t_value &value)
{
	_mutex.lock();
	value = m_values[channel];
	_mutex.unlock();
	
	return 0;
}

//
// Read num channel's values starting from address
//
int LlaOut::readRange(t_channel address, t_value* values, t_channel num)
{
	assert(values);
	
	_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	_mutex.unlock();
	
	return 0;
}
