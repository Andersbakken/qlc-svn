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

#include "../../libs/joystick/joystickplugin.h"
#include "../../libs/joystick/joystick.h"
#include "../../libs/common/plugininfo.h"

#include <dlfcn.h>

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qlist.h>
#include <qmessagebox.h>
#include <ctype.h>

extern App* _app;

Doc::Doc()
{
  m_modified = false;
  m_joystickPlugin = NULL;
}

Doc::~Doc()
{
  DMXDevice* dev = NULL;
  while (m_deviceList.isEmpty() == false)
    {
      dev = m_deviceList.take(0);
      delete dev;
    }

  delete m_dmx;

  if (m_joystickPlugin != NULL)
    {
      delete m_joystickPlugin;
    }
}

void Doc::init()
{
  initializeDMXChannels();
  initializeDMX();
  initializeJoystickPlugin();
}

void Doc::initializeDMXChannels()
{
  for (unsigned short i = 0; i < 512; i++)
    {
      m_DMXAddressAllocation[i] = new DMXChannel(i);
    }
}

void Doc::initializeDMX()
{
  m_dmx = new DMX();
  m_dmx->open();
  m_dmx->start();
}

bool Doc::isDMXAddressSpaceFree(unsigned short address, unsigned short channels)
{
  for (unsigned short i = address; i < address + channels; i++)
    {
      if (m_DMXAddressAllocation[i]->isFree() == false)
	{
	  return false;
	}
    }

  return true;
}

bool Doc::allocateDMXAddressSpace(unsigned short address, unsigned short channels)
{
  if (isDMXAddressSpaceFree(address, channels) == false)
    {
      return false;
    }
  else
    {
      for (unsigned short i = address; i < address + channels; i++)
	{
	  m_DMXAddressAllocation[i]->allocate();
	}
      return true;
    }
}

bool Doc::freeDMXAddressSpace(unsigned short address, unsigned short channels)
{
  if (isDMXAddressSpaceFree(address, channels) == false)
    {
      for (unsigned short i = address; i < address + channels; i++)
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

unsigned short Doc::findNextFreeDMXAddress(unsigned short channels)
{
  unsigned short i = 0;
  unsigned short address = 0;
  unsigned short retval = USHRT_MAX;

  while (i < 512)
    {
      bool found = true;

      for (i = address; i < address + channels; i++)
	{
	  if (m_DMXAddressAllocation[i]->isFree() == false)
	    {
	      found = false;
	      break;
	    }
	}

      if (found == true)
	{
	  retval = address;
	  break;
	}

      address++;
    }

  return address;
}

DMXChannel* Doc::dmxChannel(unsigned short channel)
{
  DMXChannel* dmxch = NULL;

  dmxch = m_DMXAddressAllocation[channel];

  return dmxch;
}

bool Doc::readDeviceClasses()
{
  DeviceClass* dc = NULL;
  QString path = QString::null;
  QString dir = _app->settings()->deviceClassPath();

  QDir d(dir);
  d.setFilter(QDir::Files);
  d.setNameFilter("*.deviceclass");
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
	  list.first();
	  delete list.take();
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

  if (dc->channels()->count() == 0)
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

  if (fileName == QString::null)
    {
      return false;
    }

  while (list.isEmpty() == false)
    {
      list.first();
      delete list.take();
    }

  if (file.open(IO_ReadOnly))
    {
      list.append(new QString("Entry"));
      list.append(new QString("Dummy"));

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
	      list.append(new QString(s));

	      /* Get the string after the equal sign */
	      t = buf.mid(i + 1).stripWhiteSpace();
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
  bool success = false;

  QString buf;
  QString s;
  QString t;
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
		  DMXDevice* d = createDevice(list);
		  
		  if (d != NULL)
		    {
		      addDevice(d);
		      allocateDMXAddressSpace(d->address(), d->deviceClass()->channels()->count());
		    }
		}
	      else if (*string == QString("Function"))
		{
		  // Only create the function but don't care for
		  // its contents yet
		  Function* f = createFunction(list);
		  
		  if (f != NULL)
		    {
		      if (f->device() != NULL)
			{
			  // Add function to its parent device
			  f->device()->addFunction(f);
			}
		      else
			{
			  // Add function to global functions
			  addFunction(f);
			}
		    }
		}
	      else if (*string == QString("Bus"))
		{
		  Bus* bus = new Bus();
		  bus->createContents(list);
		  addBus(bus);
		}
	      else if (*string == QString("Joystick"))
		{
		  createJoystickContents(list);
		}
	      else if (*string == QString("Virtual Console"))
		{
		  // Virtual console wants it all, go to "Entry"
		  list.prev();
		  list.prev();

		  _app->virtualConsole()->createContents(list);
		}
	      else
		{
		  // Unknown keyword, do nothing
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

      success = true;
    }
  else
    {
      success = false;
    }
  
  m_modified = false;

  return success;
}

void Doc::createJoystickContents(QList<QString> &list)
{
  QString name;
  QString fdName;

  Joystick* j = NULL;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("FDName"))
	{
	  fdName = *(list.next());
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());

	  j = joystickPlugin()->search(fdName);

	  if (j == NULL || j->name() != name)
	    {
	      QString text;
	      text.sprintf("Unable to find joystick \"%s\" from <%s> while loading workspace file.\n", (const char*) name, (const char*) fdName);
	      text += QString("Do you want to select another device (press Yes) or skip this joystick (press No)?");
	      if (QMessageBox::critical(NULL, QString("QLC"), text, QMessageBox::Yes,
					QMessageBox::No) == QMessageBox::Yes)
		{
		  j = joystickPlugin()->selectJoystick();
		}
	    }
	  
	  if (j != NULL)
	    {
	      j->createContents(list);
	      addInputDevice(j);
	      j->open();
	      j->start();
	    }
	}
    }
}

void Doc::createFunctionContents(QList<QString> &list)
{
  Function* f = NULL;
  DMXDevice* d = NULL;

  QString name;
  QString type;
  QString device;

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
	}
      else if (*s == QString("Device"))
	{
	  device = *(list.next());
	  if (device == QString("Global"))
	    {
	      d = NULL;
	    }
	  else
	    {
	      d = searchDevice(device);
	    }
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
      if (d != NULL)
	{
	  f = d->searchFunction(name);
	}
      else
	{
	  f = searchFunction(name);
	}

      if (f != NULL)
	{
	  f->setDevice(d);
	  f->createContents(list);
	}
      else
	{
	  qDebug("Invalid or missing information for function <%s>", (const char*) name);
	}
    }
  else
    {
      qDebug("Invalid or missing information for function <%s>", (const char*) name);
    }
}

Function* Doc::createFunction(QList<QString> &list)
{
  Function* f = NULL;
  DMXDevice* d = NULL;

  QString name;
  QString type;
  QString device;

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
	}
      else if (*s == QString("Device"))
	{
	  device = *(list.next());
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
      if (device == QString("Global"))
	{
	  d = NULL;
	}
      else
	{
	  d = searchDevice(device);
	  if (d == NULL)
	    {
	      // This function's device was not found
	      qDebug("Unable to find device %s for function %s. Discarding function.", (const char*) device, (const char*) name);
	      return NULL;
	    }
	}

      if (type == QString("Collection"))
	{
	  f = (Function*) new FunctionCollection();
	  f->setName(name);
	  f->setDevice(d);
	}
      else if (type == QString("Chaser"))
	{
	  f = (Function*) new Chaser();
	  f->setName(name);
	  f->setDevice(d);
	}
      else if (type == QString("Scene"))
	{
	  f = (Function*) new Scene();
	  f->setName(name);
	  f->setDevice(d);
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

DMXDevice* Doc::createDevice(QList<QString> &list)
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
      manufacturer == QString::null)
    {
      qDebug("Unable to add device %s [%s] [%s]" + name + manufacturer + model);
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
  DMXDevice* d = NULL;
  Function* f = NULL;
  Bus* b = NULL;
  Joystick* j = NULL;

  // Delete all buses
  m_busList.first();
  while (!m_busList.isEmpty())
    {
      b = m_busList.take(0);
      ASSERT(b);
      delete b;
    }

  // Delete all input devices
  m_inputDeviceList.first();
  while (!m_inputDeviceList.isEmpty())
    {
      j = m_inputDeviceList.take(0);
      ASSERT(j);
      delete j;
    }

  // Delete all global functions
  m_functions.first();
  while (!m_functions.isEmpty())
    {
      f = m_functions.take(0);
      ASSERT(f);
      delete f;
    }

  // Delete all devices
  m_deviceList.first();
  while (!m_deviceList.isEmpty())
    {
      d = m_deviceList.take(0);
      ASSERT(d);
      freeDMXAddressSpace(d->address(), d->deviceClass()->channels()->count());
      delete d;
    }

  m_modified = false;

  m_workspaceFileName = QString("");

  emit newDocumentClicked();
}

bool Doc::saveWorkspaceAs(QString &fileName)
{
  QFile file(fileName);
  if (file.open(IO_WriteOnly))
    {
      // Save Buses
      for (Bus* b = m_busList.first(); b != NULL; b = m_busList.next())
	{
	  b->saveToFile(file);
	}

      // Save devices & their functions
      for (DMXDevice* d = m_deviceList.first(); d != NULL; d = m_deviceList.next())
        {
	  d->saveToFile(file);
	}

      // Save input devices
      for (Joystick* j = m_inputDeviceList.first(); j != NULL; j = m_inputDeviceList.next())
	{
	  j->saveToFile(file);
	}

      // Save global functions
      // Save scenes
      for (Function* f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Scene)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save chasers
      for (Function* f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Chaser)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Sequences
      for (Function* f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->type() == Function::Sequence)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Collections
      for (Function* f = m_functions.first(); f != NULL; f = m_functions.next())
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

void Doc::addDevice(DMXDevice* device)
{
  ASSERT(device != NULL);
  
  m_deviceList.append(device);
  m_modified = true;
  emit deviceListChanged();
}

bool Doc::removeDevice(DMXDevice* device)
{
  DMXDevice* dev = NULL;
  bool ok = false;
  int id = -1;

  ASSERT(device != NULL);

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

  m_modified = true;
  emit deviceListChanged();

  return ok;
}

/* Search for a device by its run-time id number */
DMXDevice* Doc::searchDevice(int id)
{
  for (DMXDevice* device = m_deviceList.first(); device != NULL; device = m_deviceList.next())
    {
      if (device->id() == id)
	{
	  return device;
	}
    }

  return NULL;
}

/* Search for a device by its name (not very safe, because names are not unique) */
DMXDevice* Doc::searchDevice(const QString &name)
{
  for (DMXDevice* device = m_deviceList.first(); device != NULL; device = m_deviceList.next())
    {
      if (device->name() == name)
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

DeviceClass* Doc::searchDeviceClassByID(int id)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; d = m_deviceClassList.next())
    {
      if (d->id() == id)
	{
	  return d;
	}
    }

  return NULL;
}

void Doc::addFunction(const Function* function)
{
  ASSERT(function != NULL);
  m_functions.append(function);
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
  ASSERT(bus != NULL);
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

void Doc::initializeJoystickPlugin()
{
  QString path;
  QString dir = _app->settings()->pluginPath();
  void* handle = NULL;
  JoystickPlugin* plugin = NULL;

  if (m_joystickPlugin != NULL)
    {
      return;
    }

  qDebug("Probing %s for plugin objects...", (const char*) dir);

  QDir d(dir);
  d.setFilter(QDir::Files);
  if (d.exists() == false || d.isReadable() == false)
    {
      fprintf(stderr, "Unable to access plugin directory %s.\n", (const char*) dir);
      return;
    }
  
  QStringList dirlist(d.entryList());
  QStringList::Iterator it;

  for (it = dirlist.begin(); it != dirlist.end(); ++it)
    { 
      // Ignore everything else than .so files
      if ((*it).right(2) != QString("so"))
	{
	  continue;
	}

      path = dir + *it;

      qDebug("Probing file: " + path);
      
      handle = ::dlopen((const char*) path, RTLD_LAZY);
      if (handle == NULL)
	{
	  fprintf(stderr, "dlopen: %s\n", dlerror());
	}
      else
	{
	  typedef QObject* create_t();
	  typedef void destroy_t(QObject*);

	  create_t* create = (create_t*) ::dlsym(handle, "create");
	  destroy_t* destroy = (destroy_t*) ::dlsym(handle, "destroy");

	  if (create == NULL || destroy == NULL)
	    {
	      fprintf(stderr, "dlsym(init): %s\n", dlerror());
	    }
	  else
	    {
	      plugin = (JoystickPlugin*) create();
	      PluginInfo info;
	      plugin->info(info);

	      fprintf(stderr, "Plugin: %s\nType: %s\nVersion: %ld.%ld.%ld-%ld\n\n",
		      (const char*) info.name, (const char*) info.type,
		      info.version >> 24, info.version >> 16,
		      info.version >> 8, info.version & 0xff);

	      m_joystickPlugin = plugin;
	      m_joystickPlugin->init();

	      break;
	    }
	}
    }
}

Joystick* Doc::searchInputDevice(QString fdName)
{
  for (Joystick* j = m_inputDeviceList.first(); j != NULL; j = m_inputDeviceList.next())
    {
      if (fdName == j->fdName())
	{
	  return j;
	}
    }
  
  return NULL;
}

Joystick* Doc::searchInputDevice(unsigned int id)
{
  for (Joystick* j = m_inputDeviceList.first(); j != NULL; j = m_inputDeviceList.next())
    {
      if (id == j->id())
	{
	  return j;
	}
    }
  
  return NULL;
}

void Doc::addInputDevice(Joystick* j)
{
  if (searchInputDevice(j->fdName()) == NULL)
    {
      m_inputDeviceList.append(j);
      emit deviceListChanged();
    }
}

void Doc::removeInputDevice(Joystick* j)
{
  m_inputDeviceList.remove(j);
  emit deviceListChanged();
}
