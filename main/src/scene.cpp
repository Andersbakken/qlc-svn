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

extern App* _app;

Scene::Scene(unsigned short channels) : Function()
{
  m_values = (unsigned char*) calloc(channels, sizeof(unsigned char));
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
	  t.setNum(m_values[i]);
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
	  t.setNum(m_values[i]);
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
	  m_values = (unsigned char*) realloc(m_values, channels * sizeof(unsigned char));
	  t = *(list.next());
	  m_values[ch] = t.toInt();
	  channels++;
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}

void Scene::allocate(unsigned short channels)
{
  m_values = (unsigned char*) calloc(channels, sizeof(unsigned char));
}

void Scene::set(unsigned short ch, unsigned char value)
{
  m_values[ch] = value;
}

void Scene::recalculateSpeed(Feeder* f)
{
  unsigned short gap = 0;
  unsigned long delta = 0;

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
      gap = abs((signed short) f->startLevel(i) - (signed short) m_values[i]);

      if (gap != 0)
	{
	  delta = (unsigned long) ((double) f->timeSpan() / (double) gap);
	}
      else
	{
	  delta = f->timeSpan();
	}

      if (delta < f->delta())
	{
	  f->setDelta((unsigned long) delta);
	}
    }
}

Event* Scene::getEvent(Feeder* feeder)
{
  return getNextLevel(feeder);
}

Event* Scene::getNextLevel(Feeder* f)
{
  unsigned short readyCount = 0;
  unsigned short channels = 0;
  
  if (f->device() != NULL)
    {
      channels = f->device()->deviceClass()->channels();
    }

  Event* event = new Event(channels);

  for (unsigned short i = 0; i < channels; i++)
    {
      if (f->startLevel(i) > m_values[i])
	{
	  // Current level is above target, the new value is set toward 0
	  if (f->device()->getChannelValue(i) <= m_values[i])
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
  	      event->m_values[i] = f->device()->getChannelValue(i) - 1;
	    }
	}
      else if (f->startLevel(i) < m_values[i])
	{
	  // Current level is below target, the new value is set toward 255
	  if (f->device()->getChannelValue(i) >= m_values[i])
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
  	      event->m_values[i] = f->device()->getChannelValue(i) + 1;
	    }
	}
      else
	{
	  event->m_values[i] = VALUE_READY;
	  readyCount++;
	}
    }

  event->m_delta = (unsigned long) floor(f->delta());

  if (readyCount == channels)
    {
      event->m_type = Event::Ready;
    }

  return event;
}
