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

#include "../common/plugin.h"
#include "midiout.h"

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90
#define OFFSET 70

#define ID_CONFIGURE       10

//
// Exported functions
//
extern "C" OutputPlugin* create(int id)
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
MidiOut::MidiOut(int id) : OutputPlugin(id)
{
  m_fd = -1;
  m_midiOutChannel = 0;
  m_name = QString("Midi Output");
  m_version = 0x00000100;
  m_fileName = QString("/dev/midi00");

  for (unsigned short i = 0; i < MAX_MIDIOUT_DMX_CHANNELS; i++)
    {
      m_buffer[i] = 0;
    }
}

MidiOut::~MidiOut()
{

}

void MidiOut::setFileName(QString fileName)
{
  m_fileName = fileName;
}

bool MidiOut::open()
{
  qDebug("Open MidiOut plugin");

  int fd = ::open((const char*) m_fileName, O_WRONLY);
  if (fd == -1)
    {
      perror("open");
      qDebug("Midi Output not available");
      return false;
    }
  else
    {
      qDebug(QString("Midi Output available thru ") + m_fileName);
      m_fd = fd;
    }

  return true;
}

bool MidiOut::close()
{
  qDebug("Close MidiOut plugin");

  if (m_fd == -1)
    {
      return false;
    }
  else
    {
      if (::close(m_fd) == -1)
	{
	  perror("close");
	  qDebug("Unable to close MidiOut plugin");
	  return false;
	}
      else
	{
	  qDebug("MidiOut plugin closed");
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

void MidiOut::setMidiOutChannel(unsigned char channel)
{
  if (channel < 16)
    {
      m_midiOutChannel = channel;
    }
}

bool MidiOut::writeChannel(unsigned short channel, unsigned char value)
{
  unsigned char buf[3];

  if (m_fd == -1 || channel >= MAX_MIDIOUT_DMX_CHANNELS)
    {
      return false;
    }

  m_buffer[channel] = value;
  
  if (value == 0)
    {
      buf[0] = MIDI_NOTEOFF + m_midiOutChannel;
      buf[1] = OFFSET + channel;
      buf[2] = 0xFF;
    }
  else
    {
      buf[0] = MIDI_NOTEON + m_midiOutChannel;
      buf[1] = OFFSET + channel;
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

unsigned char MidiOut::readChannel(unsigned short channel)
{
  if (channel >= MAX_MIDIOUT_DMX_CHANNELS)
    {
      return 0;
    }
  else
    {
      return m_buffer[channel];
    }
}

void MidiOut::configure()
{
  QMessageBox::information(NULL, QString("QLC Midi Output Plugin"),
			   QString("Not implemented"));
}

void MidiOut::contextMenu(QPoint pos)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Configure...", ID_CONFIGURE);

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotContextMenuCallback(int)));
  menu->exec(pos, 0);
  delete menu;
}

void MidiOut::slotContextMenuCallback(int item)
{
  switch(item)
    {
    case ID_CONFIGURE:
      break;

    default:
      break;
    }
}
