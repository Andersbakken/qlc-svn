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
#include "event.h"
#include "deviceclass.h"
#include "doc.h"
#include "app.h"
#include "sequenceprovider.h"
#include "scene.h"
#include "feeder.h"

#include <stdlib.h>

extern App* _app;

Chaser::Chaser() : Function()
{
  m_type = Function::Chaser;
  m_running = false;
  m_OKforNextStep = true;
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
  if (m_running == false)
    {
      ChaserStep* step = (ChaserStep*) malloc(sizeof(ChaserStep));
      
      step->callerDevice = device;
      step->feederFunction = function;
      
      m_steps.append(step);
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }  
}

void Chaser::removeStep(int index)
{
  if (m_running == false)
    {
      ChaserStep* step = m_steps.take(index);
      if (step != NULL)
	{
	  delete step;
	}
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
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

void Chaser::recalculateSpeed(Feeder* feeder)
{
  for (unsigned long i = 0; i < m_steps.count(); i++)
    {
      m_steps.at(i)->feederFunction->recalculateSpeed(feeder);
    }
}

void Chaser::registerFunction(Feeder* feeder)
{
  m_running = true;
  m_OKforNextStep = true;

  feeder->setNextEventIndex(0);
  Function::registerFunction(feeder);
}

Event* Chaser::getEvent(Feeder* feeder)
{
  Event* event = new Event();
  ChaserStep* step = NULL;

  ASSERT(feeder != NULL);

  if (m_OKforNextStep == true)
    {
      step = m_steps.at(feeder->nextEventIndex());
      feeder->setNextEventIndex((feeder->nextEventIndex() + 1) % m_steps.count());

      ASSERT(step != NULL);

      m_OKforNextStep = false;

      _app->sequenceProvider()->registerEventFeeder(step->feederFunction, feeder->speedBus(), step->callerDevice, this);
      
      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));
      
      connect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
	      this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));
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

void Chaser::unRegisterFunction()
{
  disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, Device*, unsigned long)),
	     this, SLOT(slotFunctionUnRegistered(Function*, Function*, Device*, unsigned long)));

  m_running = false;
  m_OKforNextStep = false;

  Function::unRegisterFunction();
}
