/*
  Q Light Controller
  chaser.cpp
  
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

#include "chaser.h"
#include "deviceclass.h"
#include "doc.h"
#include "app.h"
#include "scene.h"
#include "device.h"
#include "functionstep.h"
#include "functionconsumer.h"
#include "eventbuffer.h"

#include <stdlib.h>
#include <qfile.h>
#include <sched.h>
#include <qapplication.h>

extern App* _app;

//
// Standard constructor
//
Chaser::Chaser(t_function_id id) : Function(id)
{
  m_type = Function::Chaser;
}


//
// Copy constructor
//
Chaser::Chaser(Chaser* ch, bool append) : Function(ch->id())
{
  copyFrom(ch, append);
}


//
// Copy the contents of another chaser into this
// If append == true, existing contents will not be cleared; new steps
// will appear after existing steps
//
void Chaser::copyFrom(Chaser* ch, bool append)
{
  m_name = ch->name();

  if (append == false)
    {
      while (m_steps.isEmpty() == false)
	{
	  delete m_steps.take(0);
	}
    }

  for (FunctionStep* step = ch->m_steps.first(); step != NULL; 
       step = ch->m_steps.next())
    {
      FunctionStep* newStep = new FunctionStep(step);
      m_steps.append(newStep);
    }
}


//
// Destructor
//
Chaser::~Chaser()
{
  stop();
}


//
// Save this chaser's contents into the given file
//
void Chaser::saveToFile(QFile &file)
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
  s.sprintf("ID = %d\n", m_id);
  file.writeBlock((const char*) s, s.length());

  // Device
  if (m_device != NULL)
    {
      t.setNum(m_device->id());
      s = QString("Device = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
  else
    {
      s = QString("Device = 0\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Steps
  s = QString("# Step entries") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  for (FunctionStep* step = m_steps.first(); step != NULL;
       step = m_steps.next())
    {
      ASSERT(step->function() != NULL);

      s.sprintf("Function = %d\n", step->function()->id());
      file.writeBlock((const char*) s, s.length());
    }
}


//
// Create this chaser's contents from a file that has been read into list
//
void Chaser::createContents(QPtrList <QString> &list)
{
  t_function_id functionId = 0;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Function"))
	{
	  functionId = list.next()->toULong();
	  
	  Function* function = _app->doc()->searchFunction(functionId);
	  if (function != NULL)
	    {
	      addStep(function);
	    }
	  else
	    {
	      qDebug("Unable to find member for chaser <" + name() + ">");
	    }

	  functionId = 0;
	}
      else
	{
	  // Unknown keyword
	  list.next();
	}
    }
}


//
// Add a new step into the end
//
void Chaser::addStep(Function* function)
{
  m_startMutex.lock();

  if (m_running == false)
    {
      FunctionStep* step = new FunctionStep(function);
      m_steps.append(step);
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }  

  m_startMutex.unlock();
}


//
// Completely remove a step (gap is removed)
//
void Chaser::removeStep(int index)
{
  ASSERT( ((unsigned)index) < m_steps.count());

  m_startMutex.lock();

  if (m_running == false)
    {
      FunctionStep* step = m_steps.take(index);
      ASSERT(step != NULL);
      delete step;
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }

  m_startMutex.unlock();
}


//
// Raise the given step once (move it one step earlier)
//
void Chaser::raiseStep(unsigned int index)
{
  m_startMutex.lock();

  if (m_running == false)
    {
      if (index > 0)
	{
	  FunctionStep* step = m_steps.take(index);
	  ASSERT(step != NULL);
	  
	  m_steps.insert(index - 1, step);
	}
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }

  m_startMutex.unlock();
}


//
// Lower the given step once (move it one step later)
//
void Chaser::lowerStep(unsigned int index)
{
  m_startMutex.lock();

  if (m_running == false)
    {
      if (index < m_steps.count() - 1)
	{
	  FunctionStep* step = m_steps.take(index);
	  ASSERT(step != NULL);

	  m_steps.insert(index + 1, step);
	}
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }

  m_startMutex.lock();
}


//
// Initiate a speed change (from a speed bus)
//
void Chaser::speedChange(long unsigned int newTimeSpan)
{
}


//
// Explicitly stop this function
//
void Chaser::stop()
{
  m_stopMutex.lock();
  m_stopped = true;
  m_stopMutex.unlock();
}


//
// Free run-time allocations
//
void Chaser::freeRunTimeData()
{
  qDebug("Chaser::freeRunTimeData");

  delete m_eventBuffer;
  m_eventBuffer = NULL;

  if (m_virtualController)
    {
      qDebug("signalling");
      QApplication::postEvent(m_virtualController,
			      new FunctionStopEvent(this));

      m_virtualController = NULL;
    }

  m_stopMutex.lock();
  m_stopped = true;
  m_stopMutex.unlock();

  m_startMutex.lock();
  m_running = false;
  m_startMutex.unlock();
}


//
// Initialize some run-time values
//
void Chaser::init()
{
  m_childRunning = false;
  m_removeAfterEmpty = false;
  // There's actually no need for an eventbuffer, but
  // because FunctionConsumer does EventBuffer::get() calls, it must be
  // there... So allocate a zero length buffer.
  m_eventBuffer = new EventBuffer(0, 0); 
}


//
// Main producer thread
//
void Chaser::run()
{
  // Calculate starting values
  init();

  QPtrListIterator <FunctionStep> it(m_steps);

  _app->functionConsumer()->cue(this);

  m_stopMutex.lock();
  m_stopped = false;
  while ( !m_stopped )
    {
      m_stopMutex.unlock();

      if (it.current())
	{
	  m_childRunning = true;

	  it.current()->function()->engage(this);
	  qDebug("member started");

	  while (m_childRunning) sched_yield();
	  qDebug("member stopped");

	  ++it;
	}
      else
	{
	  it.toFirst();
	}

      m_stopMutex.lock();
    }

  m_stopMutex.unlock();

  // This chaser can be removed from the list after the buffer is empty.
  // (meaning immediately because this doesn't produce any events).
  m_removeAfterEmpty = true;
  qDebug("Chaser stopped");
}

void Chaser::childFinished()
{
  m_childRunning = false;
}
