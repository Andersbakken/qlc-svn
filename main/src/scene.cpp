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

#include <math.h>

extern App* _app;

Scene::Scene(unsigned short channels) : Function()
{
  m_values = (unsigned short*) calloc(channels, sizeof(short));
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

void Scene::unRegister()
{
  Function::unRegister();
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
      for (unsigned i = 0; i < deviceClass()->m_channels.count(); i++)
	{
	  t.setNum(i+1);
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
      for (unsigned i = 0; i < device()->deviceClass()->m_channels.count(); i++)
	{
	  t.setNum(i+1);
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
  int channels = 1;
  QString t;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if(s->at(0).isNumber() == true)
	{
	  int ch = s->toInt() - 1;
	  m_values = (unsigned short*) realloc(m_values, channels * sizeof(int));
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
  m_values = (unsigned short*) calloc(channels, sizeof(short));
}

void Scene::set(unsigned short ch, unsigned short value)
{
  m_values[ch - 1] = value;
}

Event* Scene::getEvent(Feeder* feeder)
{
  return getNextLevel(feeder);
}

Event* Scene::getNextLevel(Feeder* f)
{
  float gap = 0;
  float readyCount = 0;
  float delta = 0;

  short channels = 0;
  if (f->callerDevice != NULL)
    {
      channels = f->callerDevice->deviceClass()->m_channels.count();
    }

  Event* event = new Event(channels);

  /* Find out the channel needing the smallest delta (most frequent
     updates needed) and set it to this whole scene's delta */
  if (f->first == true)
    { 
      gap = abs((signed short) f->startLevels[0] - (signed short) m_values[0]);
      if (gap != 0)
	{
	  delta = f->timeSpan / gap;
	  f->delta = (unsigned long) delta;
	}
      else
	{
	  delta = f->timeSpan;
	}

      for (short i = 1; i < channels; i++)
	{
	  gap = abs((signed short) f->startLevels[i] - (signed short) m_values[i]);
	  if (gap != 0)
	    {
	      delta = f->timeSpan / gap;
	    }
	  else
	    {
	      delta = f->timeSpan;
	    }
	     
	  if (delta < f->delta)
	    {
	      f->delta = (unsigned long) delta;
	    }
	}
      f->first = false;
    }

  for (short i = 0; i < channels; i++)
    {
      if (f->startLevels[i] > m_values[i])
	{
	  // Current level is above target, the values need to be toward 0
	  if (f->currentLevels[i] <= m_values[i])
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
  	      event->m_values[i] = f->currentLevels[i] - 1;
	      f->currentLevels[i] = event->m_values[i];
	    }
	}
      else if (f->startLevels[i] < m_values[i])
	{
	  // Current level is below target, the values need to be toward 255
	  if (f->currentLevels[i] >= m_values[i])
	    {
	      event->m_values[i] = VALUE_READY;
	      readyCount++;
	    }
	  else
	    {
  	      event->m_values[i] = f->currentLevels[i] + 1;
	      f->currentLevels[i] = event->m_values[i];
	    }
	}
      else
	{
	  event->m_values[i] = VALUE_READY;
	  readyCount++;
	} 
    }

  event->m_delta = (unsigned long) floor(f->delta);

  if (readyCount == channels)
    {
      event->m_type = Event::Ready;
    }

  return event;
}


