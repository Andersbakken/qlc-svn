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
#include "devicemanagerview.h"
#include "configkeys.h"
#include "functionconsumer.h"

#include "../../libs/common/filehandler.h"

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qptrlist.h>
#include <qmessagebox.h>

extern App* _app;

//using namespace FunctionNS;

//
// Constructor
//
Doc::Doc() : QObject()
{
  m_workspaceFileName = QString::null;
  setModified(false);
}


//
// Destructor
//
Doc::~Doc()
{
  Device* d = NULL;
  Function* f = NULL;

  //
  // Delete all functions
  //
  m_functions.first();
  while (!m_functions.isEmpty())
    {
      f = m_functions.take(0);
      ASSERT(f);
      delete f;
    }

  //
  // Signal that function list has changed
  //
  emit functionListChanged();

  //
  // Reset function ID numbers
  //
  Function::resetID();

  //
  // Delete all devices
  //
  m_deviceList.first();
  while (!m_deviceList.isEmpty())
    {
      d = m_deviceList.take(0);
      ASSERT(d);
      delete d;
    }

  //
  // Reset device ID numbers
  //
  Device::resetID();

  //
  // Signal that device list has changed
  //
  emit deviceListChanged();
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
// Load workspace from a given filename
//
bool Doc::loadWorkspaceAs(QString &fileName)
{
  QString buf;
  QString s;
  QString t;
  QPtrList <QString> list;

  if (FileHandler::readFileToList(fileName, list) == true)
    {
      m_workspaceFileName = QString(fileName);
    }
  else
    {
      return false;
    }

  // Create devices and functions from the list
  for (QString* string = list.first(); 
       string != NULL; string = list.next())
    {
      if (*string == QString("Entry"))
	{
	  string = list.next();
	  
	  if (*string == QString("Device"))
	    {
	      Device* d = Device::create(list);
	      
	      if (d)
		{
		  addDevice(d);
		}
	    }
	  else if (*string == QString("Function"))
	    {
	      // Only create the function but not its contents
	      Function* f = FunctionNS::createFunction(list);
	      
	      if (f)
		{
		  addFunction(f);
		}
	    }
	  else if (*string == QString("Bus"))
	    {
	      // Read bus names and values
	      Bus::createContents(list);
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
  // be created if their children have not been created.
  for (QString* string = list.first(); 
       string != NULL; string = list.next())
    {
      if (*string == QString("Entry"))
	{
	  string = list.next();
	  if (*string == QString("Function"))
	    {
	      Function::createAllContents(list);
	    }
	}
    }
  
  //
  // Set the last workspace name
  //
  _app->settings()->set(KEY_LAST_WORKSPACE_NAME, workspaceFileName());
  
  setModified(false);
  
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
// Save the workspace by a given filename
//
bool Doc::saveWorkspaceAs(QString &fileName)
{
  QFile file(fileName);
  if (file.open(IO_WriteOnly))
    {
      //////////////////////////
      // Devices              //
      //////////////////////////
      for (Device* d = m_deviceList.first(); d != NULL; 
	   d = m_deviceList.next())
        {
	  d->saveToFile(file);
	}

      //////////////////////////
      // Functions            //
      //////////////////////////
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

      ///////////
      // Buses //
      ///////////
      Bus::saveToFile(file);

      /////////////////////
      // Virtual Console //
      /////////////////////
      _app->virtualConsole()->saveToFile(file);

      // Mark the document unmodified
      setModified(false);

      // Current workspace file
      m_workspaceFileName = QString(fileName);

      //
      // Set the last workspace name
      //
      _app->settings()->set(KEY_LAST_WORKSPACE_NAME, workspaceFileName());

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


//////////////////
// Device stuff //
//////////////////

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
  if (_app->mode() == App::Design)
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
// Search for a device by its id number
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




////////////////////
// Function stuff //
////////////////////

//
// Add a new function to list
//
bool Doc::addFunction(const Function* function)
{
  ASSERT(function != NULL);
  m_functions.append(function);

  setModified(true);

  emit functionListChanged();

  return true;
}


//
// Remove (and delete) a function from list only when in design mode
//
bool Doc::removeFunction(const t_function_id id, bool deleteFunction)
{
  Function* f = NULL;

  if (_app->mode() == App::Design)
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

      emit functionListChanged();

      return true;
    }
  else
    {
      return false;
    }
}


//
// Search for a function by its id
//
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
