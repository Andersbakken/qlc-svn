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
}

MidiOut::~MidiOut()
{

}

void MidiOut::setFileName(QString fileName)
{
  m_deviceName = fileName;
}

bool MidiOut::open()
{
  if (m_fd != -1)
    {
      qDebug("MidiOut already open");
      return false;
    }

  m_fd = ::open((const char*) m_deviceName, O_WRONLY | O_NONBLOCK);
  if (m_fd == -1)
    {
      perror("open");
      qDebug("Midi Output not available");
      return false;
    }

  return true;
}

bool MidiOut::close()
{
  if (m_fd == -1)
    {
      return false;
    }
  else
    {
      if (::close(m_fd) == -1)
	{
	  return false;
	}
      else
	{
	  m_fd = -1;
	}
    }

  return true;
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
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR><TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">") + name() + QString("</FONT></TD></TR></TABLE>");
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

void MidiOut::setConfigDirectory(QString dir)
{
  m_configDirectory = dir;
}

void MidiOut::saveSettings()
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
      qDebug("Unable to save MidiOut configuration");
    }
}

void MidiOut::loadSettings()
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

void MidiOut::setMidiChannel(unsigned char channel)
{
  if (channel <= 16 && channel > 0)
    {
      m_midiChannel = channel;
    }
}

bool MidiOut::writeChannel(unsigned short channel, unsigned char value)
{
  unsigned char buf[3];

  if (m_fd == -1 || channel >= (MAX_MIDIOUT_DMX_CHANNELS - m_firstNote))
    {
      return false;
    }

  // Convert [0|255] values to [0|127]
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
      return true;
    }
  else
    {
      return false;
    }
}

bool MidiOut::writeRange(unsigned short address, unsigned char* values,
			 unsigned short num)
{
  //
  // TODO: Make use of MIDI's running status-feature to reduce overhead
  // http://www.borg.com/~jglatt/tech/midispec.htm
  //
  for (unsigned short i = 0; i < num; i++)
    {
      if (writeChannel(address + i, values[i]) == false)
	{
	  return false;
	}
    }

  return true;
}

bool MidiOut::readChannel(unsigned short channel, unsigned char &value)
{
  value = 0;
  return false;
}

bool MidiOut::readRange(unsigned short address, unsigned char* values,
			unsigned short num)
{
  for (int i = 0; i < num; i++)
    {
      values[i] = 0;
    }

  return false;
}

void MidiOut::configure()
{
  ConfigureMidiOut* conf = new ConfigureMidiOut(this);

  if (conf->exec() == QDialog::Accepted)
    {
      m_deviceName = conf->device();
      m_midiChannel = conf->midiChannel();
      m_firstNote = conf->firstNote();
      saveSettings();
    }
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

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotContextMenuCallback(int)));
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
