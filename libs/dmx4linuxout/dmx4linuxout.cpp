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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <qthread.h>
#include <qstring.h>
#include <qpoint.h>

#include <dmx.h>

static QMutex _mutex;

//
// Exported functions
//
extern "C" OutputPlugin* create(int id)
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
DMX4LinuxOut::DMX4LinuxOut(int id) : OutputPlugin(id)
{
  m_device = -1;
  m_name = QString("DMX4Linux Output");
  m_type = OutputType;
  m_version = 0x00010000;
}

DMX4LinuxOut::~DMX4LinuxOut()
{
}

/* Attempt to open dmx device */
bool DMX4LinuxOut::open()
{
  qDebug("Open DMX4Linux plugin");

  m_device = ::open("/dev/dmx", O_RDWR | O_NONBLOCK);
  if (m_device == -1)
    {
      perror("open");
      qDebug("DMX4Linux output is not available");
      return false;
    }
  else
    {
      m_deviceName = QString("/dev/dmx");

      qDebug("DMX output available thru " + m_deviceName);
      return true;
    }
}

bool DMX4LinuxOut::close()
{
  qDebug("Close MidiOut plugin");

  if (m_device != -1)
    {
      m_deviceName = QString::null;
      ::close(m_device);
      m_device = -1;

      qDebug("DMX4Linux plugin closed");

      return true;
    }
  else
    {
      return false;
    }
}

void DMX4LinuxOut::configure()
{
}

QString DMX4LinuxOut::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR><TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">") + name() + QString("</FONT></TD></TR></TABLE>");
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

  str += QString("</TR>");
  str += QString("</TABLE>");
  str += QString("</BODY></HTML>");

  return str;
}

void DMX4LinuxOut::contextMenu(QPoint pos)
{
}

/* Write value to channel */
bool DMX4LinuxOut::writeChannel(unsigned short channel, unsigned char value)
{
  bool result = true;

  _mutex.lock();
  lseek(m_device, channel, SEEK_SET);
  if (write(m_device, &value, 1))
    {
      result = false;
    }
  _mutex.unlock();

  return result;
}
