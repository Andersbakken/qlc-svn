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
#include <qthread.h>
#include <qptrlist.h>

class QString;

extern "C" OutputPlugin* create(t_plugin_id id);
extern "C" void destroy(OutputPlugin* object);

#define MAX_MIDIOUT_DMX_CHANNELS 128

class ConfigureMidiOut;

class MidiOut : public OutputPlugin
{
  Q_OBJECT

    friend class ConfigureMidiOut;

 public:
  MidiOut(t_plugin_id id);
  virtual ~MidiOut();

  int open();
  int close();
  bool isOpen();
  int configure();
  QString infoText();
  void contextMenu(QPoint pos);

  int setConfigDirectory(QString dir);
  int saveSettings();
  int loadSettings();

  // OutputPlugin functions
  int writeChannel(t_channel channel, t_value value);
  int writeRange(t_channel address, t_value* values, t_channel num);

  int readChannel(t_channel channel, t_value &value);
  int readRange(t_channel address, t_value* values, t_channel num);

  // Own functions
  int setDeviceName(QString name);
  QString deviceName() { return m_deviceName; }

 protected:
  void setFileName(QString);
  void setMidiChannel(t_channel channel);
  t_channel midiChannel() { return m_midiChannel; }
  t_value firstNote() { return m_firstNote; }
  void activate();
  void createContents(QPtrList <QString> &list);

 protected slots:
  void slotContextMenuCallback(int);

 private:
  QString m_deviceName;
  QString m_configDirectory;
  int m_fd;
  t_channel m_midiChannel;
  t_channel m_firstNote;

  QMutex m_mutex;
  t_value m_values[512];
};

#endif
