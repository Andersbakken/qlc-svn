/*
  Q Light Controller
  functioncollection.cpp
  
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

#include "functioncollection.h"
#include "device.h"
#include "function.h"
#include "app.h"
#include "sequenceprovider.h"
#include "feeder.h"
#include "event.h"
#include "doc.h"
#include "deviceclass.h"

#include <qstring.h>
#include <qthread.h>
#include <stdlib.h>

extern App* _app;
static QMutex _mutex;

FunctionCollection::FunctionCollection() : Function()
{
  m_registerCount = 0;
  m_type = Function::Collection;
}

FunctionCollection::~FunctionCollection()
{

}

void FunctionCollection::unRegisterFunction(Feeder* feeder)
{
  m_running = false; // Not running anymore

  Function::unRegisterFunction(feeder);
}

void FunctionCollection::registerFunction(Feeder* feeder)
{
  m_running = false; // Not running until the first event has been fetched

  Function::registerFunction(feeder);
}

void FunctionCollection::saveToFile(QFile &file)
{
  QString s;
  QString t;

  // Comment line
  s = QString("# Function entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Function") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + name() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Type
  s = QString("Type = ") + typeString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Device class, device name or "Global"
  if (deviceClass() != NULL)
    {
      ASSERT(false);
    }
  else if (device() != NULL)
    {
      ASSERT(false);
    }
  else
    {
      s = QString("Device = Global") + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // For global collections, write device+scene pairs
      for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
	{
	  if (item->callerDevice != NULL)
	    {
	      s = QString("Device = ") + item->callerDevice->name() + QString("\n");
	    }
	  else
	    {
	      s = QString("Device = Global") + QString("\n");
	    }

	  file.writeBlock((const char*) s, s.length());

	  s = QString("Function = ") + item->feederFunction->name() + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
}

void FunctionCollection::createContents(QList<QString> &list)
{
  QString device = QString::null;
  QString function = QString::null;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Function"))
	{
	  function = *(list.next());
	}
      else if (*s == QString("Device"))
	{
	  device = *(list.next());
	  
	  if (device == QString("Global"))
	    {
	      Function* f = _app->doc()->searchFunction(function);
	      if (f != NULL)
		{
		  addItem(NULL, f);
		}
	      else
		{
		  qDebug("Unable to find member <" + function + "> for Function Collection <" + name() + ">");
		}
	    }
	  else
	    {
	      Device* d = _app->doc()->searchDevice(device, DeviceClass::ANY);
	      Function* f = NULL;
	      if (d != NULL)
		{
		  f = d->searchFunction(function);
		  if (f != NULL)
		    {
		      addItem(d, f);
		    }
		  else if ((f = d->deviceClass()->searchFunction(function)) != NULL)
		    {
		      addItem(d, f);
		    }
		  else
		    {
		      qDebug("Unable to find member <" + function + "> for Function Collection <" + name() + ">");
		    }
		}
	      
	      device = QString::null;
	      function = QString::null;
	    }
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }
}

void FunctionCollection::addItem(Device* device, Function* function)
{
  ASSERT(function != NULL);

  CollectionItem* item = (CollectionItem*) malloc (sizeof(CollectionItem));
  item->callerDevice = device;
  item->feederFunction = function;

  m_items.append(item);
}

bool FunctionCollection::removeItem(Device* device, Function* function)
{
  bool retval = false;

  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      if (item->callerDevice == device && item->feederFunction == function)
	{
	  delete m_items.take();
	  retval = true;
	  break;
	}
    }

  return retval;
}

bool FunctionCollection::removeItem(QString deviceString, QString functionString)
{
  bool retval = false;

  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      if (item->callerDevice->name() == deviceString && item->feederFunction->name() == functionString)
	{
	  free(m_items.take());
	  retval = true;
	  break;
	}
    }

  return retval;
}

void FunctionCollection::increaseRegisterCount()
{
  _mutex.lock();
  m_registerCount++;
  _mutex.unlock();
}

void FunctionCollection::decreaseRegisterCount()
{
  _mutex.lock();
  m_registerCount--;
  _mutex.unlock();
}

void FunctionCollection::slotFunctionUnRegistered(Function* function, Function* controller, Device* caller, unsigned long feederID)
{
  if (controller == this)
    {
      decreaseRegisterCount();
    }
}

void FunctionCollection::recalculateSpeed(Feeder* feeder)
{
  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      item->feederFunction->recalculateSpeed(feeder);
    }
}

Event* FunctionCollection::getEvent(Feeder* feeder)
{
  Event* event = new Event();

  if (m_running == false)
    {
      m_running = true;

      // Disconnect the previous signal
      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

      connect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
	      this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

      for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
	{
	  increaseRegisterCount();
	  _app->sequenceProvider()->registerEventFeeder(item->feederFunction, feeder->speedBus(), item->callerDevice, this);
	}
    }

  if (m_registerCount == 0)
    {
      // Disconnect the previous signal
      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

      // Ready event signals sequenceprovider that this
      // function is ready and can be unregistered
      event->m_type = Event::Ready;
    }
  
  return event;
}
