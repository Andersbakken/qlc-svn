/*
  Q Light Controller
  doc.cpp
  
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

#include "dmx.h"
#include "device.h"
#include "dmxdevice.h"
#include "doc.h"
#include "app.h"
#include "function.h"
#include "settings.h"
#include "logicalchannel.h"
#include "deviceclass.h"
#include "dmxchannel.h"
#include "scene.h"
#include "function.h"
#include "sequence.h"
#include "functioncollection.h"
#include "chaser.h"
#include "virtualconsole.h"
#include "joystickplugin.h"
#include "../../libs/common/qlcplugininfo.h"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qlist.h>
#include <ctype.h>

extern App* _app;

Doc::Doc()
{
  initializeDMXChannels();

  findPluginObjects();

  m_modified = false;
}

Doc::~Doc()
{
  Device* dev;
  while (m_deviceList.isEmpty() == false)
    {
      dev = m_deviceList.take();
      delete dev;
    }
  
  delete m_dmx;
}

void Doc::initializeDMXChannels()
{
  for (int i = 0; i < 512; i++)
    {
      m_DMXAddressAllocation[i] = new DMXChannel(i + 1);
    }
}

bool Doc::isDMXAddressSpaceFree(int address, int channels)
{
  ASSERT(channels > 0 && address > 0);
  for (int i = address - 1; i < address + channels - 1; i++)
    {
      if (m_DMXAddressAllocation[i]->isFree() == false)
	{
	  return false;
	}
    }

  return true;
}

bool Doc::allocateDMXAddressSpace(int address, int channels)
{
  ASSERT(channels > 0 && address > 0);
  if (isDMXAddressSpaceFree(address, channels) == false)
    {
      return false;
    }
  else
    {
      for (int i = address - 1; i < address + channels - 1; i++)
	{
	  m_DMXAddressAllocation[i]->allocate();
	}
      return true;
    }
}

bool Doc::freeDMXAddressSpace(int address, int channels)
{
  ASSERT(channels > 0 && address > 0);
  if (isDMXAddressSpaceFree(address, channels) == false)
    {
      for (int i = address - 1; i < address + channels - 1; i++)
	{
	  m_DMXAddressAllocation[i]->free();
	}
      return true;
    }
  else
    {
      return false;
    }
}

int Doc::findNextFreeDMXAddress(int channels)
{
  int i = 0;
  int address = 0;

  ASSERT(channels > 0);

  while (i < 512)
    {
      if (m_DMXAddressAllocation[i]->isFree() == true)
	{
	  bool found = true;
	  address = i + 1;
	  for (i = i; i < address + channels - 1; i++)
	    {
	      if (m_DMXAddressAllocation[i]->isFree() == false)
		{
		  break;
		  found = false;
		}
	    }

	  if (found == true)
	    {
	      return address;
	    }
	}

      i++;
    }

  return 0;
}

DMXChannel* Doc::dmxChannel(unsigned int channel)
{
  DMXChannel* dmxch = NULL;

  if (channel > 0 && channel < 512)
    {
      dmxch = m_DMXAddressAllocation[channel - 1];
    }

  return dmxch;
}

// Debugging, dumps one deviceclass with its capabilities
void Doc::dumpDeviceClass(DeviceClass* dc)
{
  QString t;
  t.setNum(dc->m_channels.count());
  qDebug("Manufacturer: " + dc->manufacturer() + "\nModel: " + dc->model() + "\nChannels: " + t + "\n");
  
  for (LogicalChannel* ch = dc->m_channels.first(); ch != NULL; ch = dc->m_channels.next())
    {
      t.setNum(ch->channel());
      QString c;
      c.setNum(ch->capabilities().count());
      qDebug("Channel:" + t + " Name:" + ch->name() + " Capabilities:" + c);
      
      for (LogicalChannel::Capability* cap = ch->m_capabilities.first(); cap != NULL; cap = ch->m_capabilities.next())
	{
	  QString foo;
	  foo.sprintf("Name:%s Lo:%d Hi:%d", (const char*) cap->name(), cap->lo(), cap->hi());
	  qDebug(foo);
	}
    }
}

bool Doc::readDeviceClasses()
{
  DeviceClass* dc = NULL;
  QString path = QString::null;
  QString dir = _app->settings()->deviceClassPath();

  QDir d(dir);
  d.setFilter(QDir::Files);
  if (d.exists() == false || d.isReadable() == false)
    {
      MSG_WARN("Unable to open or read from device directory! Check settings and permissions.");
      return false;
    }
  
  QStringList dirlist(d.entryList());
  QStringList::Iterator it;

  QList <QString> list; // Our stringlist that contains the files' contents

  // Put a slash to the end of the directory name if it isn't there
  if (dir.right(1) != QString("/"))
    {
      dir = dir + QString("/");
    }

  // Go thru all files
  for (it = dirlist.begin(); it != dirlist.end(); ++it)
    {
      path = dir + *it;
      readFileToList(path, list);
      dc = createDeviceClass(list);
      if (dc != NULL)
	{
          dc->setFile(path);
	  m_deviceClassList.append(dc);
	}

      // 03-Jan-2002 / HJu
      // The list wasn't cleared between files
      while (list.isEmpty() == false)
	{
	  delete list.take(0);
	}
    }
  return true;
}

DeviceClass* Doc::createDeviceClass(QList<QString> &list)
{
  QString entry;
  QString manufacturer;
  QString model;
  QString t;
  
  DeviceClass* dc = new DeviceClass();

  for (QString *s = list.first(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  entry = *(list.next());
	  if (entry == QString("Device Class"))
	    {
	      dc->createInfo(list);
	    }
	  else if (entry == QString("Channel"))
	    {
	      dc->createChannel(list);
	    }
	  else if (entry == QString("Function"))
	    {
	      dc->createFunction(list);
	    }
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  if (dc->m_channels.count() == 0)
    {
      QString msg;
      msg.sprintf("No channels specified for device class " + dc->manufacturer() + QString(" - ") + dc->model() + QString(". Ignored."));
      qDebug(msg);
      delete dc;
      dc = NULL;
    }

  return dc;
}

bool Doc::readFileToList(QString &fileName, QList<QString> &list)
{
  QFile file(fileName);
  QString s = QString::null;
  QString t = QString::null;
  QString buf = QString::null;
  int i = 0;

  if (file.open(IO_ReadOnly))
    {
      // First read all entries to a string list
      while (file.atEnd() == false)
	{
	  file.readLine(buf, 1024);
	  
	  // If there is no "equal" sign on this row or it begins
	  // with a hash, ignore it
	  i = buf.find(QString("="));
	  if (i > -1 && buf.left(1) != QString("#"))
	    {
	      /* Get the string up to equal sign */
	      s = buf.mid(0, i).stripWhiteSpace();
	      t = buf.mid(i + 1).stripWhiteSpace();
	      
	      list.append(new QString(s));
	      list.append(new QString(t));
	    }
	}
      
      file.close();
      return true;
    }
  else
    {
      return false;
    }
}

bool Doc::loadWorkspaceAs(QString &fileName)
{
  bool success = true;

  QString buf = QString::null;
  QString s = QString::null;
  QString t = QString::null;
  QList<QString> list;

  newDocument();

  if (readFileToList(fileName, list) == true)
    {
      m_workspaceFileName = QString(fileName);
      
      // Create devices and functions from the list
      for (QString* string = list.first(); string != NULL; string = list.next())
	{
	  if (*string == QString("Entry"))
	    {
	      string = list.next();

	      if (*string == QString("Device"))
		{
		  Device* d = createDevice(list);

		  if (d != NULL)
		    {
		      addDevice(d);
		    }
		}
	      else if (*string == QString("Function"))
		{
		  // Only create the functions but don't care for
		  // their contents yet
		  Function* f = createFunction(list);

		  if (f != NULL)
		    {
		      m_functions.append(f);
		    }
		}
	      else if (*string == QString("Bus"))
		{
		  Bus* bus = new Bus();
		  bus->createContents(list);
		}
	      else if (*string == QString("Frame"))
		{
		  list.prev();
		  _app->virtualConsole()->createWidget(list);
		}
	      else if (*string == QString("Button"))
		{
		  list.prev();
		  _app->virtualConsole()->createWidget(list);
		}
	      else if (*string == QString("SpeedSlider"))
		{
		  list.prev();
		  _app->virtualConsole()->createWidget(list);
		}
	      else
		{
		  // Reel back one step, it might contain the keyword
		  // "Entry"
		  string = list.prev();
		}
	    }
	}

      // Now put contents to functions
      // The functions are given their contents after every function
      // object has been created. Otherwise some functions can not
      // be created.
      for (QString* string = list.first(); string != NULL; string = list.next())
	{
	  if (*string == QString("Entry"))
	    {
	      string = list.next();
	      if (*string == QString("Function"))
		{
		  createFunctionContents(list);
		}
	      else
		{
		  // Reel back one step, it might contain the keyword
		  // "Entry"
		  string = list.prev();
		}
	    }
	}

    }
  
  m_modified = false;

  return success;
}

void Doc::createFunctionContents(QList<QString> &list)
{
  Function* f = NULL;

  QString name = QString::null;
  QString type = QString::null;
  QString t = QString::null;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());
	}		      
      else if (*s == QString("Type"))
	{
	  type = *(list.next());
	  break;
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  if (name != QString::null && type != QString::null)
    {
      f = searchFunction(name);
      if (f != NULL && f->typeString() == type)
	{
	  f->createContents(list);
	}
      else
	{
	  qDebug("Unable to create function contents for " + name);
	}
    }
}

Function* Doc::createFunction(QList<QString> &list)
{
  Function* f = NULL;

  QString name = QString::null;
  QString type = QString::null;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());
	}		      
      else if (*s == QString("Type"))
	{
	  type = *(list.next());
	  break;
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  if (name != QString::null && type != QString::null)
    {
      if (type == QString("Collection"))
	{
	  f = (Function*) new FunctionCollection();
	  f->setName(name);
	}
      else if (type == QString("Chaser"))
	{
	  f = (Function*) new Chaser();
	  f->setName(name);
	}
      else if (type == QString("Scene"))
	{
	  f = (Function*) new Scene();
	  f->setName(name);
	}
      else if (type == QString("Sequence"))
	{
	  f = NULL;
	}
      else
	{
	  f = NULL;
	}
    }
  else
    {
      f = NULL;
    }

  return f;
}

Device* Doc::createDevice(QList<QString> &list)
{
  QString name = QString::null;
  QString manufacturer = QString::null;
  QString model = QString::null;
  QString t = QString::null;
  int address = 0;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());
	}		      
      else if (*s == QString("Manufacturer"))
	{
	  manufacturer = *(list.next());
	}
      else if (*s == QString("Model"))
	{
	  model = *(list.next());
	}
      else if (*s == QString("Address"))
	{
	  t = *(list.next());
	  address = t.toInt();
	}
      else
	{
	  // Unknown keyword
	  list.next();
	}
    }

  if (name == QString::null || manufacturer == QString::null ||
      manufacturer == QString::null || address == 0)
    {
      qDebug(QString("Unable to add device:") + name + manufacturer + model);
      return NULL;
    }
  else
    {
      DeviceClass* dc = searchDeviceClass(manufacturer, model);
      if (dc == NULL)
	{
	  qDebug("No device description found for" + manufacturer + QString(" ") + model);
	  return NULL;
	}
      else
	{
	  DMXDevice* d = new DMXDevice(address, dc, name);
	  return d;
	}
    }
}

void Doc::newDocument()
{
  Device* d = NULL;
  Function* f = NULL;
  Bus* b = NULL;

  // Delete all buses
  m_busList.first();
  while (!m_busList.isEmpty())
    {
      b = m_busList.take();
      ASSERT(b);
      delete b;
    }

  // Delete all global functions
  m_functions.first();
  while (!m_functions.isEmpty())
    {
      f = m_functions.take();
      ASSERT(f);
      delete f;
    }

  // Delete all devices
  m_deviceList.first();
  while (!m_deviceList.isEmpty())
    {
      d = m_deviceList.take();
      ASSERT(d);
      freeDMXAddressSpace(d->address(), d->deviceClass()->m_channels.count());
      delete d;
    }

  m_modified = false;

  m_workspaceFileName = QString("");

  emit deviceListChanged();
}

bool Doc::saveWorkspaceAs(QString &fileName)
{
  QFile file(fileName);
  if (file.open(IO_WriteOnly))
    {
      Device* d;
      Function* f;
      Bus* b;

      // Save Buses
      for (b = m_busList.first(); b != NULL; b = m_busList.next())
	{
	  b->saveToFile(file);
	}

      // Save devices & their functions
      for (d = m_deviceList.first(); d != NULL; d = m_deviceList.next())
        {
	  d->saveToFile(file);
	}

      // Save global functions
      // Save scenes
      for (f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Scene)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save chasers
      for (f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Chaser)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Sequences
      for (f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Sequence)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Collections
      for (f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Collection)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Virtual Console
      _app->virtualConsole()->saveToFile(file);

      // Mark the document unmodified
      m_modified = false;

      // Current workspace file
      m_workspaceFileName = QString(fileName);

      file.close();
    }
  else
    {
      MSG_CRIT("Unable to open file for writing!\nCheck permissions.");
      return false;
    }

  return true;
}

bool Doc::saveWorkspace()
{
  return saveWorkspaceAs(m_workspaceFileName);
}

void Doc::initDMX()
{
  m_dmx = new DMX();
  m_dmx->open();
  m_dmx->start();
}

void Doc::addDevice(Device* device)
{
  if (device != NULL)
    {
      m_deviceList.append(device);
      m_modified = true;
      emit deviceListChanged();
    }
  else
    {
      qDebug("Error: Doc: Null Device append not allowed!");
    }
}

bool Doc::removeDevice(Device* device)
{
  Device* dev = NULL;
  bool ok = false;
  int id = -1;

  if (device != NULL)
    {
      for (dev = m_deviceList.first(); dev != NULL; dev = m_deviceList.next())
	{
	  if (dev->id() == device->id())
	    {
	      ok = m_deviceList.removeRef(dev);
	      id = dev->id();
	      delete dev;
	      break;
	    }
	}
    }
  else
    {
      qDebug("Error: Doc: Null device remove not allowed!");
    }

  m_modified = true;
  emit deviceListChanged();

  return ok;
}

QList <Device> Doc::deviceList() const
{
  return ( QList <Device> ) m_deviceList;
}

/* Search for a device by its run-time id number */
Device* Doc::searchDevice(int id, DeviceClass::Protocol p)
{
  for (Device* device = m_deviceList.first(); device != NULL; device = m_deviceList.next())
    {
      if (device->id() == id && (device->deviceClass()->protocol() == p || p == DeviceClass::ANY))
	{
	  return device;
	}
    }

  return NULL;
}

/* Search for a device by its name (not very safe, because names are not unique) */
Device* Doc::searchDevice(const QString &name, DeviceClass::Protocol p)
{
  for (Device* device = m_deviceList.first(); device != NULL; device = m_deviceList.next())
    {
      if (device->name() == name && (device->deviceClass()->protocol() == p || p == DeviceClass::ANY))
	{
	  return device;
	}
    }

  return NULL;
}

DeviceClass* Doc::searchDeviceClass(const QString &manufacturer, const QString &model)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; d = m_deviceClassList.next())
    {
      if (d->manufacturer() == manufacturer && d->model() == model)
	{
	  return d;
	}
    }

  return NULL;
}

void Doc::removeFunction(const QString &functionString)
{
  Function* f = NULL;

  for (f = m_functions.first(); f != NULL; f = m_functions.next())
    {
      if (f->name() == functionString)
	{
	  delete m_functions.take();
	  break;
	}
    }
}

Function* Doc::searchFunction(const QString &fname)
{
  Function* function = NULL;
  
  Function* f = NULL;
  for (f = m_functions.first(); f != NULL; f = m_functions.next())
    {
      if (f->name() == fname)
	{
	  function = f;
	  break;
	}
    }

  return function;
}

void Doc::addBus(Bus* bus)
{
  m_busList.append(bus);
  emit deviceListChanged();
}

Bus* Doc::searchBus(unsigned int id)
{
  Bus* bus = NULL;

  for (unsigned int i = 0; i < m_busList.count(); i++)
    {
      bus = m_busList.at(i);
      ASSERT(bus);

      if (bus->id() == id)
	{
	  return bus;
	}
    }

  return NULL;
}

Bus* Doc::searchBus(QString name)
{
  Bus* bus = NULL;

  for (unsigned int i = 0; i < m_busList.count(); i++)
    {
      bus = m_busList.at(i);
      ASSERT(bus);

      if (bus->name() == name)
	{
	  return bus;
	}
    }

  return NULL;
}

void Doc::removeBus(unsigned int id, bool deleteBus)
{
  Bus* bus = NULL;
  
  bus = searchBus(id);

  ASSERT(bus);

  m_busList.remove(bus);

  if (deleteBus == true)
    {
      delete bus;
    }

  emit deviceListChanged();
}

void Doc::findPluginObjects()
{
  QString path = QString::null;
  QString dir = _app->settings()->installDir() + QString("libs/");
  qDebug("Probing %s for plugin objects...", dir.latin1());

  QDir d(dir);
  d.setFilter(QDir::Files);
  if (d.exists() == false || d.isReadable() == false)
    {
      fprintf(stderr, "Unable to access plugin directory %s.\n", dir.latin1());
      return;
    }
  
  QStringList dirlist(d.entryList());
  QStringList::Iterator it;

  Plugin* plugin = NULL;

  // Go thru all files in the directory
  for (it = dirlist.begin(); it != dirlist.end(); ++it)
    {
      path = dir + *it;
      
      plugin = new Plugin(path);
      if (plugin->isValid() == true)
	{
	  qDebug("Library: %s | Version: %s", plugin->name().latin1(), 
		 plugin->versionString().latin1());

	  if (QString(plugin->typeString()) == QString(JOYSTICK_TYPE_STRING))
	    {
	      delete plugin;
	      plugin = NULL;
	      JoystickPlugin* jp = new JoystickPlugin(path);
	      m_pluginList.append((Plugin*) jp);
	    }
	}
      else
	{
	  delete plugin;
	}
    }

  qDebug("Found %d plugin object(s)", m_pluginList.count());
}



