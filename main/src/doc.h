/*
  Q Light Controller
  doc.h
  
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

#ifndef DOC_H
#define DOC_H

#include <qobject.h>
#include <qptrlist.h>

#include "function.h"
#include "device.h"
#include "deviceclass.h"
#include "bus.h"

class Doc : public QObject
{
  Q_OBJECT

 public:
  Doc();
  ~Doc();

  void init();

  //
  // Document handling stuff
  //
  void newDocument();
  bool isModified() { return m_modified; }
  void setModified(bool modified);

  QString workspaceFileName() { return m_workspaceFileName; }
  bool loadWorkspaceAs(QString &);
  bool saveWorkspaceAs(QString &);
  bool saveWorkspace();

  //
  // Devices
  //
  bool addDevice(Device* d);
  bool removeDevice(Device* d);
  QPtrList <Device> *deviceList() { return &m_deviceList; }
  Device* searchDevice(const t_device_id id);

  //
  // Functions
  //
  // Function list
  QPtrList <Function> *functions() { return &m_functions; }

  // Function list operations
  bool addFunction(const Function* function);
  bool removeFunction(const t_function_id id, bool deleteFunction = true);
  Function* searchFunction(const t_function_id id);

  //
  // Device classes
  //
  QPtrList <DeviceClass> *deviceClassList() { return &m_deviceClassList; }
  bool readDeviceClasses();
  DeviceClass* searchDeviceClass(const QString &manufacturer,
				 const QString &model);
  DeviceClass* searchDeviceClass(const t_deviceclass_id id);

  //
  // Buses
  //
  void initBuses();
  bool busValue(t_bus_id busID, t_bus_value& value);
  bool setBusValue(t_bus_id busID, t_bus_value value);

  /*
  QPtrList <Bus> *busList()  { return &m_busList; }
  Bus* searchBus(const t_bus_id id);
  void addBus(Bus* bus);
  void removeBus(const t_bus_id id, bool deleteBus = true);
  */

 signals:
  void deviceListChanged();
  void deviceRemoved(int);
  void newDocumentClicked();

 public slots:
  void slotBusValueChanged(t_bus_id, t_bus_value);

 private:
  DeviceClass* createDeviceClass(QPtrList <QString> &list);
  Device* createDevice(QPtrList <QString> &list);
  Function* createFunction(QPtrList <QString> &list);
  void createFunctionContents(QPtrList <QString> &list);

 private:
  QString m_workspaceFileName;
  bool m_modified;

  QPtrList <Function> m_functions;
  QPtrList <Device> m_deviceList;
  QPtrList <DeviceClass> m_deviceClassList;

  Bus* m_busArray;
};

#endif

