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

#include "function.h"
#include "classes.h"
#include "dmxdevice.h"
#include "deviceclass.h"

class Doc : public QObject
{
  Q_OBJECT

 public:
  Doc();
  ~Doc();

  bool loadWorkspaceAs(QString &);
  bool saveWorkspaceAs(QString &);
  bool saveWorkspace();

  void newDocument();

  QString workspaceFileName() { return m_workspaceFileName; }

  bool isDMXAddressSpaceFree(int address, int channels);
  bool allocateDMXAddressSpace(int address, int channels);
  bool freeDMXAddressSpace(int address, int channels);
  int findNextFreeDMXAddress(int channels);

  void addDevice(Device*);
  bool removeDevice(Device*);

  QList <Device> deviceList() const;

  Device* searchDevice(int id, DeviceClass::Protocol p);
  Device* searchDevice(const QString &name, DeviceClass::Protocol p);

  bool isModified() const { return m_modified; }

  QList <Function> m_functions;
  Function* searchFunction(const QString &fname);
  void removeFunction(const QString &functionString);

  // Init our DMX interface
  void initDMX();
  
  // Return the pointer to our DMX interface object
  DMX* dmx() const { return m_dmx; }

  // Read device class definition files
  bool readDeviceClasses();

  // Container for device classes
  QList <DeviceClass> deviceClassList() const { return m_deviceClassList; }

  // Search for manuf + model
  DeviceClass* searchDeviceClass(const QString &manufacturer, const QString &model);

  // Read file contents to a list of strings
  bool readFileToList(QString &fileName, QList<QString> &list);
  
 signals:
  void deviceListChanged();
  void deviceRemoved(int);

 private:
  void dumpDeviceClass(DeviceClass* dc);
  DeviceClass* createDeviceClass(QList<QString> &list);
  Device* createDevice(QList<QString> &list);
  Function* createFunction(QList<QString> &list);
  void createFunctionContents(QList<QString> &list);

 private:
  bool m_DMXAddressAllocation[512];

  QString m_workspaceFileName;

  DMX* m_dmx;

  bool m_modified;

  QList <Device> m_deviceList;
  QList <DeviceClass> m_deviceClassList;
};

#endif
