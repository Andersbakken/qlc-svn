/*
  Q Light Controller
  scene.cpp
  
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

#include "scene.h"
#include "app.h"
#include "doc.h"
#include "event.h"
#include "settings.h"
#include "feeder.h"
#include "logicalchannel.h"
#include "dmxdevice.h"

#include <math.h>
#include <stdlib.h>
#include <iostream.h>
#include <qfile.h>

extern App* _app;

Scene::Scene(unsigned long id) : Function(id)
{
  m_type = Function::Scene;

  for (int i = 0; i < 512; i++)
    {
      m_values[i].value = 0;
      m_values[i].type = Fade;
    }
}

Scene::Scene(Scene* sc)
{
  copyFrom(sc);
}

void Scene::copyFrom(Scene* sc)
{
  m_type = Function::Scene;
  m_name = QString(sc->name());
  m_device = sc->m_device;
  m_id = sc->id();

  for (int i = 0; i < 512; i++)
    {
      m_values[i].value = sc->m_values[i].value;
      m_values[i].type = sc->m_values[i].type;
    }
}

Scene::~Scene()
{
  emit destroyed();
}

bool Scene::unRegisterFunction(Feeder* feeder)
{
  Function::unRegisterFunction(feeder);

  return true;
}

bool Scene::registerFunction(Feeder* feeder)
{
  recalculateSpeed(feeder);
  Function::registerFunction(feeder);

  return true;
}

QString Scene::valueTypeString(int ch)
{
  switch(m_values[ch].type)
    {
    case Set:
      return QString("Set");
      break;

    case Fade:
      return QString("Fade");
      break;

    default:
    case NoSet:
      return QString("NoSet");
      break;
    }
}

void Scene::saveToFile(QFile &file)
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
  t.setNum(m_id);
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  if (device() != NULL)
    {
      t.setNum(device()->id());
      s = QString("Device = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // Data
      for (unsigned short i = 0; i < device()->deviceClass()->channels()->count(); i++)
	{
	  t.setNum(i);
	  s = t + QString(" = ");
	  t.setNum(m_values[i].value);
	  s += t + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	  
	  s = QString("ValueType = ") + valueTypeString(i) + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
  else
    {
      // For global scenes the device name is "Global"
      s = QString("Device = 0") + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // TODO: write global scene data
    }
}

void Scene::createContents(QList<QString> &list)
{
  QString t;

  unsigned char ch = 0;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (s->at(0).isNumber() == true)
	{
	  ch = (unsigned char) s->toInt();
	  t = *(list.next());
	  m_values[ch].value = t.toInt();
	  m_values[ch].type = Set;
	}
      else if (*s == QString("ValueType"))
	{
	  t = *(list.next());
	  if (t == QString("Set"))
	    {
	      m_values[ch].type = Set;
	    }
	  else if (t == QString("Fade"))
	    {
	      m_values[ch].type = Fade;
	    }
	  else if (t == QString("NoSet"))
	    {
	      m_values[ch].type = NoSet;
	    }
	  else
	    {
	      list.next();
	    }
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}

bool Scene::set(unsigned short ch, unsigned char value, SceneValueType type)
{
  if (m_device != NULL)
    {
      if (ch < m_device->deviceClass()->channels()->count())
	{
	  m_values[ch].value = value;
	  m_values[ch].type = type;
	}
      else
	{
	  // Channel value is beyond this device's limits
	  return false;
	}
    }
  else
    {
      return false;
    }

  return true;
}

bool Scene::clear(unsigned short ch)
{
  if (m_device != NULL)
    {
      if (ch < m_device->deviceClass()->channels()->count())
	{
	  m_values[ch].value = 0;
	  m_values[ch].type = NoSet;
	}
      else
	{
	  // Channel value is beyond this device's limits
	  return false;
	}
    }
  else
    {
      return false;
    }

  return true;
}

SceneValue Scene::channelValue(unsigned short ch)
{
  return m_values[ch];
}

void Scene::recalculateSpeed(Feeder* f)
{
  unsigned short gap = 0;
  float delta = 0;

  unsigned short channels = 0;

  if (f->device() != NULL)
    {
      channels = f->device()->deviceClass()->channels()->count();
    }

  // Calculate delta for the rest of the channels.
  // Find out the channel needing the smallest delta (most frequent
  // updates needed) and set it to this whole scene's delta
  for (unsigned short i = 0; i < channels; i++)
    {
      if (m_values[i].type == NoSet)
	{
	  continue;
	}

      gap = abs((signed short) f->device()->dmxChannel(i)->value() - (signed short) m_values[i].value);

      if (gap != 0)
	{
	  delta = (double) f->timeSpan() / (double) gap;
	}
      else
	{
	  // This channel doesn't need update at all. If gap == 0, then both the target value
	  // and the current value are the same.
	  // continue;
	  
	  delta = f->timeSpan();
	}

      if (delta < f->delta())
	{
	  f->setDelta((unsigned long) ceil(delta));

	  if (delta < 1.0)
	    {
	      // If the next updates should come more often than
	      // 1/1024 sec, we need to increase the step value because
	      // the sequence timer provides timer tick of 1/1024sec.
	      f->setStep((unsigned long) ceil(1.0 / delta));
	    }
	  else
	    {
	      // The next step value is 1 per each 1/1024 sec
	      f->setStep(1);
	    }
	}
    }
}

Event* Scene::getEvent(Feeder* feeder)
{
  unsigned short readyCount = 0;
  unsigned short channels = 0;
  unsigned char currentValue = 0;

  channels = feeder->device()->deviceClass()->channels()->count();

  Event* event = new Event(channels);

  for (unsigned short i = 0; i < channels; i++)
    {
      if (m_values[i].type == NoSet)
	{
	  event->m_values[i].type = Ready;
	  readyCount++;
	  continue;
	}

      currentValue = feeder->device()->dmxChannel(i)->value();
      
      if (currentValue == m_values[i].value)
	{
	  event->m_values[i].type = Ready;
	  readyCount++;
	  continue;
	}
      else
	{
	  event->m_values[i].type = Normal;
	}
      
      if (m_values[i].type == Set)
	{
	  event->m_values[i].value = m_values[i].value;
	}
      else if (m_values[i].type == Fade)
	{
	  if (currentValue > m_values[i].value)
	    {
	      // Current level is above target, the new value is set toward 0
	      unsigned short nextGap = abs(m_values[i].value - currentValue);
	      unsigned short nextStep = feeder->step();
	      if (nextStep > nextGap)
		{
		  nextStep = nextGap;
		}
	      
	      event->m_values[i].value = currentValue - nextStep;
	    }
	  else if (currentValue < m_values[i].value)
	    {
	      // Current level is below target, the new value is set toward 255
	      unsigned short nextGap = abs(m_values[i].value - currentValue);
	      unsigned short nextStep = feeder->step();
	      if (nextStep > nextGap)
		{
		  nextStep = nextGap;
		}
	      
	      event->m_values[i].value = currentValue + nextStep;
	    }
	}
    }

  // Get delta from feeder and set it to next step's delta time
  event->m_delta = (unsigned long) floor(feeder->delta());
  event->m_address = feeder->device()->address();

  // If all channels are marked "ready", then this scene is ready and
  // we can unregister this scene.
  if (readyCount == channels)
    {
      event->m_type = Event::Ready;
    }

  return event;
}

void Scene::directSet(unsigned char intensity)
{
  double percent = 0;
  unsigned short int channels = 0;
  double value = 0;

  percent = ((double) intensity) / ((double) 255);
  channels = m_device->deviceClass()->channels()->count();

  for (int i = 0; i < channels; i++)
    {
      if (m_values[i].type != NoSet)
	{
	  value = m_values[i].value * percent;
	  _app->doc()->dmxChannel(m_device->address() + i)->setValue((unsigned char) value);
	}
    }
}
