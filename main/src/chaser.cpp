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

#include <stdlib.h>
#include <unistd.h>
#include <qfile.h>
#include <sched.h>
#include <qapplication.h>
#include <assert.h>

#include "common/deviceclass.h"
#include "common/filehandler.h"
#include "chaser.h"
#include "doc.h"
#include "app.h"
#include "bus.h"
#include "scene.h"
#include "device.h"
#include "functionconsumer.h"
#include "eventbuffer.h"

extern App* _app;

//
// Standard constructor
//
Chaser::Chaser() : 
	Function(Function::Chaser),
  
	m_runOrder     (   Loop ),
	m_direction    ( Forward ),
	m_childRunning (  false ),

	m_holdTime     (       0 ),
	m_holdStart    (       0 ),
	m_timeCode     (       0 ),

	m_runTimeDirection ( Forward ),
	m_runTimePosition  (      0 )
{
	setBus(KBusIDDefaultHold);
}


//
// Copy the contents of another chaser into this
// If append == true, existing contents will not be cleared; new steps
// will appear after existing steps
//
void Chaser::copyFrom(Chaser* ch, bool append)
{
	assert(ch);

	Function::setName(ch->name());
	setDirection(ch->direction());
	setRunOrder(ch->runOrder());

	if (append == false)
	{
		m_steps.clear();
	}

	QValueList <t_function_id>::iterator it;
	for (it = ch->m_steps.begin(); it != ch->m_steps.end(); ++it)
	{
		m_steps.append(*it);
	}
}


//
// Destructor
//
Chaser::~Chaser()
{
	stop();
	m_steps.clear();
}


//
// Save this chaser's contents into the given file
//
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
	s = QString("Type = ") + Function::typeToString(m_type) + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// ID
	s.sprintf("ID = %d\n", m_id);
	file.writeBlock((const char*) s, s.length());

	// Bus ID
	t.setNum(m_busID);
	s = QString("Bus = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());

	// Device
	s = QString("Device = 0\n");
	file.writeBlock((const char*) s, s.length());

	// Steps
	s = QString("# Step entries") + QString("\n");
	file.writeBlock((const char*) s, s.length());

	QValueList <t_function_id>::iterator it;
	for (it = m_steps.begin(); it != m_steps.end(); ++it)
	{
		s.sprintf("Function = %d\n", *it);
		file.writeBlock((const char*) s, s.length());
	}

	// Direction
	s.sprintf("Direction = %d\n", (int) m_direction);
	file.writeBlock((const char*) s, s.length());

	// Run order
	s.sprintf("RunOrder = %d\n", (int) m_runOrder);
	file.writeBlock((const char*) s, s.length());
}


//
// Create this chaser's contents from a file that has been read into list
//
void Chaser::createContents(QPtrList <QString> &list)
{
	t_function_id fid = KNoID;
  
	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else if (*s == QString("Function"))
		{
			fid = list.next()->toInt();
			addStep(fid);
			fid = KNoID;
		}
		else if (*s == QString("Direction"))
		{
			m_direction = (Direction) list.next()->toInt();
		}
		else if (*s == QString("RunOrder"))
		{
			m_runOrder = (RunOrder) list.next()->toInt();
		}
		else
		{
			// Unknown keyword
			list.next();
		}
	}
}

// Save this function to an XML document
void Chaser::saveXML(QDomDocument* doc)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	int i = 0;
	
	assert(doc);

	/* Function tag */
	root = doc->createElement(KXMLFunctionNode);
	doc->appendChild(root);

	root.setAttribute(KXMLFunctionID, id());
	root.setAttribute(KXMLFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLFunctionName, name());

	/* Speed bus */
	tag = doc->createElement(KXMLBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLBusRole, KXMLBusHold);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Direction */
	tag = doc->createElement(KXMLFunctionDirection);
	root.appendChild(tag);
	text = doc->createTextNode(Function::directionToString(m_direction));
	tag.appendChild(text);

	/* Run order */
	tag = doc->createElement(KXMLFunctionRunOrder);
	root.appendChild(tag);
	text = doc->createTextNode(Function::runOrderToString(m_runOrder));
	tag.appendChild(text);

	/* Steps */
	QValueList <t_function_id>::iterator it;
	for (it = m_steps.begin(); it != m_steps.end(); ++it)
	{
		/* Step tag */
		tag = doc->createElement(KXMLFunctionStep);
		root.appendChild(tag);

		/* Step number */
		tag.setAttribute(KXMLFunctionNumber, i++);

		/* Step Function ID */
		str.setNum(*it);
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}
}


//
// Add a new step into the end
//
void Chaser::addStep(t_function_id id)
{
	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.append(id);
		_app->doc()->setModified(true);
	}
	else
	{
		qDebug("Chaser is running. Cannot modify steps!");
	}  

	m_startMutex.unlock();
}


//
// Remove a step
//
void Chaser::removeStep(int index)
{
	ASSERT(((unsigned int)index) < m_steps.count());

	m_startMutex.lock();

	if (m_running == false)
	{
		m_steps.remove(m_steps.at(index));
		_app->doc()->setModified(true);
	}
	else
	{
		qDebug("Chaser is running. Cannot modify steps!");
	}

	m_startMutex.unlock();
}


//
// Raise the given step once (move it one step earlier)
//
bool Chaser::raiseStep(unsigned int index)
{
	bool result = false;

	m_startMutex.lock();

	if (m_running == false)
	{
		if (index > 0)
		{
			QValueList <t_function_id>::iterator it;
			it = m_steps.at(index);
			m_steps.remove(it);
			m_steps.insert(m_steps.at(index - 1), *it);
	  
			_app->doc()->setModified(true);

			result = true;
		}
	}
	else
	{
		qDebug("Chaser is running. Cannot modify steps!");
	}

	m_startMutex.unlock();

	return result;
}


//
// Lower the given step once (move it one step later)
//
bool Chaser::lowerStep(unsigned int index)
{
	bool result = false;

	m_startMutex.lock();

	if (m_running == false)
	{
		if (index < m_steps.count() - 1)
		{
			QValueList <t_function_id>::iterator it;
			it = m_steps.at(index);
			m_steps.remove(it);
			m_steps.insert(m_steps.at(index + 1), *it);

			_app->doc()->setModified(true);

			result = true;
		}
	}
	else
	{
		qDebug("Chaser is running. Cannot modify steps!");
	}

	m_startMutex.unlock();

	return result;
}


//
// Set run order
//
void Chaser::setRunOrder(RunOrder ro)
{
	m_runOrder = ro;
}


//
// Set direction
//
void Chaser::setDirection(Direction dir)
{
	m_direction = dir;
}

//
// Initiate a speed change (from a speed bus)
//
void Chaser::busValueChanged(t_bus_id id, t_bus_value value)
{
	if (id == m_busID)
	{
		m_holdTime = value;
	}
}


//
// Allocate everything needed in run-time
//
void Chaser::arm()
{
	// There's actually no need for an eventbuffer, but
	// because FunctionConsumer does EventBuffer::get() calls, it must be
	// there... So allocate a zero length buffer.
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}


//
// Delete everything needed in run-time
//
void Chaser::disarm()
{
	if (m_eventBuffer) delete m_eventBuffer;
	m_eventBuffer = NULL;
}

//
// Initialize some run-time values
//
void Chaser::init()
{
	m_childRunning = false;
	m_removeAfterEmpty = false;
	m_stopped = false;

	// Get speed
	Bus::value(m_busID, m_holdTime);

	// Add this to function consumer
	_app->functionConsumer()->cue(this);
}


//
// Main producer thread
//
void Chaser::run()
{
	// Calculate starting values
	init();

	m_runTimeDirection = m_direction;

	if (m_runTimeDirection == Forward)
	{
		m_runTimePosition = 0;
	}
	else
	{
		m_runTimePosition = m_steps.count() - 1;
	}

	while ( !m_stopped )
	{
		//
		// Run thru either normal or reverse
		//
		if (m_runTimeDirection == Forward)
		{
			while (m_runTimePosition < (int) m_steps.count() && !m_stopped)
			{
				m_childRunning = startMemberAt(m_runTimePosition);
	      
				// Wait for child to complete or stop signal
				while (m_childRunning && !m_stopped) sched_yield();

				if (m_stopped)
				{
					stopMemberAt(m_runTimePosition);
					break;
				}
				else
				{
					// Wait for m_holdTime
					hold();
					m_runTimePosition++;
				}
			}
		}
		else
		{
			while (m_runTimePosition >= 0 && !m_stopped)
			{
				m_childRunning = startMemberAt(m_runTimePosition);

				// Wait for child to complete or stop signal
				while (m_childRunning && !m_stopped) sched_yield();

				if (m_stopped)
				{
					stopMemberAt(m_runTimePosition);
					break;
				}
				else
				{
					// Wait for m_holdTime
					hold();
					m_runTimePosition--;
				}
			}
		}

		//
		// Check what should be done after a round
		//
		if (m_runOrder == SingleShot)
		{
			// That's it
			break;
		}
		else if (m_runOrder == Loop)
		{
			// Just continue as before
			m_runTimePosition = 0;
			continue;
		}
		else // if (m_runOrder == PingPong)
		{
			// Change run order
			if (m_runTimeDirection == Forward)
			{
				m_runTimeDirection = Backward;
	      
				// -2: Don't run the last function again
				m_runTimePosition = m_steps.count() - 2; 
			}
			else
			{
				m_runTimeDirection = Forward;

				// 1: Don't run the first function again
				m_runTimePosition = 1; 
			}
		}
	}

	// This chaser can be removed from the list after the buffer is empty.
	// (meaning immediately because this doesn't produce any events).
	m_removeAfterEmpty = true;
}


//
// Start a member function at index
//
bool Chaser::startMemberAt(int index)
{
	t_function_id id = *m_steps.at(index);
  
	Function* f = _app->doc()->function(id);
	if (!f)
	{
		qDebug("Chaser step function <id:%d> deleted!", id);
		return false;
	}
  
	if (f->engage(this))
	{
		return true;
	}
	else
	{
		qDebug("Chaser step function <id:%d> is already running!", id);
		return false;
	}
}


//
// Stop a member function at index
//
void Chaser::stopMemberAt(int index)
{
	t_function_id id = *m_steps.at(index);
  
	Function* f = _app->doc()->function(id);
	if (!f)
	{
		qDebug("Chaser step function <id:%d> deleted!", id);
	}
	else
	{
		f->stop();
	}
}


//
// Wait until m_holdTime ticks (1/Hz) have elapsed
//
void Chaser::hold()
{
	// Don't engage sleeping at all if holdtime is zero.
	if (m_holdTime > 0)
	{
		_app->functionConsumer()->timeCode(m_holdStart);
		while (!m_stopped)
		{
			_app->functionConsumer()->timeCode(m_timeCode);
			if ((m_timeCode - m_holdStart) >= m_holdTime)
			{
				break;
			}
			else
			{
				sched_yield();
			}
		}
	}
}


//
// Stop this function
//
void Chaser::stop()
{
	Function::stop();
}


//
// Do some post-run cleanup
//
void Chaser::cleanup()
{
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

	m_stopped = false;

	m_startMutex.lock();
	m_running = false;
	m_startMutex.unlock();
}


//
// Currently running child function calls this function
// when it is ready. This function wakes up the chaser
// producer thread.
//
void Chaser::childFinished()
{
	m_childRunning = false;
}
