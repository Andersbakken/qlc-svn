/*
  Q Light Controller
  midiout.cpp

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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <qstring.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qfile.h>

#include "../common/plugin.h"
#include "../common/filehandler.h"
#include "../../main/src/types.h"

#include "midiout.h"
#include "configuremidiout.h"

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90

#define ID_CONFIGURE       10
#define ID_ACTIVATE        20

#define CONF_FILE          "midiout.conf"

//
// Exported functions
//
extern "C" OutputPlugin* create(t_plugin_id id)
{
  return new MidiOut(id);
}

extern "C" void destroy(OutputPlugin* object)
{
  delete object;
}

//
// Class implementation
//
MidiOut::MidiOut(t_plugin_id id) : OutputPlugin(id)
{
  m_fd = -1;
  m_midiChannel = 1;
  m_firstNote = 0;
  m_name = QString("Midi Output");
  m_version = 0x00000100;
  m_deviceName = QString("/dev/midi00");
  m_configDirectory = QString("~/.qlc/");

  for (t_channel i = 0; i < 512; i++)
    {
      m_values[i] = 0;
    }
}

MidiOut::~MidiOut()
{

}

void MidiOut::setFileName(QString fileName)
{
  m_deviceName = fileName;
}

int MidiOut::setDeviceName(QString name)
{
  m_deviceName = name;
  return 0;
}

int MidiOut::open()
{
  int r = 0;

  m_mutex.lock();

  r = ::open((const char*) m_deviceName, O_WRONLY | O_NONBLOCK);
  if (r == -1)
    {
      perror("open");
    }
  else
    {
      m_fd = r;
      r = 0;
    }

  m_mutex.unlock();

  return r;
}

int MidiOut::close()
{
  int r = 0;

  m_mutex.lock();

  r = ::close(m_fd);
  if (r == -1)
    {
      perror("close");
    }
  else
    {
      m_fd = -1;
    }

  m_mutex.unlock();

  return r;
}

bool MidiOut::isOpen()
{
  if (m_fd == -1)
    {
      return false;
    }
  else
    {
      return true;
    }
}

QString MidiOut::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
  str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
  str += name() + QString("</FONT></TD></TR></TABLE>");
  str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");

  str += QString("<TR><TD><B>Version</B></TD>");
  str += QString("<TD>");
  t.setNum((version() >> 16) & 0xff);
  str += t + QString(".");
  t.setNum((version() >> 8) & 0xff);
  str += t + QString(".");
  t.setNum(version() & 0xff);
  str += t + QString("</TD></TR>");

  str += QString("<TR><TD><B>Midi Channel</B></TD>");
  t.setNum(m_midiChannel);
  str += QString("<TD>") + t + QString("</TD></TR>");

  str += QString("<TR><TD><B>First Note Number</B></TD>");
  t.setNum(m_firstNote);
  str += QString("<TD>") + t + QString("</TD></TR>");

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

  str += QString("</TABLE>");
  str += QString("</BODY></HTML>");

  return str;
}

int MidiOut::setConfigDirectory(QString dir)
{
  m_configDirectory = dir;
  return 0;
}

int MidiOut::saveSettings()
{
  QString s;
  QString t;
  
  QString fileName = m_configDirectory + QString(CONF_FILE);
  qDebug(fileName);
  QFile file(fileName);
  
  if (file.open(IO_WriteOnly))
    {
      // Comment
      s = QString("# MidiOut Plugin Configuration\n");
      file.writeBlock((const char*) s, s.length());
      
      // Entry type
      s = QString("Entry = ") + name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      s = QString("Device = ") + m_deviceName + QString("\n");
      file.writeBlock((const char*) s, s.length());

      t.setNum(m_midiChannel);
      s = QString("MidiChannel = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());

      t.setNum(m_firstNote);
      s = QString("FirstNote = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());

      file.close();
    }
  else
    {
      perror("file.open");
    }

  return -1;
}

int MidiOut::loadSettings()
{
  QString fileName;
  QPtrList <QString> list;
  
  fileName = m_configDirectory + QString(CONF_FILE);
  
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

void MidiOut::createContents(QPtrList <QString> &list)
{
  QString t;

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
      else if (*s == QString("MidiChannel"))
	{
	  t = *(list.next());
	  m_midiChannel = t.toInt();
	}
      else if (*s == QString("FirstNote"))
	{
	  t = *(list.next());
	  m_firstNote = t.toInt();
	}
    }
}

void MidiOut::setMidiChannel(t_channel channel)
{
  if (channel <= 16 && channel > 0)
    {
      m_midiChannel = channel;
    }
}

int MidiOut::writeChannel(t_channel channel, t_value value)
{
  t_value buf[3];

  if (channel >= (MAX_MIDIOUT_DMX_CHANNELS - m_firstNote))
    {
      return -1;
    }

  m_mutex.lock();

  // Convert [0|255] values to [0|127]
  m_values[channel] = value;
  value = value / 2;

  if (value == 0)
    {
      buf[0] = MIDI_NOTEOFF + m_midiChannel - 1;
      buf[1] = m_firstNote + channel;
      buf[2] = 0xFF;
    }
  else
    {
      buf[0] = MIDI_NOTEON + m_midiChannel - 1;
      buf[1] = m_firstNote + channel;
      buf[2] = value;
    }

  ssize_t num = ::write(m_fd, buf, sizeof(buf));
  if (num == sizeof(buf))
    {
      m_mutex.unlock();
      return 0;
    }
  else
    {
      m_mutex.unlock();
      perror("write");
      return -1;
    }
}

int MidiOut::writeRange(t_channel address, t_value* values, t_channel num)
{
  for (t_channel i = 0; i < num; i++)
    {
      if (writeChannel(address + i, values[i]) == -1)
	{
	  return -1;
	}
    }

  return 0;
}

int MidiOut::readChannel(t_channel channel, t_value &value)
{
  m_mutex.lock();

  value = m_values[channel];

  m_mutex.unlock();

  return 0;
}

int MidiOut::readRange(t_channel address, t_value* values, t_channel num)
{
  m_mutex.lock();

  for (t_channel i = 0; i < num; i++)
    {
      values[i] = m_values[i];
    }

  m_mutex.unlock();

  return 0;
}

int MidiOut::configure()
{
  ConfigureMidiOut* conf = new ConfigureMidiOut(this);

  if (conf->exec() == QDialog::Accepted)
    {
      m_deviceName = conf->device();
      m_midiChannel = conf->midiChannel();
      m_firstNote = conf->firstNote();
      saveSettings();
    }

  return 0;
}

void MidiOut::activate()
{
  emit activated(this);
}

void MidiOut::contextMenu(QPoint pos)
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

void MidiOut::slotContextMenuCallback(int item)
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
