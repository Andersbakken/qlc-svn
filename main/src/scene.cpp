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
#include <assert.h>

#include "app.h"
#include "doc.h"
#include "bus.h"
#include "device.h"
#include "deviceclass.h"
#include "eventbuffer.h"
#include "scene.h"
#include "functionconsumer.h"
#include "common/filehandler.h"

extern App* _app;

//
// Standard constructor
//
Scene::Scene() : 
	Function      ( Function::Scene ),
	m_values      (            NULL ),
	m_timeSpan    (             255 ),
	m_elapsedTime (               0 ),
	m_runTimeData (            NULL ),
	m_channelData (            NULL ),
	m_dataMutex   (           false ),
	m_address     ( KChannelInvalid )
{
	setBus(KBusIDDefaultFade);
}


//
// Copy sequence contents
//
void Scene::copyFrom(Scene* sc, t_device_id toDevice)
{
	assert(sc);
	
	Function::setName(sc->name());
	Function::setBus(sc->busID());
	
	setDevice(toDevice);
	
	if (m_values) delete [] m_values;
	m_values = new SceneValue[m_channels];
	
	for (t_channel ch = 0; ch < m_channels; ch++)
	{
		m_values[ch].value = sc->m_values[ch].value;
		m_values[ch].type = sc->m_values[ch].type;
	}
}


//
// Assign this scene to a device
//
bool Scene::setDevice(t_device_id id)
{
	Device* device = _app->doc()->device(id);
	if (!device)
	{
		return false;
	}
	
	t_channel newChannels = device->deviceClass()->channels()->count();
	
	if (m_channels == 0)
	{
		m_channels = newChannels;
		
		m_values = new SceneValue[m_channels];
		
		for (t_channel i = 0; i < m_channels; i++)
		{
			m_values[i].value = 0;
			m_values[i].type = Fade;
		}
	}
	else
	{
		assert(m_channels == newChannels);
	}
	
	m_deviceID = id;
	
	_app->doc()->emitFunctionChanged(m_id);
	
	return true;
}


//
// Standard destructor
//
Scene::~Scene()
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
	
	if (m_values) delete [] m_values;
}

Scene::ValueType Scene::valueType(t_channel ch)
{
	assert(ch < m_channels);
	return m_values[ch].type;
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
		// Data
		for (t_channel i = 0; i < m_channels; i++)
		{
			t.setNum(i);
			s = t + QString(" = ");
			t.setNum(m_values[i].value);
			s += t + QString("\n");
			file.writeBlock((const char*) s, s.length());
			
			s = QString("ValueType = ") + valueTypeString(i) + 
				QString("\n");
			file.writeBlock((const char*) s, s.length());
		}
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


// Save this function to an XML document
void Scene::saveXML(QDomDocument* doc)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	
	assert(doc);

	/* Function tag */
	root = doc->createElement(KXMLFunctionNode);
	doc->appendChild(root);

	root.setAttribute(KXMLFunctionID, id());
	root.setAttribute(KXMLFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLFunctionName, name());
	root.setAttribute(KXMLFunctionDevice, device());

	/* Speed bus */
	tag = doc->createElement(KXMLBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLBusRole, KXMLBusFade);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Scene contents */
	for (t_channel i = 0; i < m_channels; i++)
	{
		/* Value tag */
		tag = doc->createElement(KXMLFunctionValue);
		root.appendChild(tag);
		
		/* Value type & channel */
		tag.setAttribute(KXMLFunctionChannel, i);
		tag.setAttribute(KXMLFunctionType, valueTypeString(i));

		/* Value contents */
		str.setNum(m_values[i].value);
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}
}


//
// Set one channel value for this scene
//
bool Scene::set(t_channel ch, t_value value, ValueType type)
{
	m_startMutex.lock();
	if (m_running)
	{
		m_startMutex.unlock();
		return false;
	}
	else if (ch < m_channels)
	{
		m_values[ch].value = value;
		m_values[ch].type = type;
		
		m_startMutex.unlock();
		return true;
	}
	else
	{
		m_startMutex.unlock();
		return false;
	}
}


//
// Return one channel value & type from this scene
//
SceneValue Scene::channelValue(t_channel ch)
{
	return m_values[ch];
}


//
// Bus value has changed
//
void Scene::busValueChanged(t_bus_id id, t_bus_value value)
{
	if (id != m_busID)
	{
		return;
	}
	
	m_startMutex.lock();
	
	if (m_running)
	{
		speedChange(value);
	}
	else
	{
		m_timeSpan = value;
	}
	
	m_startMutex.unlock();
}


//
// Calculate new values
//
void Scene::speedChange(t_bus_value newTimeSpan)
{
	m_timeSpan = newTimeSpan;
	if (m_timeSpan == 0)
	{
		m_timeSpan = static_cast<t_bus_value>
			(1.0 / static_cast<float> (KFrequency));
	}
}


//
// Allocate space for some run time stuff
//
void Scene::arm()
{
	// Fetch the device address for run time access.
	// It cannot change when functions have been armed for running
	m_address = _app->doc()->device(m_deviceID)->universeAddress();
	
	if (m_runTimeData == NULL)
		m_runTimeData = new RunTimeData[m_channels];
	
	if (m_channelData == NULL)
		m_channelData = new t_buffer_data[m_channels];
	
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(m_channels, KFrequency >> 1);
}


//
// Free any run-time data
//
void Scene::disarm()
{
	// Just a nuisance to prevent using this at non-run-time :)
	m_address = KChannelInvalid;
	
	if (m_runTimeData) delete [] m_runTimeData;
	m_runTimeData = NULL;
	
	if (m_channelData) delete [] m_channelData;
	m_channelData = NULL;
	
	if (m_eventBuffer) delete m_eventBuffer;
	m_eventBuffer = NULL;
}


//
// Get starting values
//
void Scene::init()
{
	m_removeAfterEmpty = false;
	m_stopped = false;
	
	for (t_channel i = 0; i < m_channels; i++)
	{
		m_runTimeData[i].current =
			m_runTimeData[i].start =
			static_cast<float> (_app->value(m_address + i));
		
		m_runTimeData[i].target = 
			static_cast<float> (m_values[i].value);
		
		m_runTimeData[i].ready = false;
	}
	
	// No time has yet passed for this scene.
	m_elapsedTime = 0;
	
	// Get speed
	Bus::value(m_busID, m_timeSpan);
	
	// Set speed
	speedChange(m_timeSpan);

	// Append this function to running functions' list
	_app->functionConsumer()->cue(this);
}


//
// The main scene producer thread
//
void Scene::run()
{
	t_channel ch = 0;
	t_channel ready = 0;
	
	// Initialize this scene for running
	init();
	
	// Check if this scene needs to play
	for (t_channel i = 0; i < m_channels; i++)
	{
		if (m_values[i].type == Set ||
		    m_values[i].type == Fade)
		{
			if (m_values[i].value == (int) m_runTimeData[i].current)
			{
				ready++;
			}
		}
		else
		{
			// NoSet values are treated as ready
			ready++;
		}
	}
	
	// This scene does not need to be played because all target
	// values are already where they are supposed to be.
	if (ready == m_channels)
	{
		m_stopped = true;
	}
	
	for (m_elapsedTime = 0; m_elapsedTime < m_timeSpan && !m_stopped;
	     m_elapsedTime++)
	{
		for (ch = 0; ch < m_channels; ch++)
		{
			if (m_values[ch].type == NoSet || 
			    m_runTimeData[ch].ready)
			{
				m_channelData[ch] = KChannelInvalid << 8;
				m_channelData[ch] |= 0;
				
				// This channel contains a value that is not
				// supposed to be written (anymore, in case of
				// a ready value, which comes from "set" type)
				continue;
			}
			else if (m_values[ch].type == Set)
			{
				// Just set the target value
				m_channelData[ch] = (m_address + ch) << 8;
				m_channelData[ch] |= static_cast<t_buffer_data>
					(m_values[ch].value);
				
				// ...and don't touch this channel anymore
				m_runTimeData[ch].ready = true;
			}
			else if (m_values[ch].type == Fade)
			{
				// Calculate the current value based on what
				// it should be after m_elapsedTime to be
				// ready at m_timeSpan
				m_runTimeData[ch].current = 
					m_runTimeData[ch].start
					+ (m_runTimeData[ch].target 
					   - m_runTimeData[ch].start)
					* ((float)m_elapsedTime / m_timeSpan);
				
				m_channelData[ch] = (m_address + ch) << 8;
				
				m_channelData[ch] |= static_cast<t_buffer_data>
					(m_runTimeData[ch].current);
			}
		}
		
		m_eventBuffer->put(m_channelData);
	}
	
	// Write the last step exactly to target because timespan might have
	// been set to a smaller amount than what has elapsed. Also, because
	// floats are NEVER exact numbers, it might be that we never quite reach
	// the target within the given timespan (in case values don't add up).
	for (ch = 0; ch < m_channels; ch++)
	{
		if (m_values[ch].type == NoSet || m_runTimeData[ch].ready)
		{
			m_channelData[ch] = KChannelInvalid << 8;
			m_channelData[ch] |= 0;
		}
		else
		{
			// Just set the target value
			m_channelData[ch] = (m_address + ch) << 8;
			m_channelData[ch] |= 
				static_cast<t_buffer_data> (m_values[ch].value);
			
			// ...and don't touch this channel anymore
			m_runTimeData[ch].ready = true;
		}
	}
	
	if (!m_stopped)
	{
		m_eventBuffer->put(m_channelData);
	}
	else
	{
		//
		// This scene was stopped. Clear buffer so that this function
		// can finish as quickly as possible
		//
		m_eventBuffer->purge();
	}
	
	// No more items produced -> this scene can be removed from
	// the list after the buffer is empty.
	m_removeAfterEmpty = true;
}


//
// This fuction must be called ONLY from functionconsumer AFTER
// this function is REALLY stopped.
//
void Scene::cleanup()
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
