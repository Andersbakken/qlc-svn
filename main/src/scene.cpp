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

#include <math.h>
#include <stdlib.h>
#include <iostream.h>

extern App* _app;

Scene::Scene(unsigned short channels) : Function()
{
  m_values = (SceneValue*) calloc(channels, sizeof(SceneValue));
  m_type = Function::Scene;
}

Scene::Scene()
{
  m_values = NULL;

  m_type = Function::Scene;
}

Scene::~Scene()
{
  if (m_values != NULL)
    {
      free(m_values);
      m_values = NULL;
    }
}

void Scene::unRegisterFunction()
{
  Function::unRegisterFunction();
}

void Scene::registerFunction(Feeder* feeder)
{
  recalculateSpeed(feeder);
  Function::registerFunction(feeder);
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

  // Device class, device name or "Global"
  if (deviceClass() != NULL)
    {
      // Write only the data for device class scenes
      for (unsigned short i = 0; i < deviceClass()->channels(); i++)
	{
	  t.setNum(i);
	  s = t + QString(" = ");
	  t.setNum(m_values[i].value);
	  s += t + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
  else if (device() != NULL)
    {
      // For device scenes (that are saved in the workspace file)
      // write also the device name that this scene is attached to
      s = QString("Device = ") + device()->name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // Data
      for (unsigned short i = 0; i < device()->deviceClass()->m_channels.count(); i++)
	{
	  t.setNum(i);
	  s = t + QString(" = ");
	  t.setNum(m_values[i].value);
	  s += t + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
  else
    {
      // For global scenes the device name is "Global"
      s = QString("Device = Global") + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
}

void Scene::createContents(QList<QString> &list)
{
  unsigned short channels = 1;
  QString t;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (s->at(0).isNumber() == true)
	{
	  unsigned char ch = (unsigned char) s->toInt();
	  m_values = (SceneValue*) realloc(m_values, channels * sizeof(SceneValue));
	  t = *(list.next());
	  m_values[ch].value = t.toInt();
	  channels++;
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}

bool Scene::allocate(unsigned short channels)
{
  if (m_device != NULL)
    {
      if (channels < m_device->deviceClass()->channels())
	{
	  m_values = (SceneValue*) calloc(channels, sizeof(SceneValue));
	}
      else
	{
	  return false;
	}
    }
  else if (m_deviceClass != NULL)
    {
      if (channels < m_deviceClass->channels())
	{
	  m_values = (SceneValue*) calloc(channels, sizeof(SceneValue));
	}
      else
	{
	  return false;
	}
    }

  return true;
}

bool Scene::set(unsigned short ch, unsigned char value)
{
  if (m_device != NULL)
    {
      if (ch < m_device->deviceClass()->channels())
	{
	  m_values[ch].value = value;
	}
      else
	{
	  // Channel value is beyond this device's limits
	  return false;
	}
    }
  else if (m_deviceClass != NULL)
    {
      if (ch < deviceClass()->channels())
	{
	  m_values[ch].value = value;
	}
      else
	{
	  // Channel value is beyond this deviceclass' limits
	  return false;
	}
    }
  else
    {
      return false;
    }

  return true;
}

void Scene::recalculateSpeed(Feeder* f)
{
  unsigned short gap = 0;
  float delta = 0;

  unsigned short channels = 0;

  if (f->device() != NULL)
    {
      channels = f->device()->deviceClass()->channels();
    }

  // Calculate delta for the rest of the channels.
  // Find out the channel needing the smallest delta (most frequent
  // updates needed) and set it to this whole scene's delta
  for (unsigned short i = 0; i < channels; i++)
    {
      gap = abs((signed short) f->startLevel(i) - (signed short) m_values[i].value);

      if (gap != 0)
	{
	  delta = (double) f->timeSpan() / (double) gap;
	}
      else
	{
	  delta = f->timeSpan();
	}

      if (delta < f->delta())
	{
	  f->setDelta((unsigned long) ceil(delta));

	  if (delta < 1.0)
	    {
	      // If the next updates should come more often than
	      // 1/1000 sec, we need to increase the step value because
	      // the sequence timer provides timer tick of 1/1000sec.
	      f->setStep((unsigned long) ceil(1.0 / delta));
	    }
	  else
	    {
	      // The next step value is 1 per each 1/1000 sec
	      f->setStep(1);
	    }
	}
    }
}

Event* Scene::getEvent(Feeder* feeder)
{
  unsigned short readyCount = 0;
  unsigned short channels = 0;
  
  channels = feeder->device()->deviceClass()->channels();

  Event* event = new Event(channels);

  for (unsigned short i = 0; i < channels; i++)
    {
      if (feeder->device()->getChannelValue(i) == m_values[i].value)
	{
	  event->m_values[i].type = Ready;
	  readyCount++;
	  continue;
	}
      else
	{
	  event->m_values[i].type = Normal;
	}

      if (feeder->startLevel(i) > m_values[i].value)
	{
	  // Current level is above target, the new value is set toward 0
	  unsigned short nextGap = abs(m_values[i].value - feeder->device()->getChannelValue(i));
	  unsigned short nextStep = feeder->step();
	  if (nextStep > nextGap)
	    {
	      nextStep = nextGap;
	    }

	  event->m_values[i].value = feeder->device()->getChannelValue(i) - nextStep;
	}
      else if (feeder->startLevel(i) < m_values[i].value)
	{
	  // Current level is below target, the new value is set toward 255
	  unsigned short nextGap = abs(m_values[i].value - feeder->device()->getChannelValue(i));
	  unsigned short nextStep = feeder->step();
	  if (nextStep > nextGap)
	    {
	      nextStep = nextGap;
	    }

	  event->m_values[i].value = feeder->device()->getChannelValue(i) + nextStep;
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


unsigned char Scene::getChannelValue(unsigned short ch)
{
  return m_values[ch].value;
}

/*
This left here in purpose, in case I break something... :)

Event* Scene::getEvent(Feeder* feeder)
{
  unsigned short readyCount = 0;
  unsigned short channels = 0;
  
  if (feeder->device() != NULL)
    {
      channels = feeder->device()->deviceClass()->channels();
    }

  Event* event = new Event(channels);

  for (unsigned short i = 0; i < channels; i++)
    {
      if (feeder->startLevel(i) > m_values[i].value)
	{ // Current level is above target, the new value is set toward 0
	  if (feeder->device()->getChannelValue(i) <= m_values[i].value)
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
	      // If the next step value is bigger than the next gap we need
	      // to set the next step value to the gap value so that
	      // we won't write too big values.
	      // Example: current channel value is 254 and the scene's target
	      // value is 255. If the step value is 2, the next value would
	      // be 256 which (since we are using unsigned chars) would mean
	      // actually 0 (with signed chars -127) so this would loop
	      // endlessly without ever reaching the target value
	      int nextGap = abs(m_values[i].value - feeder->device()->getChannelValue(i));
	      int nextStep = feeder->step();
	      if (nextStep > nextGap)
		{
		  nextStep = nextGap;
		}
	      event->m_values[i] = feeder->device()->getChannelValue(i) - nextStep;
	    }
	}
      // Current level is below target, the new value is set toward 255
      else if (feeder->startLevel(i) < m_values[i].value)
	{
	  if (feeder->device()->getChannelValue(i) >= m_values[i].value)
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
	      // If the next step value is bigger than the next gap we need
	      // to set the next step value to the gap value so that
	      // we won't write too big values.
	      // Example: current channel value is 254 and the scene's target
	      // value is 255. If the step value is 2, the next value would
	      // be 256 which (since we are using unsigned chars) would mean
	      // actually 0 (with signed chars -127) so this would loop
	      // endlessly without ever reaching the target value
	      int nextGap = abs(m_values[i].value - feeder->device()->getChannelValue(i));
	      int nextStep = feeder->step();
  	      if (nextStep > nextGap)
		{
		  nextStep = nextGap;
		}
	      event->m_values[i] = feeder->device()->getChannelValue(i) + nextStep;
	    }
	}
      else
	{
	  event->m_values[i] = VALUE_READY;
	  readyCount++;
	}
    }

  event->m_delta = (unsigned long) floor(feeder->delta());

  if (readyCount == channels)
    {
      event->m_type = Event::Ready;
    }

  return event;
}
*/
