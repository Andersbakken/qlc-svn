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
#include <qfile.h>

extern App* _app;
static QMutex _mutex;

FunctionCollection::FunctionCollection(unsigned long id) : Function(id)
{
  m_registerCount = 0;
  m_type = Function::Collection;
  m_running = false;
}

FunctionCollection::FunctionCollection(FunctionCollection* fc) : Function(fc->id())
{
  copyFrom(fc);
}

void FunctionCollection::copyFrom(FunctionCollection* fc)
{
  m_type = Function::Collection;
  m_running = fc->m_running;
  m_registerCount = fc->m_registerCount;
  m_name = fc->m_name;

  QList <CollectionItem> *il = fc->items();

  for (CollectionItem* item = il->first(); item != NULL; item = il->next())
    {
      CollectionItem* newItem = new CollectionItem(item);
      m_items.append(newItem);

      connect(newItem->function(), SIGNAL(destroyed(unsigned long)),
	      this, SLOT(slotMemberFunctionDestroyed(unsigned long)));
    }
}

FunctionCollection::~FunctionCollection()
{
  while (m_items.isEmpty() == false)
    {
      delete m_items.take(0);
    }
}

bool FunctionCollection::unRegisterFunction(Feeder* feeder)
{
  m_running = false; // Not running anymore

  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      _app->sequenceProvider()->unRegisterEventFeeder(item->function());
    }

  disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, unsigned long)),
	     this, SLOT(slotFunctionUnRegistered(Function*, Function*, unsigned long)));

  Function::unRegisterFunction(feeder);

  return true;
}

bool FunctionCollection::registerFunction(Feeder* feeder)
{
  if (m_running == false)
    {
      m_running = true;

      // Disconnect the previous signal
      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, unsigned long)));
      
      connect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, unsigned long)),
	      this, SLOT(slotFunctionUnRegistered(Function*, Function*, unsigned long)));
      
      for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
	{
	  increaseRegisterCount();
	  item->setRegistered(true);
	  _app->sequenceProvider()->registerEventFeeder(item->function(), feeder->speedBus(), this);
	}

      Function::registerFunction(feeder);

      return true;
    }
  else
    {
      return false;
    }
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

  // ID
  s.sprintf("ID = %ld\n", id());
  file.writeBlock((const char*) s, s.length());

  // Device
  if (m_device != NULL)
    {
      s.sprintf("Device = %ld\n", m_device->id());
    }
  else
    {
      s = QString("Device = 0\n");
    }
  file.writeBlock((const char*) s, s.length());

  // For global collections, write device+scene pairs
  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      s.sprintf("Function = %ld", item->function()->id());
      file.writeBlock((const char*) s, s.length());
    }
}

void FunctionCollection::createContents(QList<QString> &list)
{
  unsigned long did = 0;
  unsigned long fid = 0;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Device"))
	{
	  did = list.next()->toULong();
	}
      else if (*s == QString("Function"))
	{
	  fid = list.next()->toULong();

	  if (did == 0)
	    {
	      Function* f = _app->doc()->searchFunction(fid);
	      if (f != NULL)
		{
		  addItem(f);
		}
	      else
		{
		  qDebug("Unable to find member for function collection <" + name() + ">");
		}
	    }
	  else
	    {
	      Function* f = NULL;
	      f = _app->doc()->searchFunction(fid);
	      if (f != NULL)
		{
		  addItem(f);
		}
	      else
		{
		  qDebug("Unable to find member %d for function collection <" + name() + ">", fid);
		}
	    }
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }
}

void FunctionCollection::slotMemberFunctionDestroyed(unsigned long fid)
{
  for (unsigned i = 0; i < m_items.count(); i++)
    {
      if (m_items.at(i)->functionId() == fid)
	{
	  delete m_items.take(i);
	}
    }
}

void FunctionCollection::addItem(Function* function)
{
  ASSERT(function != NULL);

  CollectionItem* item = new CollectionItem();
  item->setFunction(function);
  item->setRegistered(false);

  m_items.append(item);

  connect(item->function(), SIGNAL(destroyed(unsigned long)),
	  this, SLOT(slotMemberFunctionDestroyed(unsigned long)));
}

bool FunctionCollection::removeItem(Function* function)
{
  bool retval = false;

  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      if (item->function()->id() == function->id())
	{
	  delete m_items.take();
	  retval = true;
	  break;
	}
    }

  return retval;
}

bool FunctionCollection::removeItem(const unsigned long functionId)
{
  bool retval = false;

  for (CollectionItem* item = m_items.first(); item != NULL; item = m_items.next())
    {
      if (item->function()->id() == functionId)
	{
	  delete m_items.take();
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

void FunctionCollection::slotFunctionUnRegistered(Function* function, Function* controller, unsigned long feederID)
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
      item->function()->recalculateSpeed(feeder);
    }
}

Event* FunctionCollection::getEvent(Feeder* feeder)
{
  Event* event = new Event();

  _mutex.lock();
  if (m_registerCount == 0)
    {
      _mutex.unlock();

      // Disconnect the previous signal
      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, unsigned long)));

      // Ready event signals sequenceprovider that this
      // function is ready and can be unregistered
      event->m_type = Event::Ready;
    }
  else
    {
      _mutex.unlock();
    }

  return event;
}
