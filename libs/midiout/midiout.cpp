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

#include "../common/plugininfo.h"
#include "midiout.h"

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90
#define OFFSET 70

//
// Exported functions
//
extern "C" OutputPlugin* create()
{
  return new MidiOut;
}

extern "C" void destroy(OutputPlugin* object)
{
  delete object;
}

extern "C" int getPluginType(void)
{
  return PLUGIN_TYPE_OUTPUT;
}

//
// Class implementation
//
MidiOut::MidiOut() : OutputPlugin()
{
  m_fd = -1;
  m_midiOutChannel = 0;

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
  int fd = ::open(m_fileName, O_WRONLY);
  if (fd == -1)
    {
      perror("open");
      return false;
    }
  else
    {
      m_fd = fd;
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
    }

  return true;
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
