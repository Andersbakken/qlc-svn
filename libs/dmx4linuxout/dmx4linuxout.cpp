/*
  Q Light Controller
  dmx4linuxout.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#include "dmx4linuxout.h"
#include "configuredmx4linuxout.h"
#include "../common/filehandler.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <qthread.h>
#include <qstring.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <assert.h>

#define CONF_FILE "dmx4linuxout.conf"

#define ID_CONFIGURE      10
#define ID_ACTIVATE       20

static QMutex _mutex;

//
// Exported functions
//
extern "C" OutputPlugin* create(t_plugin_id id)
{
  return new DMX4LinuxOut(id);
}

extern "C" void destroy(OutputPlugin* object)
{
  delete object;
}

//
// Class implementation
//
DMX4LinuxOut::DMX4LinuxOut(t_plugin_id id) : OutputPlugin(id)
{
  m_device = -1;
  m_name = QString("DMX4Linux Output");
  m_type = OutputType;
  m_version = 0x00010000;
  m_deviceName = QString("/dev/dmx");    
  m_configDir = QString("~/.qlc/");

  for (t_channel i = 0; i < 512; i++)
    {
      m_values[i] = 0;
    }
}

DMX4LinuxOut::~DMX4LinuxOut()
{
}

/* Attempt to open dmx device */
int DMX4LinuxOut::open()
{
  if (m_device != -1)
    {
      qDebug("DMX4Linux already open");
      return false;
    }

  m_device = ::open((const char*) m_deviceName, O_WRONLY | O_NONBLOCK);
  if (m_device == -1)
    {
      perror("open");
      qDebug("DMX4Linux output is not available");
    }

  return errno;
}

int DMX4LinuxOut::close()
{
  int r = 0;
  r = ::close(m_device);
  if (r == -1)
    {
      perror("close");
    }
  else
    {
      m_device = -1;
    }

  return r;
}

bool DMX4LinuxOut::isOpen()
{
  if (m_device == -1)
    {
      return false;
    }
  else
    {
      return true;
    }
}

int DMX4LinuxOut::configure()
{
  ConfigureDMX4LinuxOut* conf = new ConfigureDMX4LinuxOut(this);

  if (conf->exec() == QDialog::Accepted)
    {
      m_deviceName = conf->device();
      saveSettings();
    }

  delete conf;

  return 0;
}

QString DMX4LinuxOut::infoText()
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

int DMX4LinuxOut::setConfigDirectory(QString dir)
{
  m_configDir = dir;
  return 0;
}

int DMX4LinuxOut::saveSettings()
{
  QString s;
  QString t;

  QString fileName = m_configDir + QString(CONF_FILE);
  qDebug(fileName);
  QFile file(fileName);

  if (file.open(IO_WriteOnly))
    {
      // Comment
      s = QString("# DMX4LinuxOut Plugin Configuration\n");
      file.writeBlock((const char*) s, s.length());

      // Entry type
      s = QString("Entry = ") + name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      s = QString("Device = ") + m_deviceName + QString("\n");
      file.writeBlock((const char*) s, s.length());

      file.close();
    }
  else
    {
      perror("file.open");
      qDebug("Unable to save DMX4LinuxOut configuration");
    }

  return errno;
}

int DMX4LinuxOut::loadSettings()
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

void DMX4LinuxOut::createContents(QPtrList <QString> &list)
{
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Device"))
	{
	  m_deviceName = *(list.next());
	}
    }
}

void DMX4LinuxOut::contextMenu(QPoint pos)
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

void DMX4LinuxOut::slotContextMenuCallback(int item)
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

void DMX4LinuxOut::activate()
{
  emit activated(this);
}

//
// Write a value to a channel
//
int DMX4LinuxOut::writeChannel(t_channel channel, t_value value)
{
  int r = 0;

  _mutex.lock();

  m_values[channel] = value;

  lseek(m_device, channel, SEEK_SET);
  r = write(m_device, &value, 1);
  if (r == -1)
    {
      perror("write");
    }

  _mutex.unlock();

  return r;
}

//
// Write num values starting from address
//
int DMX4LinuxOut::writeRange(t_channel address, t_value* values, t_channel num)
{
  assert(values);
  int r = 0;

  _mutex.lock();

  memcpy(m_values + address, values, num * sizeof(t_value));

  lseek(m_device, address, SEEK_SET);
  r = write(m_device, values, num);
  if (r == -1)
    {
      perror("write");
    }

  _mutex.unlock();

  return r;
}

//
// Read a channel's value
//
int DMX4LinuxOut::readChannel(t_channel channel, t_value &value)
{
  _mutex.lock();
  value = m_values[channel];
  _mutex.unlock();

  return 0;
}

//
// Read num channel's values starting from address
//
int DMX4LinuxOut::readRange(t_channel address, t_value* values, t_channel num)
{
  assert(values);

  _mutex.lock();
  memcpy(values, m_values + address, num * sizeof(t_value));
  _mutex.unlock();

  return 0;
}
