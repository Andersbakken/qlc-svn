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
#include "event.h"
#include "deviceclass.h"
#include "doc.h"
#include "app.h"
#include "sequenceprovider.h"
#include "scene.h"
#include "feeder.h"
#include "dmxdevice.h"

#include <stdlib.h>
#include <qfile.h>

extern App* _app;

Chaser::Chaser(unsigned long id) : Function(id)
{
  m_type = Function::Chaser;
  m_running = false;
  m_OKforNextStep = true;
}

Chaser::Chaser(Chaser* ch, bool append) : Function()
{
  copyFrom(ch, append);
}

void Chaser::copyFrom(Chaser* ch, bool append)
{
  m_running = ch->m_running;
  m_OKforNextStep = ch->m_OKforNextStep;
  m_repeatTimes = ch->m_repeatTimes;
  m_id = ch->id();
  m_name = ch->name();

  if (append == false)
    {
      while (m_steps.isEmpty() == false)
	{
	  ChaserStep* step = m_steps.take(0);
	  delete step;
	}
    }

  for (ChaserStep* step = ch->m_steps.first(); step != NULL; step = ch->m_steps.next())
    {
      ChaserStep* newStep = new ChaserStep(step);
      m_steps.append(newStep);

      connect(newStep->function(), SIGNAL(destroyed(unsigned long)),
	      this, SLOT(slotMemberFunctionDestroyed(unsigned long)));
    }
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

  // ID
  s.sprintf("ID = %ld\n", m_id);
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
      t.setNum(0);
      s = QString("Device = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Steps
  s = QString("# Step entries") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  for (ChaserStep* step = m_steps.first(); step != NULL; step = m_steps.next())
    {
      ASSERT(step->function() != NULL);

      s.sprintf("Function = %ld\n", step->function()->id());
      file.writeBlock((const char*) s, s.length());
    }
}

void Chaser::createContents(QPtrList <QString> &list)
{
  unsigned long functionId = 0;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Function"))
	{
	  DMXDevice* device = NULL;

	  functionId = list.next()->toULong();
	  
	  Function* function = _app->doc()->searchFunction(functionId, &device);
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

void Chaser::addStep(Function* function)
{
  if (m_running == false)
    {
      ChaserStep* step = new ChaserStep(function);
      m_steps.append(step);

      connect(step->function(), SIGNAL(destroyed(unsigned long)),
	      this, SLOT(slotMemberFunctionDestroyed(unsigned long)));
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }  
}

void Chaser::removeStep(int index)
{
  ASSERT( ((unsigned)index) < m_steps.count());

  if (m_running == false)
    {
      ChaserStep* step = m_steps.take(index);
      ASSERT(step != NULL);
      delete step;
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }
}

void Chaser::raiseStep(unsigned int index)
{
  if (m_running == false)
    {
      if (index > 0)
	{
	  ChaserStep* step = m_steps.take(index);
	  ASSERT(step != NULL);
	  
	  m_steps.insert(index - 1, step);
	}
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }
}

void Chaser::lowerStep(unsigned int index)
{
  if (m_running == false)
    {
      if (index < m_steps.count() - 1)
	{
	  ChaserStep* step = m_steps.take(index);
	  ASSERT(step != NULL);

	  m_steps.insert(index + 1, step);
	}
    }
  else
    {
      qDebug("Chaser is running. Cannot modify steps!");
    }
}

void Chaser::slotMemberFunctionDestroyed(unsigned long fid)
{
  for (unsigned i = 0; i < m_steps.count(); i++)
    {
      if (m_steps.at(i)->functionId() == fid)
	{
	  delete m_steps.take(i);
	}
    }
}

void Chaser::recalculateSpeed(Feeder* feeder)
{
  for (unsigned long i = 0; i < m_steps.count(); i++)
    {
      m_steps.at(i)->function()->recalculateSpeed(feeder);
    }
}

bool Chaser::registerFunction(Feeder* feeder)
{
  if (m_running == false)
    {
      m_running = true;
      m_OKforNextStep = true;

      feeder->setNextEventIndex(0);

      Function::registerFunction(feeder);

      recalculateSpeed(feeder);

      return true;
    }
  else
    {
      return false;
    }
}

bool Chaser::unRegisterFunction(Feeder* feeder)
{
  ChaserStep* step = NULL;
  int index = (feeder->nextEventIndex() - 1 + m_steps.count()) % m_steps.count();
  step = m_steps.at(index);

  ASSERT(step != NULL);

  _app->sequenceProvider()->unRegisterEventFeeder(step->function()->device(), step->function());

  disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, DMXDevice*, unsigned long)),
	     this, SLOT(slotFunctionUnRegistered(Function*, Function*, DMXDevice*, unsigned long)));

  m_running = false;
  m_OKforNextStep = false;

  Function::unRegisterFunction(feeder);

  return true;
}

Event* Chaser::getEvent(Feeder* feeder)
{
  Event* event = new Event();
  ChaserStep* step = NULL;

  ASSERT(feeder != NULL);

  if (m_OKforNextStep == true)
    {
      m_OKforNextStep = false;
      
      step = m_steps.at(feeder->nextEventIndex());
      feeder->setNextEventIndex((feeder->nextEventIndex() + 1) % m_steps.count());
      
      ASSERT(step != NULL);
      
      _app->sequenceProvider()->registerEventFeeder(step->function(), feeder->speedBus(), step->function()->device(), this);

      disconnect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, DMXDevice*, unsigned long)),
		 this, SLOT(slotFunctionUnRegistered(Function*, Function*, DMXDevice*, unsigned long)));
      
      connect(_app->sequenceProvider(), SIGNAL(unRegistered(Function*, Function*, DMXDevice*, unsigned long)),
	      this, SLOT(slotFunctionUnRegistered(Function*, Function*, DMXDevice*, unsigned long)));
    }

  return event;
}

void Chaser::slotFunctionUnRegistered(Function* feeder, Function* controller, DMXDevice* caller, unsigned long feederID)
{
  if (controller == this)
    {
      m_OKforNextStep = true;
    }
}
