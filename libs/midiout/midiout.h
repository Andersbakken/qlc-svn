/*
  Q Light Controller
  midiout.h

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

#ifndef MIDIOUT_H
#define MIDIOUT_H

#include "../common/outputplugin.h"

extern "C" OutputPlugin* create();
extern "C" void destroy(OutputPlugin* object);
extern "C" int getPluginType(void);

#define MAX_MIDIOUT_DMX_CHANNELS 128

class MidiOut : public OutputPlugin
{
  Q_OBJECT

 public:
  MidiOut();
  virtual ~MidiOut();

  virtual void setFileName(QString);
  virtual bool open();
  virtual bool close();

  virtual void configure();

  bool writeChannel(unsigned short channel, unsigned char value);
  unsigned char readChannel(unsigned short channel);

 protected:
  void setMidiOutChannel(unsigned char channel);
  unsigned char midiOutChannel() { return m_midiOutChannel; }

 private:
  QString m_fileName;
  int m_fd;
  unsigned char m_midiOutChannel;

  unsigned char m_buffer[MAX_MIDIOUT_DMX_CHANNELS];
};

#endif
