/*
  Q Light Controller
  seqeuence.cpp
  
  Copyright (C) Heikki Junnila
  
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
#include <qpointarray.h>
#include <assert.h>

#include "app.h"
#include "doc.h"
#include "bus.h"
#include "device.h"
#include "deviceclass.h"
#include "eventbuffer.h"
#include "sequence.h"
#include "functionconsumer.h"
#include "../../libs/common/outputplugin.h"

extern App* _app;

//
// Standard constructor
//
Sequence::Sequence() : 
  Function      ( Function::Sequence ),

  m_channels    (               0 ),
  m_holdTime    (             255 ),
  m_dataMutex   (           false )
{
  setBus(KBusIDDefaultHold);
}


//
// Standard destructor
//
Sequence::~Sequence()
{
  stop();

  m_startMutex.lock();
  while (m_running)
    {
      m_startMutex.unlock();
      sched_yield();
      m_startMutex.lock();
    }
  m_startMutex.unlock();
}


//
// Copy sequence contents
//
bool Sequence::copyFrom(Sequence* sc, t_device_id toDevice)
{
  assert(sc);

  if (toDevice == KNoID || toDevice == m_deviceID)
    {
      // Same device
      m_deviceID = sc->m_deviceID;
      m_channels = sc->m_channels;
      m_name = QString(sc->m_name);
      m_busID = sc->m_busID;

      m_values.setAutoDelete(true);
      m_values.clear();
      m_values.setAutoDelete(false);

      for (unsigned int i = 0; i < sc->m_values.count(); i++)
	{
	  SceneValue* val = new SceneValue[m_channels];
	  
	  for (t_channel ch = 0; ch < m_channels; ch++)
	    {
	      val[ch].value = sc->m_values.at(i)[ch].value;
	      val[ch].type = sc->m_values.at(i)[ch].type;
	    }
	  
	  m_values.append(val);
	}
    }
  else
    {
      // Different device
      Device* device = _app->doc()->device(toDevice);
      assert(device);

      m_deviceID = toDevice;
      m_channels = device->deviceClass()->channels()->count();
      m_name = QString(sc->m_name);
      m_busID = sc->m_busID;
      
      m_values.setAutoDelete(true);
      m_values.clear();
      m_values.setAutoDelete(false);
      
      for (unsigned int i = 0; i < sc->m_values.count(); i++)
	{
	  SceneValue* val = new SceneValue[m_channels];
	  
	  if (sc->m_channels > m_channels)
	    {
	      for (t_channel ch = 0; ch < m_channels; ch++)
		{
		  val[ch].value = sc->m_values.at(i)[ch].value;
		  val[ch].type = sc->m_values.at(i)[ch].type;
		}
	    }
	  else
	    {
	      for (t_channel ch = 0; ch < sc->m_channels; ch++)
		{
		  val[ch].value = sc->m_values.at(i)[ch].value;
		  val[ch].type = sc->m_values.at(i)[ch].type;
		}

	      for (t_channel ch = sc->m_channels; ch < m_channels; ch++)
		{
		  val[ch].value = 0;
		  val[ch].type = Scene::NoSet;
		}
	    }
	  
	  m_values.append(val);
	}
    }

  return true;
}


//
// Assign this sequence to a device (or vice versa, whatever feels
// familiar to you) and allocate value arrays the size of device's
// channels
//
bool Sequence::setDevice(t_device_id id)
{
  Device* device = _app->doc()->device(id);
  assert(device);

  t_channel newChannels = device->deviceClass()->channels()->count();
  
  if (m_values.count())
    {
      for (unsigned int i = 0; i < m_values.count(); i++)
	{
	  SceneValue* oldVal = m_values.take(i);
	  SceneValue* newVal = new SceneValue[newChannels];
	  
	  if (newChannels > m_channels)
	    {
	      // Copy existing values
	      for (t_channel ch = 0; ch < m_channels; ch++)
		{
		  newVal[ch].value = oldVal[ch].value;
		  newVal[ch].type = oldVal[ch].type;
		}

	      // Fill with zeros and NoSet
	      for (t_channel ch = m_channels; ch < newChannels; ch++)
		{
		  newVal[ch].value = 0;
		  newVal[ch].type = Scene::NoSet;
		}
	    }
	  else
	    {
	      for (t_channel ch = 0; ch < newChannels; ch++)
		{
		  newVal[ch].value = oldVal[ch].value;
		  newVal[ch].type = oldVal[ch].type;
		}
	    }

	  delete oldVal;
	  m_values.insert(i, newVal);
	}
    }

  m_channels = newChannels;
  m_deviceID = id;

  return true;
}


void Sequence::constructFromPointArray(const QPointArray& array,
				       t_channel horizontalChannel,
				       t_channel verticalChannel)
{
  SceneValue* value = NULL;

  m_values.setAutoDelete(true);
  m_values.clear();
  m_values.setAutoDelete(false);

  for (unsigned int i = 0; i < array.size(); i++)
    {
      value = new SceneValue[m_channels];
      
      for (t_channel ch = 0; ch < m_channels; ch++)
	{
	  if (ch == horizontalChannel)
	    {
	      value[ch].value = array.point(i).x();
	      value[ch].type = Scene::Set;
	    }
	  else if (ch == verticalChannel)
	    {
	      value[ch].value = array.point(i).y();
	      value[ch].type = Scene::Set;	      
	    }
	  else
	    {
	      value[ch].value = 0;
	      value[ch].type = Scene::NoSet;
	    }
	}

      m_values.append(value);
    }
}

//
// Save this sequence's contents to given file
//
void Sequence::saveToFile(QFile &file)
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
  t.setNum(m_id);
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus ID
  t.setNum(m_busID);
  s = QString("Bus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Device ID
  t.setNum(m_deviceID);
  s = QString("Device = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  if (m_deviceID != KNoID)
    {
      for (unsigned int i = 0; i < m_values.count(); i++)
	{
	  SceneValue* value = m_values.at(i);
	  s = QString("Step = ");

	  for (t_channel ch = 0; ch < m_channels; ch++)
	    {
	      if (value[ch].type == Scene::NoSet)
		{
		  t = QString("XXX");
		}
	      else
		{
		  t.setNum(value[ch].value);
		}

	      s += t;
	      s += QString(" ");
	    }

	  s += QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
}

//
// Create the sequence's contents from file that has been read into list
//
void Sequence::createContents(QPtrList <QString> &list)
{
  QString t;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
        {
          s = list.prev();
          break;
        }
      else if (*s == QString("Step"))
        {
	  assert(m_channels > 0);
	  SceneValue* values = new SceneValue[m_channels];
	  QString t;

	  unsigned int i = 0;
	  int j = 0;
	  t_channel ch = 0;

	  s = list.next();

	  while (i < s->length())
	    {
	      j = s->find(QChar(' '), i, false);
	      if (j == -1)
		{
		  j = s->length();
		}

	      t = s->mid(i, j-i);
	      if (t == QString("XXX"))
		{
		  values[ch].value = 0;
		  values[ch].type = Scene::NoSet;
		}
	      else
		{
		  values[ch].value = t.toInt();
		  values[ch].type = Scene::Set;
		}
	      
	      ch++;
	      i = j + 1;
	    }

	  m_values.append(values);
        }
      else
        {
          // Unknown keyword, skip
          list.next();
        }
    }
}


//
// Bus value has changed
//
void Sequence::busValueChanged(t_bus_id id, t_bus_value value)
{
  if (id != m_busID)
    {
      return;
    }

  m_startMutex.lock();

  m_holdTime = value;

  m_startMutex.unlock();
}


//
// Allocate space for some run time stuff
//
void Sequence::arm()
{
  if (m_eventBuffer == NULL)
    m_eventBuffer = new EventBuffer(m_channels);
}


//
// Free any run-time data
//
void Sequence::disarm()
{
  if (m_eventBuffer) delete m_eventBuffer;
  m_eventBuffer = NULL;
}


//
// Get starting values
//
void Sequence::init()
{
  m_removeAfterEmpty = false;

  // Get speed
  Bus::value(m_busID, m_holdTime);
  
  // Append this function to running functions' list
  _app->functionConsumer()->cue(this);
}


//
// The main scene producer thread
//
void Sequence::run()
{
  t_bus_value holdStart = 0;
  t_bus_value timeCode = 0;

  t_channel ch = 0;
  t_value values[m_channels];

  // Initialize this scene for running
  init();

  QPtrListIterator<SceneValue> it(m_values);

  while (it.current() && !m_stopped)
    {
      for (ch = 0; ch < m_channels; ch++)
	{
	  values[ch] = it.current()[ch].value;
	}
      
      if (m_holdTime > 0)
	{
	  // Hold
	  _app->functionConsumer()->timeCode(holdStart);
	  while (!m_stopped)
	    {
	      _app->functionConsumer()->timeCode(timeCode);
	      if ((timeCode - holdStart) == m_holdTime)
		{
		  break;
		}
	      else
		{
		  sched_yield();
		}
	    }

	  ++it;
	}
    }

  // Finished
  m_removeAfterEmpty = true;
}


//
// This fuction must be called ONLY from functionconsumer AFTER
// this function is REALLY stopped.
//
void Sequence::cleanup()
{
  m_stopped = false;

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

  m_startMutex.lock();
  m_running = false;
  m_startMutex.unlock();
}
