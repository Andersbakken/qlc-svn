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

//
// Constructor
//
Doc::Doc() : QObject()
{
  m_fileName = QString::null;

  //
  // Allocate function array
  //
  m_functionArray = (Function**) 
    malloc(sizeof(Function*) * KFunctionArraySize);
  for (t_function_id i = 0; i < KFunctionArraySize; i++)
    {
      m_functionArray[i] = NULL;
    }

  //
  // Allocate device array
  //
  m_deviceArray = (Device**) malloc(sizeof(Device*) * KDeviceArraySize);
  for (t_device_id i = 0; i < KDeviceArraySize; i++)
    {
      m_deviceArray[i] = NULL;
    }

  setModified(false);

  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}


//
// Destructor
//
Doc::~Doc()
{
  //
  // Delete all functions
  //
  for (t_function_id i = 0; i < KFunctionArraySize; i++)
    {
      if (m_functionArray[i])
	{
	  delete m_functionArray[i];
	  m_functionArray[i] = NULL;
	}
    }

  delete [] m_functionArray;

  //
  // Delete all devices
  //
  for (t_device_id i = 0; i < KDeviceArraySize; i++)
    {
      if (m_deviceArray[i])
	{
	  delete m_deviceArray[i];
	  m_deviceArray[i] = NULL;
	}
    }

  delete [] m_deviceArray;
}


//
// Mode changed
//
void Doc::slotModeChanged()
{
  Function* f = NULL;
  if (_app->mode() == App::Operate)
    {
      //
      // Arm all functions, allocate anything that is needed
      // during run-time.
      //
      for (int i = 0; i < KFunctionArraySize; i++)
	{
	  f = m_functionArray[i];
	  if (f)
	    {
	      f->arm();
	    }
	}
    }
  else
    {
      //
      // Disarm all functions, delete anything that was
      // allocated above.
      //
      for (int i = 0; i < KFunctionArraySize; i++)
	{
	  f = m_functionArray[i];
	  if (f)
	    {
	      f->disarm();
	    }
	}
    }
}


//
// Set doc's modified status
//
void Doc::setModified(bool modified)
{
  m_modified = modified;
  
  QString caption(KApplicationNameLong);
  if (fileName() != QString::null)
    {
      caption += QString(" - ") + fileName();
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

  bool result = false;

  if (FileHandler::readFileToList(fileName, list) == true)
    {
      result = true;
      m_fileName = QString(fileName);
      
      // Create devices and functions from the list
      for (QString* string = list.first(); string != NULL;
	   string = list.next())
	{
	  if (*string == QString("Entry"))
	    {
	      string = list.next();
	      
	      if (*string == QString("Device"))
		{
		  Device::create(list);
		}
	      else if (*string == QString("Function"))
		{
		  Function::create(list);
		}
	      else if (*string == QString("Bus"))
		{
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
      
      //
      // Set the last workspace name
      //
      _app->settings()->set(KEY_LAST_WORKSPACE_NAME, m_fileName);

      emit deviceListChanged();
      emit functionListChanged();
    }

  setModified(false);
  
  return result;
}


//
// Save the workspace
//
bool Doc::saveWorkspace()
{
  return saveWorkspaceAs(m_fileName);
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
      for (t_device_id i = 0; i < KDeviceArraySize; i++)
        {
	  if (m_deviceArray[i])
	    {
	      m_deviceArray[i]->saveToFile(file);
	    }
	}

      //////////////////////////
      // Functions            //
      //////////////////////////
      for (t_function_id i = 0; i < KFunctionArraySize; i++)
	{
	  if (m_functionArray[i])
	    {
	      m_functionArray[i]->saveToFile(file);
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
      m_fileName = QString(fileName);

      //
      // Set the last workspace name
      //
      _app->settings()->set(KEY_LAST_WORKSPACE_NAME, m_fileName);

      file.close();
    }
  else
    {
      QMessageBox::critical(_app, KApplicationNameShort, 
			    QString("Unable to open file for writing:\n")
			    + file.errorString());
      return false;
    }

  return true;
}


//////////////////
// Device stuff //
//////////////////
Device* Doc::newDevice(DeviceClass* dc, QString name,
		       t_channel address, t_device_id id)
{
  Device* d = new Device();

  //
  // If the device was created successfully, save it to device
  // array and set its position in the array as its ID
  //
  if (d)
    {
      if (id == KNoID)
	{
	  bool ok = false;

	  for (t_device_id i = 0; i < KDeviceArraySize; i++)
	    {
	      if (m_deviceArray[i] == NULL)
		{
		  m_deviceArray[i] = d;
		  d->setID(i);
		  d->setDeviceClass(dc);
		  d->setAddress(address);
		  d->setName(name);
		  
		  ok = true;
		  break;
		}
	    }

	  if (!ok)
	    {
	      QString num;
	      num.setNum(KDeviceArraySize);
	      QMessageBox::warning(_app, KApplicationNameShort,
				   "You cannot add more than " + num +
				   " devices.");
	      delete d;
	      d = NULL;
	    }
	}
      else if (id >= 0 && id < KFunctionArraySize)
	{
	  if (m_deviceArray[id] == NULL)
	    {
	      m_deviceArray[id] = d;
	      d->setID(id);
	      d->setDeviceClass(dc);
	      d->setAddress(address);
	      d->setName(name);
	    }
	  else
	    {
	      delete d;
	      d = NULL;
	      QMessageBox::critical(_app, KApplicationNameShort,
				 "Unable to create device; ID already taken!");
	    }
	}
      else
	{
	  delete d;
	  d = NULL;
	  QMessageBox::warning(_app, KApplicationNameShort,
			       "Function ID out of bounds!");
	}

      setModified(true);
      emit deviceListChanged();
    }

  return d;
}


//
// Remove a device by a given id
//
void Doc::deleteDevice(t_device_id id)
{
  if (m_deviceArray[id])
    {
      delete m_deviceArray[id];
      m_deviceArray[id] = NULL;

      for (int i = 0; i < KFunctionArraySize; i++)
	{
	  if (m_functionArray[i] && m_functionArray[i]->device() == id)
	    {
	      deleteFunction(i);
	    }
	}

      setModified(true);
      emit deviceListChanged();
    }
  else
    {
      qDebug("No such device ID:%d", id);
    }
}


//
// Return a device from the device array
//
Device* Doc::device(t_device_id id)
{
  if (id >= 0 && id < KDeviceArraySize)
    {
      return m_deviceArray[id];
    }
  else
    {
      return NULL;
    }
}


////////////////////
// Function stuff //
////////////////////
//
// Create a new function
//
Function* Doc::newFunction(Function::Type type, t_function_id id)
{
  Function* f = NULL;

  //
  // Create the function
  //
  switch(type)
    {
    case Function::Scene:
      f = new Scene();
      break;
    case Function::Chaser:
      f = new Chaser();
      break;
    case Function::Collection:
      f = new FunctionCollection();
      break;
    case Function::Sequence:
      f = NULL;
      break;
    case Function::Undefined:
    default:
      f = NULL;
    }

  //
  // If the function was created successfully, save it to function
  // array and set its position in the array as its ID
  //
  if (f)
    {
      if (id == KNoID)
	{
	  for (t_function_id i = 0; i < KFunctionArraySize; i++)
	    {
	      if (m_functionArray[i] == NULL)
		{
		  m_functionArray[i] = f;
		  f->setID(i);
		  break;
		}
	    }
	}
      else if (id >= 0 && id < KFunctionArraySize)
	{
	  if (m_functionArray[id] == NULL)
	    {
	      m_functionArray[id] = f;
	      f->setID(id);
	    }
	  else
	    {
	      delete f;
	      QMessageBox::critical(_app, KApplicationNameShort,
		     "Unable to add function; ID already taken!");
	    }
	}
      else
	{
	  QMessageBox::warning(_app, KApplicationNameShort,
			       "Function ID out of bounds!");
	}
    }

  emit functionListChanged();

  return f;
}


//
// Remove a function by a given id
//
void Doc::deleteFunction(t_function_id id)
{
  if (m_functionArray[id])
    {
      delete m_functionArray[id];
      m_functionArray[id] = NULL;
    }

  emit functionListChanged();
}


//
// Return a function from the function array
//
Function* Doc::function(t_function_id id)
{
  if (id >= 0 && id < KFunctionArraySize)
    {
      return m_functionArray[id];
    }
  else
    {
      return NULL;
    }
}
