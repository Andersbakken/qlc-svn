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
#include "device.h"
#include "scene.h"
#include "function.h"
#include "event.h"
#include "deviceclass.h"
#include "doc.h"
#include "app.h"
#include "sequenceprovider.h"
#include <stdlib.h>

extern App* _app;

Chaser::Chaser() : Function()
{
  m_type = Function::Chaser;
  m_running = false;
  m_OKforNextStep = false;
}

Chaser::~Chaser()
{
}

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

  // Device class, device name or "Global"
  if (deviceClass() != NULL)
    {
      // For device class chasers we need to save only the steps because
      // all scenes are inside the device class
      for (ChaserStep* step = m_steps.first(); step != NULL; step = m_steps.next())
	{
	  s = QString("Function = ") + step->feederFunction->name() + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
  else if (device() != NULL)
    {
      // For device chasers (that are saved in the workspace file)
      // write also the device name that this chaser is attached to
      s = QString("Device = ") + device()->name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      for (ChaserStep* step = m_steps.first(); step != NULL; step = m_steps.next())
	{
	  s = QString("Function = ") + step->feederFunction->name() + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
  else
    {
      // For global chasers, write device+scene pairs
      for (ChaserStep* step = m_steps.first(); step != NULL; step = m_steps.next())
	{
	  // Global chasers need a device+scene pair
	  s = QString("Device = ") + step->callerDevice->name() + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	  
	  s = QString("Function = ") + step->feederFunction->name() + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
}

void Chaser::createContents(QList<QString> &list)
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
      else if (*s == QString("Device"))
	{
	  device = *(list.next());
	}
      else if (*s == QString("Function"))
	{
	  function = *(list.next());
	  
	  if (device == QString("Global"))
	    {
	      Function* f = _app->doc()->searchFunction(function);
	      if (f != NULL)
		{
		  addStep(NULL, f);
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
		      addStep(d, f);
		    }
		  else if ((f = d->deviceClass()->searchFunction(function)) != NULL)
		    {
		      addStep(d, f);
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

void Chaser::addStep(Device* device, Function* function)
{
  ChaserStep* step = (ChaserStep*) malloc(sizeof(ChaserStep));

  step->callerDevice = device;
  step->feederFunction = function;

  m_steps.append(step);
}

void Chaser::removeStep(int index)
{
  ChaserStep* step = m_steps.take(index);
  if (step != NULL)
    {
      delete step;
    }
}

int Chaser::steps(void)
{ 
  return m_steps.count();
}

ChaserStep* Chaser::at(int index)
{ 
  return m_steps.at(index);
}

Event* Chaser::getEvent(Feeder* feeder)
{
  Event* event = new Event();
  ChaserStep* step = NULL;

  ASSERT(feeder != NULL);

  if (feeder->first == true)
    {
      feeder->first = false;

      if (m_running == true)
	{
	  // Each chaser only once at a time
	  event->m_type = Event::Ready;
	  return event;
	}
      else
	{
	  step = m_steps.at(feeder->nextEventIndex);
	  
	  if (step != NULL)
	    {
	      feeder->nextEventIndex++;
	      m_running = true;
	      m_OKforNextStep = false;

	      connect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
		      this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

	      _app->sequenceProvider()->registerEventFeeder(step->callerDevice, step->feederFunction, this);
	    }
	  else
	    {
	      m_OKforNextStep = false;
	      event->m_type = Event::Ready;
	      return event;
	    }
	}
    }
  else if (m_OKforNextStep == true)
    {
      step = m_steps.at(feeder->nextEventIndex);

      if (step != NULL)
	{
	  m_OKforNextStep = false;
	  feeder->nextEventIndex++;
	  _app->sequenceProvider()->registerEventFeeder(step->callerDevice, step->feederFunction, this);
	}
      else
	{
	  if (feeder->repeatTimes == 0)
	    {
	      // Repeat forever
	      feeder->nextEventIndex = 0;
	    }
	  else if (feeder->repeatCounter == feeder->repeatTimes)
	    {
	      // This chaser has been repeated as many times as wanted.
	      // Disconnect any signals this class has been receiving
	      // Signal sequence provider to unregister this
	      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
			 this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

	      m_OKforNextStep = false;
	      event->m_type = Event::Ready;
	      return event;
	    }
	  else
	    {
	      // Repeated once more
	      m_OKforNextStep = true;
	      feeder->repeatCounter++;
	      feeder->nextEventIndex = 0;
	    }
	}
    }

  return event;
}

void Chaser::slotFunctionUnRegistered(Function* feeder, Function* controller, Device* caller, unsigned long feederID)
{
  if (controller == this)
    {
      m_OKforNextStep = true;
    }
}

void Chaser::unRegister()
{
  disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
	     this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

  m_running = false;
  m_OKforNextStep = false;

  Function::unRegister();
}
