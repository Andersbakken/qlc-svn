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
#include <qlist.h>
#include "function.h"
#include "dmxdevice.h"
#include "deviceclass.h"
#include "inputdevice.h"
#include "plugin.h"
#include "bus.h"
#include "classes.h"

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

  bool isDMXAddressSpaceFree(unsigned short address, unsigned short channels);
  bool allocateDMXAddressSpace(unsigned short address, unsigned short channels);
  bool freeDMXAddressSpace(unsigned short address, unsigned short channels);
  unsigned short findNextFreeDMXAddress(unsigned short channels);
  DMXChannel* dmxChannel(unsigned short channel);

  void addDevice(DMXDevice*);
  bool removeDevice(DMXDevice*);
  QList <DMXDevice> deviceList() const;

  DMXDevice* searchDevice(int id, DeviceClass::Protocol p);
  DMXDevice* searchDevice(const QString &name, DeviceClass::Protocol p);

  bool isModified() const { return m_modified; }

  QList<Function> functions() const { return m_functions; }
  Function* searchFunction(const QString &fname);
  void addFunction(const Function* function);
  void removeFunction(const QString &functionString);

  // Init our DMX interface
  void initDMX();
  
  // Return the pointer to our DMX interface object
  DMX* dmx() const { return m_dmx; }

  // Read device class definition files
  bool readDeviceClasses();

  // Container for device classes
  QList <DeviceClass> deviceClassList() const { return m_deviceClassList; }
  QList <DeviceClass> * deviceClassListPointer()  { return &m_deviceClassList; }

  // Search for manuf + model
  DeviceClass* searchDeviceClass(const QString &manufacturer, const QString &model);

  // Read file contents to a list of strings
  bool readFileToList(QString &fileName, QList<QString> &list);

  // Input devices
  QList <InputDevice> inputDeviceList() const { return m_inputDeviceList; }

  // Plugin libraries
  QList <Plugin> pluginList() const { return m_pluginList; }

  // System bus list
  QList <Bus> *busList()  { return &m_busList; }

  Bus* searchBus(unsigned int id);
  Bus* searchBus(QString name);
  void addBus(Bus* bus);
  void removeBus(unsigned int id, bool deleteBus = true);

 signals:
  void deviceListChanged();
  void deviceRemoved(int);

 private:
  void initializeDMXChannels();

  void dumpDeviceClass(DeviceClass* dc);
  DeviceClass* createDeviceClass(QList<QString> &list);
  DMXDevice* createDevice(QList<QString> &list);
  Function* createFunction(QList<QString> &list);
  void createFunctionContents(QList<QString> &list);
  void findPluginObjects();

 private:
  QList <Function> m_functions;

  DMXChannel* m_DMXAddressAllocation[512];

  QString m_workspaceFileName;

  DMX* m_dmx;

  bool m_modified;

  QList <DMXDevice> m_deviceList;
  QList <DeviceClass> m_deviceClassList;

  QList <InputDevice> m_inputDeviceList;

  QList <Plugin> m_pluginList;

  QList <Bus> m_busList;
};

#endif

