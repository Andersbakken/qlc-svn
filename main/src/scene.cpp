/*
  Q Light Controller
  scene.cpp
  
  Copyright (C) 2004 Heikki Junnila
  
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

#include <qfile.h>
#include <qptrlist.h>
#include <time.h>
#include <qapplication.h>

#include "app.h"
#include "doc.h"
#include "device.h"
#include "deviceclass.h"
#include "eventbuffer.h"
#include "scene.h"
#include "functionconsumer.h"
#include "../../libs/common/outputplugin.h"

extern App* _app;

//
// Standard constructor
//
Scene::Scene(t_function_id id) : Function(id)
{
  m_type = Function::Scene;
  m_values = NULL;
  m_runTimeData = NULL;
  m_channelData = NULL;
  m_channels = 0;
  m_timeSpan = 256;
  m_parentFunction = NULL;
}


//
// Used mainly by Advanced Scene Editor to work on a copy of the actual
// function.
//
void Scene::copyFrom(Scene* sc)
{
  ASSERT(sc);

  m_type = sc->type();
  m_id = sc->id();
  m_name = sc->name();
  m_device = sc->m_device;
  m_channels = sc->m_channels;

  if (m_values) delete[] m_values;
  m_values = new SceneValue[m_channels];
  
  for (t_channel i = 0; i < m_channels; i++)
    {
      m_values[i].value = sc->m_values[i].value;
      m_values[i].type = sc->m_values[i].type;
    }
}


//
// Assign this scene to a device (or vice versa, whatever feels
// familiar to you) and allocate a value array the size of device's
// channels
//
void Scene::setDevice(Device* device)
{
  ASSERT(device);

  if (m_values)
    {
      t_channel newChannels = device->deviceClass()->channels()->count();
      
      // Store old values temporarily
      SceneValue tempValues[m_channels];
      memcpy(&tempValues, m_values, m_channels * sizeof(SceneValueType));
      
      // Delete old values
      delete [] m_values;
      
      // Allocate space for new values
      m_values = new SceneValue[newChannels];
      
      // Copy old values
      if (newChannels < m_channels)
        {
          // New device has less channels than previous one, copy only
          // those that fit in
          memcpy(m_values, &tempValues, newChannels * sizeof(SceneValue));
          m_channels = newChannels;
          m_device = device;
        }
      else if (newChannels > m_channels)
        {
          // New device has more channels than previous one, copy old
          // values and fill the rest with 0 and NoSet.
          memcpy(m_values, &tempValues, newChannels * sizeof(SceneValue));
          for (t_channel i = m_channels; i < newChannels; i++)
            {
	      m_values[i].value = 0;
	      m_values[i].type = NoSet;
            }
	  
          m_channels = newChannels;
          m_device = device;
        }
      else
        {
          // Channel count is identical, just copy the values.
          memcpy(m_values, &tempValues, m_channels * sizeof(SceneValue));
          m_device = device;
        }
    }
  else
    {
      // Get channel count
      m_channels = device->deviceClass()->channels()->count();
      
      // Set device
      m_device = device;
      
      // Allocate space for new values
      m_values = new SceneValue[m_channels];
      
      // Set all channel values to 0 and fade
      for (t_channel i = 0; i < m_channels; i++)
        {
          m_values[i].value = 0;
          m_values[i].type = Fade;
        }
    }
}


//
// Standard destructor
//
Scene::~Scene()
{
  stop();
  while (m_running) sched_yield();

  if (m_values) delete[] m_values;
}


//
// Return one channel's value type as string
//
QString Scene::valueTypeString(t_channel ch)
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


//
// Save this scene's contents to given file
//
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
      for (t_channel i = 0;
           i < (t_channel) device()->deviceClass()->channels()->count(); i++)
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
      // For global scenes the device is zero
      s = QString("Device = 0") + QString("\n");
      file.writeBlock((const char*) s, s.length());

      // TODO: write global scene data
    }
}

//
// Create the scene's contents from file that has been read into list
//
void Scene::createContents(QPtrList <QString> &list)
{
  QString t;

  t_value ch = 0;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
        {
          s = list.prev();
          break;
        }
      else if (s->at(0).isNumber() == true)
        {
          ch = static_cast<t_value> (s->toInt());
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


//
// Set one channel value for this scene
//
bool Scene::set(t_channel ch, t_value value, SceneValueType type)
{
  if (m_device != NULL)
    {
      if (ch < m_channels)
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


//
// Return one channel value & type from this scene
//
SceneValue Scene::channelValue(t_channel ch)
{
  return m_values[ch];
}


//
// Explicitly stop this function
//
void Scene::stop()
{
  m_stopMutex.lock();
  m_stopped = true;
  m_stopMutex.unlock();
}


//
// Calculate run time values for producer thread (below)
//
void Scene::init()
{
  ASSERT(m_runTimeData == NULL);
  m_runTimeData = new RunTimeData[m_channels];

  ASSERT(m_channelData == NULL);
  m_channelData = new t_value[m_channels];

  ASSERT(m_eventBuffer == NULL);
  m_eventBuffer = new EventBuffer(m_channels);

  // Current values
  _app->doc()->outputPlugin()->readRange(m_device->address(),
					 m_channelData, m_channels);

  for (t_channel i = 0; i < m_channels; i++)
    {
      m_runTimeData[i].current = static_cast<float> (m_channelData[i]);

      // Step increment
      m_runTimeData[i].increment = 
	(static_cast<float>(m_values[i].value) - 
	 m_runTimeData[i].current) / (float) m_timeSpan;

      qDebug("%d: %d -> %d incs %f", i, m_channelData[i],
	     m_values[i].value, m_runTimeData[i].increment);
    }
}


//
// The main scene producer thread
//
void Scene::run()
{
  time_t time = 0;
  unsigned short ch = 0;

  // Calculate starting values
  init();

  // Append this function to running functions list
  _app->functionConsumer()->cue(this);

  m_stopMutex.lock();
  for (time = 0; time < m_timeSpan && !m_stopped; time++)
    {
      m_stopMutex.unlock();

      for (ch = 0; ch < m_channels; ch++)
	{
	  m_runTimeData[ch].current += m_runTimeData[ch].increment;
	  m_channelData[ch] =
	    static_cast<t_value> (m_runTimeData[ch].current);
	}

      // Put data to buffer
      m_eventBuffer->put(m_channelData);
      //t_value* ev = m_channelData;
      //qDebug("%d %d %d %d %d %d", ev[0], ev[1], ev[2], 
      //       ev[3], ev[4], ev[5]);

      m_stopMutex.lock();
    }

  m_stopMutex.unlock();

  // No more items produced -> this scene can be removed from
  // the list after the buffer is empty.
  m_removeAfterEmpty = true;
}


//
// Free run time allocations
// This fuction must be called ONLY from functionconsumer AFTER
// this function is REALLY stopped.
//
void Scene::freeRunTimeData()
{
  qDebug("Scene::freeRunTimeData");

  delete [] m_runTimeData;
  m_runTimeData = NULL;

  delete [] m_channelData;
  m_channelData = NULL;

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

  m_stopMutex.lock();
  m_stopped = true;
  m_stopMutex.unlock();

  m_startMutex.lock();
  m_running = false;
  m_startMutex.unlock();
}
