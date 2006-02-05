/*
  Q Light Controller
  midid-inout.h
  
  Copyright (C) 2006 Heikki Junnila, Stefan Krumm
  
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

#ifndef MIDIINOUT_H
#define MIDIINOUT_H

#include "../common/inputplugin.h"
#include "../../main/src/types.h"

#include <qptrlist.h>
#include <qstring.h>
#include <qthread.h>






class ConfigureMidiInOut;
class QPoint;

extern "C" InputPlugin* create(t_plugin_id id);
extern "C" void destroy(InputPlugin* object);


class MidiInThread : public QThread
{
public:
    MidiInThread()
    { ; }

    void run();
    void stop();
    void setDevice(int device);
    void set_eventReceiver( QWidget &obj){m_eventReceiver = &obj;}

private:
    QMutex mutex;
    bool stopped;
    int m_device;
    QWidget* m_eventReceiver;
};


class MidiInOut : public InputPlugin
{
  Q_OBJECT

  friend class ConfigureMidiInOut;

 public:
  MidiInOut(t_plugin_id id);
  ~MidiInOut();

  // Plugin methods
  int open();
  int close();
  bool isOpen();
  int configure();
  QString infoText();
  void contextMenu(QPoint pos);

  int setConfigDirectory(QString dir);
  int saveSettings();
  int loadSettings();

  // OutputPlugin methods
  int writeChannel(t_channel channel, t_value value);
  int writeRange(t_channel address, t_value* values, t_channel num);

  int readChannel(t_channel channel, t_value &value);
  int readRange(t_channel address, t_value* values, t_channel num);

  // Own methods
  QString deviceName() { return m_deviceName; }
  void setDeviceName(QString name) { m_deviceName = name; }
  void set_eventReceiver( QWidget &obj){m_eventReceiver = &obj;}

 private slots:
  void slotContextMenuCallback(int item);

 private:
  void activate();
  void createContents(QPtrList <QString> &list);

 private:
  QString m_deviceName;
  QString m_configDir;
  int m_device;
  t_value m_values[KChannelMax];
  MidiInThread* m_inThread;
  QWidget* m_eventReceiver;
};

#endif
