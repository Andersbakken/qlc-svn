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

extern "C" OutputPlugin* create(int id);
extern "C" void destroy(OutputPlugin* object);

#define MAX_MIDIOUT_DMX_CHANNELS 128

class MidiOut : public OutputPlugin
{
  Q_OBJECT

 public:
  MidiOut(int id);
  virtual ~MidiOut();

  virtual bool open();
  virtual bool close();
  virtual bool isOpen();
  virtual void configure();
  virtual QString infoText();
  virtual void contextMenu(QPoint pos);

  bool writeChannel(unsigned short channel, unsigned char value);
  unsigned char readChannel(unsigned short channel);

 protected:
  void setMidiOutChannel(unsigned char channel);
  virtual void setFileName(QString);
  unsigned char midiOutChannel() { return m_midiOutChannel; }

 protected slots:
  void slotContextMenuCallback(int);

 private:
  QString m_fileName;
  int m_fd;
  unsigned char m_midiOutChannel;

  unsigned char m_buffer[MAX_MIDIOUT_DMX_CHANNELS];
};

#endif
