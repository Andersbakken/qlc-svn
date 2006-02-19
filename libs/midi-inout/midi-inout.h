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





class MidiInOut;
class ConfigureMidiInOut;
class QPoint;


extern "C" InputPlugin* create(t_plugin_id id);
extern "C" void destroy(InputPlugin* object);


class MidiInThread : public QThread
{
public:
    MidiInThread(MidiInOut* parent)
    { m_parent = parent; }

    void run();
    void stop();
    void setDevice(int device);
    const void setEventReceiver(QObject* Parent){ m_eventReceiver =  Parent;}


private:
    QMutex mutex;
    bool stopped;
    int m_device;
    QObject* m_eventReceiver;
    MidiInOut* m_parent;
};




class MidiInOut : public InputPlugin
{
  Q_OBJECT

  friend class ConfigureMidiInOut;
  friend class MidiInThread;

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

  // tPlugin methods
  void feedBack(int id, int channel, int value);
  int writeChannel(t_channel channel, t_value value);
  int writeRange(t_channel address, t_value* values, t_channel num);

  int readChannel(t_channel channel, t_value &value);
  int readRange(t_channel address, t_value* values, t_channel num);

  // Own methods
  QString deviceName() { return m_deviceName; }
  void setDeviceName(QString name) { m_deviceName = name; }
  const MidiInThread* inThread() const { return m_inThread; }
  const void setEventReceiver(QObject* Parent){ m_eventReceiver =  Parent; m_inThread->setEventReceiver(m_eventReceiver);}

/*
 signals:
   void InputEvent(const int, const int, const int);
*/

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
  
};

#endif
