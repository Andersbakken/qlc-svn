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
#include "functioncollection.h"
#include "chaser.h"
#include "virtualconsole.h"
#include "dummyoutplugin.h"
#include "devicemanagerview.h"
#include "devicemanager.h"
#include "devicelistview.h"

#include "../../libs/common/plugin.h"
#include "../../libs/common/filehandler.h"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qlist.h>
#include <qmessagebox.h>

#include <ctype.h>
#include <dlfcn.h>

extern App* _app;

int Doc::NextPluginID = PLUGIN_ID_MIN;

Doc::Doc() : QObject()
{
  m_workspaceFileName = QString("noname.qlc");
  setModified(false);

  m_outputPlugin = NULL;
  m_dummyOutPlugin = NULL;
}

Doc::~Doc()
{
  DMXDevice* dev = NULL;
  while (m_deviceList.isEmpty() == false)
    {
      dev = m_deviceList.take(0);
      delete dev;
    }
}

void Doc::setModified(bool modified)
{
  m_modified = modified;

  if (modified == true)
    {
      _app->setCaption("Q Light Controller 2 - " + workspaceFileName() + QString("*"));
    }
  else
    {
      _app->setCaption(QString("Q Light Controller 2 - ") + workspaceFileName());
    }
}

void Doc::init()
{
  initDMXChannels();
  initPlugins();
  slotChangeOutputPlugin(_app->settings()->outputPlugin());
}

void Doc::initDMXChannels()
{
  for (unsigned short i = 0; i < 512; i++)
    {
      m_DMXAddressAllocation[i] = new DMXChannel(i);
    }
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
      QMessageBox::warning(_app, "QLC", "Unable to open or read from device directory! Check settings and permissions.");
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
      FileHandler::readFileToList(path, list);
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
      msg.sprintf("No channels specified for device class \"" + dc->manufacturer() +
		  QString(" ") + dc->model() + QString("\".\n") +
		  "Use the device class editor to add one or more channels.");
      QMessageBox::warning(_app, IDS_APP_NAME_SHORT, msg);
    }

  return dc;
}


bool Doc::readFileToList(QString &fileName, QList<QString> &list)
{
  qDebug("Doc::readFileToList() is deprecated; Use FileHandler::readFileToList() instead!");
  ASSERT(false);

  return false;
}


bool Doc::loadWorkspaceAs(QString &fileName)
{
  bool success = false;

  QString buf;
  QString s;
  QString t;
  QList<QString> list;

  newDocument();

  if (FileHandler::readFileToList(fileName, list) == true)
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

		  _app->virtualConsole()->setDefaultSpeedBus(bus);
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
  
  setModified(false);

  return success;
}

void Doc::createJoystickContents(QList<QString> &list)
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
      QString msg;
      msg = QString("Unable to add device \"" + name +
		    QString("\" because device class information is missing."));
      QMessageBox::critical(_app, IDS_APP_NAME_SHORT, msg);

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
	  QMessageBox::critical(_app, IDS_APP_NAME_SHORT, msg);
	  return NULL;
	}
      else
	{
	  if (dc->channels()->count() == 0)
	    {
	      QString msg;
	      msg = QString("No channels specified for device class \"" + dc->manufacturer() +
			    QString(" ") + dc->model() + QString("\".\n") +
			    QString("Unable to load device \"") + name + QString("\" to workspace"));

	      QMessageBox::warning(_app, IDS_APP_NAME_SHORT, msg);
	      return NULL;
	    }
	  else
	    {
	      DMXDevice* d = new DMXDevice(address, dc, name);
	      return d;
	    }
	}
    }
}

void Doc::newDocument()
{
  DMXDevice* d = NULL;
  Function* f = NULL;
  Bus* b = NULL;

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
      freeDMXAddressSpace(d->address(), d->deviceClass()->channels()->count());
      delete d;
    }

  m_workspaceFileName = QString("noname.qlc");

  setModified(false);

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
      setModified(false);

      // Current workspace file
      m_workspaceFileName = QString(fileName);

      file.close();
    }
  else
    {
      QMessageBox::critical(_app, "QLC", "Unable to open file for writing!\nCheck permissions.");
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

  setModified(true);

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

  setModified(true);

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

void Doc::initPlugins()
{
  QString path;
  QString dir = _app->settings()->pluginPath();

  // First of all, add the dummy output plugin
  m_dummyOutPlugin = new DummyOutPlugin(Doc::NextPluginID++);
  connect(m_dummyOutPlugin, SIGNAL(activated(Plugin*)), this, SLOT(slotPluginActivated(Plugin*)));
  addPlugin(m_dummyOutPlugin);

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

      probePlugin(path);
    }
}

bool Doc::probePlugin(QString path)
{
  void* handle = NULL;
  
  qDebug("Probing file: " + path);

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

	  plugin->setConfigDirectory(QString(getenv("HOME")) + QString("/") + QString(QLCUSERDIR) + QString("/"));
	  plugin->loadSettings();

	  connect(plugin, SIGNAL(activated(Plugin*)), this, SLOT(slotPluginActivated(Plugin*)));
	  addPlugin(plugin);

	  qDebug(QString("Found ") + plugin->name());
	}
    }

  return true;
}

void Doc::addPlugin(Plugin* plugin)
{
  ASSERT(plugin != NULL);
  m_pluginList.append(plugin);

  emit deviceListChanged();
}

void Doc::removePlugin(Plugin* plugin)
{
  ASSERT(plugin != NULL);
  m_pluginList.remove(plugin);

  emit deviceListChanged();
}

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

Plugin* Doc::searchPlugin(int id)
{
  Plugin* plugin = NULL;

  for (unsigned int i = 0; i < m_pluginList.count(); i++)
    {
      plugin = m_pluginList.at(i);

      if (plugin->id() == id)
	{
	  return plugin;
	}
    }

  return NULL;
}

void Doc::slotPluginActivated(Plugin* plugin)
{
  if (plugin && plugin->type() == Plugin::OutputType)
    {
      slotChangeOutputPlugin(plugin->name());
      _app->settings()->setOutputPlugin(plugin->name());
      _app->settings()->save();
    }
}

void Doc::slotChangeOutputPlugin(const QString& name)
{
  if (m_outputPlugin != NULL)
    {
      m_outputPlugin->close();
    }

  if (name == QString("<None>"))
    {
      m_outputPlugin = NULL;
    }
  else
    {
      m_outputPlugin = (OutputPlugin*) searchPlugin(name, Plugin::OutputType);

      // If an output plugin cannot be found, use the dummy plugin
      if (m_outputPlugin == NULL)
	{
	  m_outputPlugin = m_dummyOutPlugin;
	}

      m_outputPlugin->open();
    }

  // This if() has to be here so that this won't get called until all
  // objects in the call chain have been created.
  if (_app && _app->deviceManagerView() && _app->deviceManagerView()->deviceManager())
    {
      _app->deviceManagerView()->deviceManager()->slotUpdateDeviceList();
    }
}

