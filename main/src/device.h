/*
  Q Light Controller
  device.h
  
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

#ifndef DEVICE_H
#define DEVICE_H

#include <qobject.h>
#include <qptrlist.h>

#include "channelui.h"
#include "types.h"

class QFile;
class QString;

class DeviceClass;
class SceneEditor;
class Function;
class ContainerView;
class Monitor;

class Device : public QObject
{
  Q_OBJECT

 public:
  Device();
  virtual ~Device();

  static Device* create(QPtrList <QString> &list);

  void setID(t_device_id);
  t_device_id id();

  void setAddress(t_channel address);
  t_channel address();

  void setName(QString);
  QString name();

  QString infoText();

  /* Device Class */
  DeviceClass* deviceClass();
  void setDeviceClass(DeviceClass* dc);

  /* Device's unique id number */
  const t_device_id id() const { return m_id; }

  void viewConsole();
  void viewMonitor();
  void viewProperties();

  QPtrList <ChannelUI> unitList() { return m_unitList; }
  SceneEditor* sceneEditor() { return m_sceneEditor; }

  /* Save this device's properties to a file */
  void saveToFile(QFile &file);

 private slots:
  void slotConsoleClosed();
  void slotMonitorClosed();

 signals:
  void valueChanged(int chnum, int value);
  void functionsListChanged(const QString &, bool);

 protected:
  DeviceClass* m_deviceClass;
  t_channel m_address;
  t_device_id m_id;
  QString m_name;

  ContainerView* m_console;
  SceneEditor* m_sceneEditor;
  Monitor* m_monitor;

 private:
  QPtrList <ChannelUI> m_unitList;

 protected:
  void createChannelUnits(void);
};

#endif
