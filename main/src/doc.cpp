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

#include "doc.h"
#include "app.h"
#include "device.h"
#include "settings.h"
#include "deviceclass.h"
#include "function.h"
#include "scene.h"
#include "function.h"
#include "functioncollection.h"
#include "chaser.h"
#include "virtualconsole.h"
#include "dummyoutplugin.h"
#include "devicemanagerview.h"
#include "configkeys.h"
#include "functionconsumer.h"

#include "../../libs/common/plugin.h"
#include "../../libs/common/filehandler.h"

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qptrlist.h>
#include <qmessagebox.h>

#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <limits.h>

extern App* _app;

t_plugin_id Doc::NextPluginID = KPluginIDMin;

//
// Constructor
//
Doc::Doc() : QObject()
{
  m_workspaceFileName = QString::null;
  setModified(false);

  m_outputPlugin = NULL;
  m_dummyOutPlugin = NULL;
}


//
// Destructor
//
Doc::~Doc()
{
  Device* dev = NULL;
  while (m_deviceList.isEmpty() == false)
    {
      dev = m_deviceList.take(0);
      delete dev;
    }
}


//
// Set doc's modified status
//
void Doc::setModified(bool modified)
{
  m_modified = modified;
  
  QString caption(KApplicationNameLong);
  if (workspaceFileName() != QString::null)
    {
      caption += QString(" - ") + workspaceFileName();
    }

  if (modified == true)
    {
      _app->setCaption(caption + QString(" *"));
    }
  else
    {
      _app->setCaption(caption);
    }
}


//
// Initialize the Doc object
//
void Doc::init()
{
  QString outputPlugin;
  _app->settings()->get("OutputPlugin", outputPlugin);

  initPlugins();

  readDeviceClasses();

  connect(Bus::defaultFadeBus(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	  this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

  connect(Bus::defaultHoldBus(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	  this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));
}


//
// Read all device classes from files
//
bool Doc::readDeviceClasses()
{
  DeviceClass* dc = NULL;
  QString path = QString::null;

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + DEVICECLASSPATH + QString("/");

  QDir d(dir);
  d.setFilter(QDir::Files);
  d.setNameFilter("*.deviceclass");
  if (d.exists() == false || d.isReadable() == false)
    {
      QString msg("Unable to read from device directory!");
      QMessageBox::warning(_app, KApplicationNameShort, msg);
      return false;
    }

  QStringList dirlist(d.entryList());
  QStringList::Iterator it;

  QPtrList <QString> list; // Our stringlist that contains the files' contents

  // Put a slash to the end of the directory name if it isn't there
  if (dir.right(1) != QString("/"))
    {
      dir = dir + QString("/");
    }

  // Go thru all files
  for (it = dirlist.begin(); it != dirlist.end(); ++it)
    {
      path = dir + *it;
      FileHandler::readFileToList(path, list);
      dc = createDeviceClass(list);
      if (dc != NULL)
	{
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


//
// Create device class entry from file entry
//
DeviceClass* Doc::createDeviceClass(QPtrList <QString> &list)
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
	      list.next();
	      // dc->createFunction(list);
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
      msg.sprintf("No channels specified for device class \"" +
		  dc->manufacturer() +
		  QString(" ") + dc->model() + QString("\".\n") +
		  "Use the device class editor to add one or more channels.");
      QMessageBox::warning(_app, KApplicationNameShort, msg);
    }

  return dc;
}


//
// Load workspace from a given filename
//
bool Doc::loadWorkspaceAs(QString &fileName)
{
  bool success = false;

  QString buf;
  QString s;
  QString t;
  QPtrList <QString> list;

  newDocument();

  if (FileHandler::readFileToList(fileName, list) == true)
    {
      m_workspaceFileName = QString(fileName);
      
      // Create devices and functions from the list
      for (QString* string = list.first(); 
	   string != NULL; string = list.next())
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
		  // Only create the function but don't care for
		  // its contents yet
		  Function* f = createFunction(list);

		  if (f != NULL)
		    {
		      // Add function to function pool
		      addFunction(f);
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
      for (QString* string = list.first(); 
	   string != NULL; string = list.next())
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
  
  setModified(false);

  return success;
}


//
// Create joystick plugin contents... now what the heck is this
// doing here???
//
void Doc::createJoystickContents(QPtrList <QString> &list)
{
  QString name;
  QString fdName;

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
	}
    }
}


//
// Load function's contents after it has been created with
// createFunction()
//
void Doc::createFunctionContents(QPtrList <QString> &list)
{
  Function* function = NULL;
  Device* device = NULL;

  t_device_id deviceId = 0;
  t_function_id functionId = KFunctionIDMax;

  QString name;

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
	  list.next();
	}
      else if (*s == QString("ID"))
	{
	  functionId = list.next()->toULong();
	}
      else if (*s == QString("Device"))
	{
	  deviceId = list.next()->toInt();
	  if (deviceId == 0)
	    {
	      device = NULL;
	    }
	  else
	    {
	      device = searchDevice(deviceId);
	    }
	  break;
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  if (functionId == KFunctionIDMax)
    {
      qDebug("Couldn't find an ID for function <" + name + ">");
    }
  else
    {
      function = searchFunction(functionId);

      if (function != NULL)
	{
	  function->setDevice(device);
	  function->createContents(list);
	}
      else
	{
	  qDebug("Couldn't find function <" + name + ">");
	}
    }
}


//
// Create a function from a file entry
//
Function* Doc::createFunction(QPtrList <QString> &list)
{
  Function* f = NULL;
  Device* d = NULL;

  QString name;
  QString type;
  t_device_id device = 0;
  t_function_id id = KFunctionIDMax;

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
      else if (*s == QString("ID"))
	{
	  id = list.next()->toInt();
	}
      else if (*s == QString("Device"))
	{
	  device = list.next()->toInt();
	  break;
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  if (id == KFunctionIDMax)
    {
      f = NULL;
    }
  else
    {
      if (device == 0)
	{
	  d = NULL;
	}
      else
	{
	  d = searchDevice(device);
	  if (d == NULL)
	    {
	      // This function's device was not found
	      qDebug("Unable to find device %d for function %s.", 
		     device, name.latin1());
	      return NULL;
	    }
	}

      if (type == QString("Collection"))
	{
	  f = static_cast<Function*> (new FunctionCollection());
	  f->setName(name);
	  f->setDevice(d);
	}
      else if (type == QString("Chaser"))
	{
	  f = static_cast<Function*> (new Chaser());
	  f->setName(name);
	  f->setDevice(d);
	}
      else if (type == QString("Scene"))
	{
	  f = static_cast<Function*> (new Scene(id));
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

  return f;
}


//
// Create a device from a file entry
//
Device* Doc::createDevice(QPtrList <QString> &list)
{
  QString name = QString::null;
  QString manufacturer = QString::null;
  QString model = QString::null;
  QString t = QString::null;
  int address = 0;
  t_device_id id = 0;

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
      else if (*s == QString("ID"))
	{
	  id = list.next()->toULong();
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

  if (id == 0 || manufacturer == QString::null || 
      manufacturer == QString::null)
    {
      QString msg;
      msg = QString("Unable to add device \"" + name +
		    QString("\" because device information is missing."));
      QMessageBox::critical(_app, KApplicationNameShort, msg);

      return NULL;
    }
  else
    {
      DeviceClass* dc = searchDeviceClass(manufacturer, model);
      if (dc == NULL)
	{
	  QString msg;
	  msg = QString("Unable to add device \"" + name + "\"." + 
			"\nNo device class description found for " +
			manufacturer + QString(" ") + model);
	  QMessageBox::critical(_app, KApplicationNameShort, msg);
	  return NULL;
	}
      else
	{
	  if (dc->channels()->count() == 0)
	    {
	      QString msg;
	      msg = QString("No channels specified for device class \"" + 
			    dc->manufacturer() +
			    QString(" ") + dc->model() + QString("\".\n") +
			    QString("Unable to load device \"") + name + 
			    QString("\" to workspace"));

	      QMessageBox::warning(_app, KApplicationNameShort, msg);
	      return NULL;
	    }
	  else
	    {
	      Device* d = new Device(address, dc, name, id);
	      return d;
	    }
	}
    }
}


//
// Clear everything and start anew
//
void Doc::newDocument()
{
  Device* d = NULL;
  Function* f = NULL;
  Bus* b = NULL;

  _app->functionConsumer()->purge();

  // Delete all buses
  m_busList.first();
  while (!m_busList.isEmpty())
    {
      b = m_busList.take(0);
      ASSERT(b);
      delete b;
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
      delete d;
    }

  m_workspaceFileName = QString("noname.qlc");

  setModified(false);

  Function::resetFunctionId();

  emit newDocumentClicked();
}


//
// Save the workspace by a given filename
//
bool Doc::saveWorkspaceAs(QString &fileName)
{
  QFile file(fileName);
  if (file.open(IO_WriteOnly))
    {
      //
      // Buses
      //
      for (Bus* b = m_busList.first(); b != NULL; b = m_busList.next())
	{
	  b->saveToFile(file);
	}

      //
      // Devices
      //
      for (Device* d = m_deviceList.first(); d != NULL; 
	   d = m_deviceList.next())
        {
	  d->saveToFile(file);
	}

      //
      // Functions
      //

      //
      // Scenes
      //
      for (Function* f = m_functions.first(); f != NULL; 
	   f = m_functions.next())
	{
	  if (f->type() == Function::Scene)
	    {
	      f->saveToFile(file);
	    }
	}

      //
      // Chasers
      //
      for (Function* f = m_functions.first(); f != NULL; 
	   f = m_functions.next())
	{
	  if (f->type() == Function::Chaser)
	    {
	      f->saveToFile(file);
	    }
	}

      //
      // Sequences
      //
      for (Function* f = m_functions.first(); f != NULL; 
	   f = m_functions.next())
	{
	  if (f->type() == Function::Sequence)
	    {
	      f->saveToFile(file);
	    }
	}

      //
      // Collections
      //
      for (Function* f = m_functions.first(); f != NULL; 
	   f = m_functions.next())
	{
	  if (f->type() == Function::Collection)
	    {
	      f->saveToFile(file);
	    }
	}

      // Save Virtual Console
      _app->virtualConsole()->saveToFile(file);

      // Mark the document unmodified
      setModified(false);

      // Current workspace file
      m_workspaceFileName = QString(fileName);

      file.close();
    }
  else
    {
      QMessageBox::critical(_app, KApplicationNameShort, 
			    "Unable to open file for writing!");
      return false;
    }

  return true;
}


//
// Save the workspace
//
bool Doc::saveWorkspace()
{
  return saveWorkspaceAs(m_workspaceFileName);
}


//
// Add a new device to list
//
bool Doc::addDevice(Device* device)
{
  ASSERT(device != NULL);
  
  m_deviceList.append(device);

  setModified(true);

  emit deviceListChanged();
  
  return true;
}


//
// Remove a device from list only when in design mode
//
bool Doc::removeDevice(Device* device)
{
  if (_app->virtualConsole()->isDesignMode())
    {
      Device* dev = NULL;
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
      
      setModified(true);
      
      emit deviceListChanged();
      
      return ok;
    }
  else
    {
      return false;
    }
}


//
// Search for a device by its run-time id number
//
Device* Doc::searchDevice(const t_device_id id)
{
  for (Device* device = m_deviceList.first(); device != NULL;
       device = m_deviceList.next())
    {
      if (device->id() == id)
	{
	  return device;
	}
    }

  return NULL;
}


//
// Search for a deviceclass by its manufacturer & model
//
DeviceClass* Doc::searchDeviceClass(const QString &manufacturer, 
				    const QString &model)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; 
       d = m_deviceClassList.next())
    {
      if (d->manufacturer() == manufacturer && d->model() == model)
	{
	  return d;
	}
    }

  return NULL;
}


//
// Search for a deviceclass by its ID
//
DeviceClass* Doc::searchDeviceClass(const t_deviceclass_id id)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; 
       d = m_deviceClassList.next())
    {
      if (d->id() == id)
	{
	  return d;
	}
    }

  return NULL;
}


//
// Add a new function to list
//
bool Doc::addFunction(const Function* function)
{
  ASSERT(function != NULL);
  m_functions.append(function);

  setModified(true);

  return true;
}


//
// Remove (and delete) a function from list only when in design mode
//
bool Doc::removeFunction(const t_function_id id, bool deleteFunction)
{
  Function* f = NULL;

  if (_app->virtualConsole()->isDesignMode())
    {
      for (f = m_functions.first(); f != NULL; f = m_functions.next())
	{
	  if (f->id() == id)
	    {
	      m_functions.take();
	      
	      if (deleteFunction)
		{
		  delete f;
		}
	      
	      break;
	    }
	}

      setModified(true);

      return true;
    }
  else
    {
      return false;
    }
}


Function* Doc::searchFunction(const t_function_id id)
{
  Function* f = NULL;
  for (f = m_functions.first(); f != NULL; f = m_functions.next())
    {
      if (f->id() == id)
	{
	  return f;
	}
    }

  return NULL;
}

///////////////
// Bus stuff //           
///////////////

//
// Add a new bus
//
void Doc::addBus(Bus* bus)
{
  ASSERT(bus != NULL);
  m_busList.append(bus);
  emit deviceListChanged();

  connect(bus, SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	  this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

  setModified(true);
}

//
// Search for a bus with its id
//
Bus* Doc::searchBus(const t_bus_id id)
{
  Bus* bus = NULL;

  for (t_bus_id i = 0; i < (t_bus_id) m_busList.count(); i++)
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

//
// Remove bus (and delete it if "deleteBus" == true)
//
void Doc::removeBus(t_bus_id id, bool deleteBus)
{
  Bus* bus = NULL;
  
  bus = searchBus(id);

  ASSERT(bus);

  m_busList.remove(bus);

  disconnect(bus, SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	     this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

  if (deleteBus == true)
    {
      delete bus;
    }

  emit deviceListChanged();

  setModified(true);
}

//
// Broadcast all bus values to all functions.
// This is not the most efficient nor latency-safe because this
// happens in QT's message posting thread.
//
void Doc::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
  QPtrListIterator <Function> it(m_functions);
  Function* function = NULL;

  while ( (function = it.current()) )
    {
      ++it;
      function->busValueChanged(id, value);
    }
}


//////////////////
// Plugin stuff //
//////////////////

//
// Search and load plugins
//
void Doc::initPlugins()
{
  QString path;

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PLUGINPATH + QString("/");

  // First of all, add the dummy output plugin
  m_dummyOutPlugin = new DummyOutPlugin(Doc::NextPluginID++);
  connect(m_dummyOutPlugin, SIGNAL(activated(Plugin*)),
	  this, SLOT(slotPluginActivated(Plugin*)));
  addPlugin(m_dummyOutPlugin);

  QDir d(dir);
  d.setFilter(QDir::Files);
  if (d.exists() == false || d.isReadable() == false)
    {
      fprintf(stderr, "Unable to access plugin directory %s.\n",
	      (const char*) dir);
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

      probePlugin(path);
    }

  //
  // Use the output plugin that user has selected previously
  //
  QString config;
  _app->settings()->get(KEY_OUTPUT_PLUGIN, config);
  Plugin* plugin = searchPlugin(config, Plugin::OutputType);
  if (plugin != NULL)
    {
      m_outputPlugin = static_cast<OutputPlugin*> (plugin);
    }
  else
    {
      m_outputPlugin = m_dummyOutPlugin;
    }

  slotPluginActivated(m_outputPlugin);
}

//
// Try if the given file is a plugin
//
bool Doc::probePlugin(QString path)
{
  void* handle = NULL;
  
  handle = ::dlopen((const char*) path, RTLD_LAZY);
  if (handle == NULL)
    {
      fprintf(stderr, "dlopen: %s\n", dlerror());
    }
  else
    {
      typedef Plugin* create_t(int);
      typedef void destroy_t(Plugin*);

      create_t* create = (create_t*) ::dlsym(handle, "create");
      destroy_t* destroy = (destroy_t*) ::dlsym(handle, "destroy");

      if (create == NULL || destroy == NULL)
	{
	  fprintf(stderr, "dlsym(init): %s\n", dlerror());
	  return false;
	}
      else
	{
	  Plugin* plugin = create(Doc::NextPluginID++);
	  ASSERT(plugin != NULL);

	  plugin->setConfigDirectory(QString(getenv("HOME")) + 
				     QString("/") + QString(QLCUSERDIR) + 
				     QString("/"));
	  plugin->loadSettings();

	  connect(plugin, SIGNAL(activated(Plugin*)), 
		  this, SLOT(slotPluginActivated(Plugin*)));
	  addPlugin(plugin);

	  qDebug(QString("Found ") + plugin->name() + " plugin");
	}
    }

  return true;
}

//
// Add a plugin to list
//
void Doc::addPlugin(Plugin* plugin)
{
  ASSERT(plugin != NULL);
  m_pluginList.append(plugin);

  emit deviceListChanged();
}

//
// Remove a plugin from list
//
void Doc::removePlugin(Plugin* plugin)
{
  ASSERT(plugin != NULL);
  m_pluginList.remove(plugin);

  emit deviceListChanged();
}

//
// Search for a plugin by its name
//
Plugin* Doc::searchPlugin(QString name)
{
  Plugin* plugin = NULL;

  for (unsigned int i = 0; i < m_pluginList.count(); i++)
    {
      plugin = m_pluginList.at(i);

      if (plugin->name() == name)
	{
	  return plugin;
	}
    }
  return NULL;
}

//
// Search for a plugin by its name & type
//
Plugin* Doc::searchPlugin(QString name, Plugin::PluginType type)
{
  Plugin* plugin = NULL;

  for (unsigned int i = 0; i < m_pluginList.count(); i++)
    {
      plugin = m_pluginList.at(i);

      if (plugin->name() == name && plugin->type() == type)
	{
	  return plugin;
	}
    }

  return NULL;
}

//
// Search plugin by its id
//
Plugin* Doc::searchPlugin(const t_plugin_id id)
{
  Plugin* plugin = NULL;

  for (t_plugin_id i = 0; i < m_pluginList.count(); i++)
    {
      plugin = m_pluginList.at(i);

      if (plugin->id() == id)
	{
	  return plugin;
	}
    }

  return NULL;
}

//
// A plugin has been activated
//
void Doc::slotPluginActivated(Plugin* plugin)
{
  if (plugin && plugin->type() == Plugin::OutputType)
    {
      slotChangeOutputPlugin(plugin->name());
      _app->settings()->set("OutputPlugin", plugin->name());
      _app->settings()->save();
    }
}

//
// Search for an output plugin and set it. If not found,
// use Dummy Output by default.
//
void Doc::slotChangeOutputPlugin(const QString& name)
{
  if (m_outputPlugin != NULL)
    {
      m_outputPlugin->close();
    }

  m_outputPlugin = (OutputPlugin*) searchPlugin(name, Plugin::OutputType);
  if (m_outputPlugin == NULL)
    {
      // If an output plugin cannot be found, use the dummy plugin
      m_outputPlugin = m_dummyOutPlugin;
    }

  m_outputPlugin->open();
  
  // This if() has to be here so that this won't get called until all
  // objects in the call chain have been created (during startup).
  if (_app && _app->deviceManagerView() && 
      _app->deviceManagerView()->deviceManager())
    {
      _app->deviceManagerView()->deviceManager()->slotUpdateDeviceList();
    }
}
