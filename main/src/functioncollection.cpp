/*
  Q Light Controller
  functioncollection.cpp
  
  Copyright (C) Heikki Junnila
  
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
#include "functionstep.h"
#include "function.h"
#include "deviceclass.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "eventbuffer.h"
#include "functionstep.h"

#include <qapplication.h>
#include <qstring.h>
#include <qthread.h>
#include <stdlib.h>
#include <qfile.h>
#include <qptrlist.h>

extern App* _app;

//
// Standard constructor
//
FunctionCollection::FunctionCollection(t_function_id id)
  : Function(id)
{
  m_type = Function::Collection;
}


//
// Copy give function's contents to this
//
void FunctionCollection::copyFrom(FunctionCollection* fc, bool append)
{
  m_startMutex.lock();

  if (!m_running)
    {
      m_type = Function::Collection;
      m_running = fc->m_running;
      m_name = fc->m_name;

      if (!append)
	{
	  while (!m_steps.isEmpty())
	    {
	      delete m_steps.take(0);
	    }
	}

      QPtrListIterator <FunctionStep> it(*fc->steps());
      while (it.current())
	{
	  FunctionStep* newItem = new FunctionStep(it.current());
	  m_steps.append(newItem);
	  ++it;
	}
    }

  m_startMutex.unlock();
}


//
// Destructor
//
FunctionCollection::~FunctionCollection()
{
  stop();
  while(m_running) sched_yield();

  while (m_steps.isEmpty() == false)
    {
      delete m_steps.take(0);
    }
}


//
// Save this function's contents to given file
//
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
  s.sprintf("ID = %d\n", id());
  file.writeBlock((const char*) s, s.length());

  // Device
  if (m_device != NULL)
    {
      s.sprintf("Device = %d\n", m_device->id());
    }
  else
    {
      s = QString("Device = 0\n");
    }
  file.writeBlock((const char*) s, s.length());

  // For global collections, write device+scene pairs
  for (FunctionStep* step = m_steps.first(); step != NULL;
       step = m_steps.next())
    {
      s.sprintf("Function = %d\n", step->function()->id());
      file.writeBlock((const char*) s, s.length());
    }
}


//
// Create this function's contents from a list
//
void FunctionCollection::createContents(QPtrList <QString> &list)
{
  t_function_id fid = 0;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Device"))
	{
	  list.next();
	}
      else if (*s == QString("Function"))
	{
	  fid = list.next()->toULong();

	  Function* f = NULL;
	  f = _app->doc()->searchFunction(fid);
	  if (f != NULL)
	    {
	      addItem(f);
	    }
	  else
	    {
	      qDebug("Unable to find member %d for function collection <"
		     + name() + ">", fid);
	    }
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }
}


//
// Add a function to this collection
//
bool FunctionCollection::addItem(Function* function)
{
  m_startMutex.lock();

  if (!m_running)
    {
      ASSERT(function != NULL);
      
      FunctionStep* step = new FunctionStep();
      step->setFunction(function);
      
      m_steps.append(step);
      
      m_startMutex.unlock();
      return true;
    }

  m_startMutex.unlock();
  return false;
}


//
// Remove a function from this collection (direct function pointer)
//
bool FunctionCollection::removeItem(Function* function)
{
  m_startMutex.lock();

  if (!m_running)
    {
      for (FunctionStep* step = m_steps.first(); step != NULL; 
	   step = m_steps.next())
	{
	  if (step->function()->id() == function->id())
	    {
	      delete m_steps.take();

	      m_startMutex.unlock();
	      return true;
	    }
	}
    }

  m_startMutex.unlock();
  return false;
}


//
// Remove a function from this collection (with function id)
//
bool FunctionCollection::removeItem(const t_function_id functionId)
{
  m_startMutex.lock();

  if (!m_running)
    {
      for (FunctionStep* step = m_steps.first(); step != NULL; 
	   step = m_steps.next())
	{
	  if (step->function()->id() == functionId)
	    {
	      delete m_steps.take();

	      m_startMutex.unlock();
	      return true;
	    }
	}
    }

  m_startMutex.unlock();
  return false;
}


//
// Initiate a speed change (from a speed bus)
//
void FunctionCollection::speedChange()
{
}


//
// Free run-time allocations
//
void FunctionCollection::freeRunTimeData()
{
  ASSERT(m_childCount == 0);

  delete m_eventBuffer;
  m_eventBuffer = NULL;

  m_stopped = false;

  if (m_virtualController)
    {
      QApplication::postEvent(m_virtualController,
			      new FunctionStopEvent(this));
      m_virtualController = NULL;
    }

  if (m_parentFunction)
    {
      m_parentFunction->childFinished();
      m_parentFunction = NULL;
    }

  m_startMutex.lock();
  m_running = false;
  m_startMutex.unlock();
}


//
// Stop
//
void FunctionCollection::stop()
{
  QPtrListIterator <FunctionStep> it(m_steps);
  m_stopped = true;

  while (it.current())
    {
      it.current()->function()->stop();
      ++it;
    }
}

//
// Initialize some run-time values
//
void FunctionCollection::init()
{
  m_childCountMutex.lock();
  m_childCount = 0;
  m_childCountMutex.unlock();

  m_removeAfterEmpty = false;
  m_eventBuffer = new EventBuffer(0, 0);

  // Append this function to running functions list
  _app->functionConsumer()->cue(this);
}


//
// Main producer thread
//
void FunctionCollection::run()
{
  QPtrListIterator <FunctionStep> it(m_steps);

  // Calculate starting values
  init();
  
  m_stopped = false;
  while (it.current() && !m_stopped)
    {
      if (it.current()->function()->engage(this))
	{
	  m_childCountMutex.lock();
	  m_childCount++;
	  m_childCountMutex.unlock();
	}

      ++it;
    }

  // Wait for all children to stop
  m_childCountMutex.lock();
  while (m_childCount > 0)
    {
      m_childCountMutex.unlock();
      sched_yield();
    }

  m_removeAfterEmpty = true;
}


void FunctionCollection::childFinished()
{
  m_childCountMutex.lock();
  m_childCount--;
  m_childCountMutex.unlock();
}
