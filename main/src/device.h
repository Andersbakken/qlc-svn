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

#include "classes.h"
#include <qobject.h>
#include <qlist.h>
#include <qfile.h>

class Device : public QObject
{ 
  Q_OBJECT

 protected:
  Device(int address, DeviceClass* dc, const QString &name);

 public:
  virtual ~Device();

  virtual void setName(QString);
  QString name() const { return m_name; }

  int address() const { return m_address; }

  /* Device Class */
  DeviceClass* deviceClass() { return m_deviceClass; }
  void setDeviceClass(DeviceClass* dc);

  /* Device's unique id number */
  int id() const { return m_id; }

  virtual void viewConsole() = 0;
  virtual void viewMonitor() = 0;
  virtual void viewFunctions() = 0;

  virtual Scene* currentLevels() = 0;

  /* Get one channel value */
  virtual unsigned char getChannelValue(unsigned int channel) = 0;

  /* Save this device's properties to a file */
  virtual void saveToFile(QFile &file) = 0;

  QList<Function> functions() const;
  QList<Function> m_functions;
  Function* searchFunction(const QString &fname);

 private slots:
  virtual void slotConsoleClosed() = 0;

 signals:
  void valueChanged(int chnum, int value);

 protected:
  int m_address;
  int m_id;

  QString m_name;

  DeviceClass* m_deviceClass;

  ContainerView* m_console;
  ContainerView* m_monitor;

  FunctionsView* m_functionsView;

 private:
  static int m_nextDeviceId;
};

#endif
