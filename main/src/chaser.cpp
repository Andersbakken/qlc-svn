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
Chaser::Chaser() : 
  Function(Function::Chaser),

  m_runOrder     (   Loop ),
  m_direction    ( Normal ),
  m_childRunning (  false ),
  m_holdTime     (      0 )
{
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
      m_steps.clear();
    }

  QValueList <t_function_id>::iterator it;
  for (it = ch->m_steps.begin(); it != ch->m_steps.end(); ++it)
    {
      m_steps.append(*it);
    }
}


//
// Destructor
//
Chaser::~Chaser()
{
  stop();
  m_steps.clear();
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
  s = QString("Type = ") + Function::typeToString(m_type) + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  s.sprintf("ID = %d\n", m_id);
  file.writeBlock((const char*) s, s.length());

  // Bus ID
  t.setNum(m_busID);
  s = QString("Bus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Device
  s = QString("Device = 0\n");
  file.writeBlock((const char*) s, s.length());

  // Steps
  s = QString("# Step entries") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  QValueList <t_function_id>::iterator it;
  for (it = m_steps.begin(); it != m_steps.end(); ++it)
    {
      s.sprintf("Function = %d\n", *it);
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
  t_function_id fid = KNoID;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Function"))
	{
	  fid = list.next()->toInt();
	  addStep(fid);
	  fid = KNoID;
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
void Chaser::addStep(t_function_id id)
{
  m_startMutex.lock();

  if (m_running == false)
    {
      m_steps.append(id);
      _app->doc()->setModified(true);
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }  

  m_startMutex.unlock();
}


//
// Remove a step
//
void Chaser::removeStep(int index)
{
  ASSERT(((unsigned int)index) < m_steps.count());

  m_startMutex.lock();

  if (m_running == false)
    {
      m_steps.remove(m_steps.at(index));
      _app->doc()->setModified(true);
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
	  qDebug("Fucking valuelist...");
	  _app->doc()->setModified(true);
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
	  qDebug("Fucking valuelist...");
	  _app->doc()->setModified(true);
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
			      new FunctionStopEvent(m_id));

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
  Direction dir = m_direction;

  // Calculate starting values
  init();

  QValueList <t_function_id>::iterator it;
  
  if (dir == Reverse)
    {
      // Start from end
      it = m_steps.end();
      --it;
    }
  else
    {
      // Start from beginning
      it = m_steps.begin(); 
    }

  while ( !m_stopped )
    {
      if (*it)
	{
	  m_childRunning = true;
	  Function* f = _app->doc()->function(*it);
	  if (!f)
	    {
	      qDebug("Chaser step function <id:%d> deleted!", *it);
	    }
	  else if (f->engage(this))
	    {
	      while (m_childRunning && !m_stopped) sched_yield();

	      // Check if we need to be stopped
	      if (m_stopped)
		{
		  f->stop();
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
	      qDebug("Chaser step function <id:%d> is already running!", *it);
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
	  it = m_steps.begin();
	}
      else if (m_runOrder == PingPong)
	{
	  if (dir == Normal)
	    {
	      // it.current() is NULL now, but because dir == Normal,
	      // we can start from the last-1 item
	      it = m_steps.end();
	      --it; --it; // Yes, twice
	      dir = Reverse;
	    }
	  else
	    {
	      // it.current() is NULL now, but because dir == Reverse,
	      // we can start from the first+1 item
	      it = m_steps.begin();
	      ++it; // Once
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
