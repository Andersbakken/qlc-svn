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
#include "bus.h"
#include "scene.h"
#include "device.h"
#include "functionstep.h"
#include "functionconsumer.h"
#include "eventbuffer.h"

#include <stdlib.h>
#include <unistd.h>
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

  m_direction = Normal;
  m_runOrder = Loop;

  m_holdTime = 0;
  setBus(KBusIDDefaultHold);
}


//
// Copy the contents of another chaser into this
// If append == true, existing contents will not be cleared; new steps
// will appear after existing steps
//
void Chaser::copyFrom(Chaser* ch, bool append)
{
  m_name = ch->name();
  m_direction = ch->direction();
  m_runOrder = ch->runOrder();

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
  s = QString("Device = 0\n");
  file.writeBlock((const char*) s, s.length());

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

  // Direction
  s.sprintf("Direction = %d\n", (int) m_direction);
  file.writeBlock((const char*) s, s.length());

  // Run order
  s.sprintf("RunOrder = %d\n", (int) m_runOrder);
  file.writeBlock((const char*) s, s.length());
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
      else if (*s == QString("Direction"))
	{
	  m_direction = (Direction) list.next()->toInt();
	}
      else if (*s == QString("RunOrder"))
	{
	  m_runOrder = (RunOrder) list.next()->toInt();
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
// Set run order
//
void Chaser::setRunOrder(RunOrder ro)
{
  m_runOrder = ro;
}


//
// Set direction
//
void Chaser::setDirection(Direction dir)
{
  m_direction = dir;
}

//
// Initiate a speed change (from a speed bus)
//
void Chaser::busValueChanged(t_bus_id id, t_bus_value value)
{
  if (id == m_busID)
    {
      m_holdTime = value;
    }
}


//
// Free run-time allocations
//
void Chaser::freeRunTimeData()
{
  delete m_eventBuffer;
  m_eventBuffer = NULL;

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

  m_stopped = false;

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
  m_stopped = false;

  // There's actually no need for an eventbuffer, but
  // because FunctionConsumer does EventBuffer::get() calls, it must be
  // there... So allocate a zero length buffer.
  m_eventBuffer = new EventBuffer(0, 0); 

  // Add this to function consumer
  _app->functionConsumer()->cue(this);
}


//
// Main producer thread
//
void Chaser::run()
{
  struct timespec timeval;
  struct timespec timerem;
  FunctionStep* step = NULL;

  Direction dir = m_direction;

  // Calculate starting values
  init();

  QPtrListIterator <FunctionStep> it(m_steps);

  if (dir == Reverse)
    {
      // Start from end
      it.toLast();
    }
  else
    {
      // Start from beginning
      it.toFirst();
    }

  while ( !m_stopped )
    {
      step = it.current();

      if (step)
	{
	  m_childRunning = true;
	  if (step->function()->engage(this))
	    {
	      while (m_childRunning && !m_stopped) sched_yield();

	      // Check if we need to be stopped
	      if (m_stopped)
		{
		  step->function()->stop();
		}
	      else if (m_holdTime > 0)
		{
		  // Because nanosleep sleeps at least 10msecs, don't
		  // sleep at all if holdtime is zero.
		  timeval.tv_sec = m_holdTime / KFrequency;
		  timeval.tv_nsec = (m_holdTime % KFrequency) * 
		    (1000000000 / KFrequency);
		  
		  nanosleep(&timeval, &timerem);
		}
	    }
	  else
	    {
	      qDebug("Chaser: " + it.current()->function()->name() + 
		     " is already running! Skip to next function.");
	    }

	  if (dir == Normal)
	    {
	      ++it;
	    }
	  else
	    {
	      --it;
	    }
	}
      else if (m_runOrder == Loop)
	{
	  it.toFirst();
	}
      else if (m_runOrder == PingPong)
	{
	  if (dir == Normal)
	    {
	      // it.current() is NULL now, but because dir == Normal,
	      // we can start from the last-1 item
	      it.toLast();
	      --it;
	      dir = Reverse;
	    }
	  else
	    {
	      // it.current() is NULL now, but because dir == Reverse,
	      // we can start from the first+1 item
	      it.toFirst();
	      ++it;
	      dir = Normal;
	    }
	}
      else // if (m_runOrder == SingleShot)
	{
	  break;
	}
    }

  // This chaser can be removed from the list after the buffer is empty.
  // (meaning immediately because this doesn't produce any events).
  m_removeAfterEmpty = true;
}


//
// Currently running child function calls this function
// when it is ready. This function wakes up the chaser
// producer thread.
//
void Chaser::childFinished()
{
  m_childRunning = false;
}
