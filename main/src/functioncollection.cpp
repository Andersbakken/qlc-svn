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
#include "function.h"
#include "deviceclass.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "eventbuffer.h"

#include <qapplication.h>
#include <qstring.h>
#include <qthread.h>
#include <stdlib.h>
#include <qfile.h>
#include <qptrlist.h>
#include <assert.h>

extern App* _app;

//
// Standard constructor
//
FunctionCollection::FunctionCollection()
  : Function(Function::Collection),

    m_childCount      (     0 ),
    m_childCountMutex ( false )
{
}


//
// Copy give function's contents to this
//
void FunctionCollection::copyFrom(FunctionCollection* fc, bool append)
{
  m_startMutex.lock();

  if (!m_running)
    {
      m_name = fc->m_name;

      if (!append)
	{
	  m_steps.clear();
	}

      QValueList <t_function_id>::iterator it;
      for (it = fc->m_steps.begin(); it != fc->m_steps.end(); ++it)
	{
	  m_steps.append(*it);
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

  m_steps.clear();
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
  s = QString("Type = ") + Function::typeToString(m_type) + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  s.sprintf("ID = %d\n", id());
  file.writeBlock((const char*) s, s.length());

  // Device
  s.sprintf("Device = %d\n", m_deviceID);
  file.writeBlock((const char*) s, s.length());

  // Steps
  QValueList <t_function_id>::iterator it;
  for (it = m_steps.begin(); it != m_steps.end(); ++it)
    {
      s.sprintf("Function = %d\n", *it);
      file.writeBlock((const char*) s, s.length());
    }
}


//
// Create this function's contents from a list
//
void FunctionCollection::createContents(QPtrList <QString> &list)
{
  t_function_id fid = KNoID;
  
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
	  fid = list.next()->toInt();
	  addItem(fid);
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
bool FunctionCollection::addItem(t_function_id id)
{
  m_startMutex.lock();

  if (!m_running)
    {
      m_steps.append(id);

      m_startMutex.unlock();
      return true;
    }

  m_startMutex.unlock();
  return false;
}


//
// Remove a function from this collection (direct function pointer)
//
bool FunctionCollection::removeItem(t_function_id id)
{
  bool result = false;

  m_startMutex.lock();

  if (!m_running)
    {
      m_steps.remove(id);
      result = true;
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
			      new FunctionStopEvent(m_id));
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
  QValueList <t_function_id>::iterator it;
  for (it = m_steps.begin(); it != m_steps.end(); ++it)
    {
      Function* f = _app->doc()->function(*it);
      if (f)
	{
	  f->stop();
	}
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
  QValueList <t_function_id>::iterator it;

  // Calculate starting values
  init();
  
  m_stopped = false;
  for (it = m_steps.begin(); it != m_steps.end(); ++it)
    {
      Function* f = _app->doc()->function(*it);
      if (f && f->engage(this))
	{
	  m_childCountMutex.lock();
	  m_childCount++;
	  m_childCountMutex.unlock();
	}
      else
	{
	  qDebug("Collection unable to start function <id:%d>", *it);
	}
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
