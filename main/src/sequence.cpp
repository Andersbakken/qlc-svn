/*
  Q Light Controller
  sequence.cpp

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

extern App* _app;

//
// Standard constructor
//
Sequence::Sequence() :
  Function     ( Function::Sequence ),

  m_runOrder      (    Loop ),
  m_direction     ( Forward ),
  m_advanced      ( SetZeroEnabled ),

  m_channelData      (    NULL ),
  m_runTimeValues    (    NULL ),
  m_runTimeChannel   (       0 ),
  m_runTimeDirection ( Forward ),

  m_holdTime      (     255 ),
  m_runTimeHold   (     255 ),
  m_holdNoSetData (    NULL ),

  m_address       ( KChannelInvalid )
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

      m_steps.setAutoDelete(true);
      m_steps.clear();
      m_steps.setAutoDelete(false);

      for (unsigned int i = 0; i < sc->m_steps.count(); i++)
	{
	  SceneValue* val = new SceneValue[m_channels];

	  for (t_channel ch = 0; ch < m_channels; ch++)
	    {
	      val[ch].value = sc->m_steps.at(i)[ch].value;
	      val[ch].type = sc->m_steps.at(i)[ch].type;
	    }

	  m_steps.append(val);
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

      m_steps.setAutoDelete(true);
      m_steps.clear();
      m_steps.setAutoDelete(false);

      for (unsigned int i = 0; i < sc->m_steps.count(); i++)
	{
	  SceneValue* val = new SceneValue[m_channels];

	  if (sc->m_channels > m_channels)
	    {
	      for (t_channel ch = 0; ch < m_channels; ch++)
		{
		  val[ch].value = sc->m_steps.at(i)[ch].value;
		  val[ch].type = sc->m_steps.at(i)[ch].type;
		}
	    }
	  else
	    {
	      for (t_channel ch = 0; ch < sc->m_channels; ch++)
		{
		  val[ch].value = sc->m_steps.at(i)[ch].value;
		  val[ch].type = sc->m_steps.at(i)[ch].type;
		}

	      for (t_channel ch = sc->m_channels; ch < m_channels; ch++)
		{
		  val[ch].value = 0;
		  val[ch].type = Scene::NoSet;
		}
	    }

	  m_steps.append(val);
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

  if (m_steps.count())
    {
      for (unsigned int i = 0; i < m_steps.count(); i++)
	{
	  SceneValue* oldVal = m_steps.take(i);
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
	  m_steps.insert(i, newVal);
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

  m_steps.setAutoDelete(true);
  m_steps.clear();
  m_steps.setAutoDelete(false);

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

      m_steps.append(value);
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

  // Steps
  if (m_deviceID != KNoID)
    {
      for (unsigned int i = 0; i < m_steps.count(); i++)
	{
	  SceneValue* value = m_steps.at(i);
	  s = QString("Step = ");

	  for (t_channel ch = 0; ch < m_channels; ch++)
	    {
	      if (value[ch].type == Scene::NoSet)
		{
		  t = QString("---");
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

  // Run order
  s.sprintf("RunOrder = %d\n", (int) m_runOrder);
  file.writeBlock((const char*) s, s.length());

  // Direction
  s.sprintf("Direction = %d\n", (int) m_direction);
  file.writeBlock((const char*) s, s.length());

  // Advanced
  s.sprintf("Advanced = %d\n", (int) m_advanced);
  file.writeBlock((const char*) s, s.length());
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
	      if (t == QString("---") ||
		  t == QString("XXX"))
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

	  m_steps.append(values);
        }
      else if (*s == QString("RunOrder"))
	{
	  m_runOrder = (RunOrder) list.next()->toInt();
	}
      else if (*s == QString("Direction"))
	{
	  m_direction = (Direction) list.next()->toInt();
	}
      else if (*s == QString("Advanced"))
	{
	  m_advanced = (Advanced) list.next()->toInt();
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
  // Fetch the device adress for run time access.
  // It cannot change when functions have been armed for running
  m_address = _app->doc()->device(m_deviceID)->universeAddress();

  if (m_channelData == NULL)
    m_channelData = new t_buffer_data[m_channels];

  if (m_holdNoSetData == NULL)
    {
      m_holdNoSetData = new t_buffer_data[m_channels];
      for (t_channel ch = 0; ch < m_channels; ch++)
	{
	  m_holdNoSetData[ch] = KChannelInvalid << 8;
	  m_holdNoSetData[ch] |= 0;
	}
    }

  if (m_eventBuffer == NULL)
    m_eventBuffer = new EventBuffer(m_channels, KFrequency >> 1);
}


//
// Free any run-time data
//
void Sequence::disarm()
{
  // Just a nuisance to prevent using this at non-run-time :)
  m_address = KChannelInvalid;

  if (m_channelData) delete [] m_channelData;
  m_channelData = NULL;

  if (m_holdNoSetData) delete [] m_holdNoSetData;
  m_holdNoSetData = NULL;

  if (m_eventBuffer) delete m_eventBuffer;
  m_eventBuffer = NULL;
}


//
// Get starting values
//
void Sequence::init()
{
  m_removeAfterEmpty = false;

  m_stopped = false;

  // Get speed
  Bus::value(m_busID, m_holdTime);

  // Append this function to running functions' list
  _app->functionConsumer()->cue(this);
}


//
// The main scene producer thread. Nothing should be allocated here
// (not even local variables) to keep the function as fast as possible.
//
void Sequence::run()
{
  // Initialize this sequence for running
  init();

  m_runTimeDirection = m_direction;

  while (!m_stopped)
    {
      if (m_runTimeDirection == Forward)
	{
	  for (m_runTimeValues = m_steps.first();
	       m_runTimeValues != NULL && !m_stopped;
	       m_runTimeValues = m_steps.next())
	    {
	      for (m_runTimeChannel = 0;
		   m_runTimeChannel < m_channels;
		   m_runTimeChannel++)
		{
		  if (m_runTimeValues[m_runTimeChannel].type == Scene::NoSet)
		    {
		      // Set the absolut adress
		      m_channelData[m_runTimeChannel] = KChannelInvalid << 8;
		      m_channelData[m_runTimeChannel] |= 0;
		    }
		  else
		    {
		      // Set the absolut adress
		      m_channelData[m_runTimeChannel] =
			(m_address + m_runTimeChannel) << 8;

		      // Set a normal value
		      m_channelData[m_runTimeChannel] |=
		        static_cast<t_buffer_data>
		        (m_runTimeValues[m_runTimeChannel].value);
		    }
		}

	      m_eventBuffer->put(m_channelData);
	      hold();
	    }
	}
      else
	{
	  for (m_runTimeValues = m_steps.last();
	       m_runTimeValues != NULL && !m_stopped;
	       m_runTimeValues = m_steps.prev())
	    {
	      for (m_runTimeChannel = 0;
		   m_runTimeChannel < m_channels;
		   m_runTimeChannel++)
		{
		  if (m_runTimeValues[m_runTimeChannel].type == Scene::NoSet)
		    {
		      // Set the absolut adress
		      m_channelData[m_runTimeChannel] = KChannelInvalid << 8;

		      // Set invalid value fur such channels that don't
		      // have a valid value
		      m_channelData[m_runTimeChannel] |= 0;
		    }
		  else
		    {
		      // Set the absolut adress
		      m_channelData[m_runTimeChannel] =
			(m_address + m_runTimeChannel) << 8;

		      // Set a normal value
		      m_channelData[m_runTimeChannel] |=
		        static_cast<t_buffer_data>
		        (m_runTimeValues[m_runTimeChannel].value);
		    }
		}

	      m_eventBuffer->put(m_channelData);
	      hold();
	    }
	}

      //
      // Check what should be done after a round
      //
      if (m_runOrder == SingleShot)
	{
	  // That's it
//	  m_stopped = true;
	  break;
	}
      else if (m_runOrder == Loop)
	{
	  // Just continue as before
	  continue;
	}
      else // if (m_runOrder == PingPong)
	{
	  // Change run order
	  if (m_runTimeDirection == Forward)
	    {
	      m_runTimeDirection = Backward;
	    }
	  else
	    {
	      m_runTimeDirection = Forward;
	    }
	}
    }

  if (m_stopped && m_advanced == SetZeroEnabled)
    {
      // Set the last step as reference
      m_runTimeValues = m_steps.last();

      // Loop over all touched channels
      for (m_runTimeChannel = 0;
	   m_runTimeChannel < m_channels;
	   m_runTimeChannel++)
	{
	  // I think, that is not really needed ??
	  if (m_runTimeValues[m_runTimeChannel].type == Scene::NoSet)
	    {
	      // Set absolut adress
	      m_channelData[m_runTimeChannel] = KChannelInvalid << 8;
	      //
	      m_channelData[m_runTimeChannel] |= 0;
	    }
	  else
	    {
	      m_channelData[m_runTimeChannel] =
			(m_address + m_runTimeChannel) << 8;
	      // Set a Zero value on touched channels
	      m_channelData[m_runTimeChannel] |= 0;
	    }
	}
      // Stops immediately the running sequence
      m_eventBuffer->purge();
      // Set the zero values after stopping
      m_eventBuffer->put(m_channelData);
    }
  else if (m_stopped && m_advanced == SetZeroDisabled)
    {
      m_eventBuffer->purge();
    }
  // Finished
  m_removeAfterEmpty = true;
}


//
// Hold
//
void Sequence::hold()
{
  if (m_holdTime > 0)
    {
      for (m_runTimeHold = m_holdTime;
	   m_runTimeHold > 0 && !m_stopped;
	   m_runTimeHold--)
	{
	  m_eventBuffer->put(m_holdNoSetData);
	}
    }
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
