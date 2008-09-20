/*
  Q Light Controller
  chaser.cpp
  
  Copyright (c) Heikki Junnila
  
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

#include <QApplication>
#include <iostream>
#include <QFile>
#include <QtXml>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "app.h"
#include "bus.h"

extern App* _app;

using namespace std;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(QObject* parent) : Function(parent, Function::Chaser)
{
	m_childRunning = false;

	m_holdTime = 0;
	m_holdStart = 0;
	m_timeCode = 0;

	m_runTimeDirection = Forward;
	m_runTimePosition = 0;

	setBus(KBusIDDefaultHold);
}

void Chaser::copyFrom(Chaser* ch, bool append)
{
	Q_ASSERT(ch != NULL);

	Function::setName(ch->name());
	setDirection(ch->direction());
	setRunOrder(ch->runOrder());

	if (append == false)
		m_steps.clear();

	QListIterator <t_function_id> it(ch->m_steps);
	while (it.hasNext() == true)
		m_steps.append(it.next());
}

Chaser::~Chaser()
{
	m_steps.clear();
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void Chaser::addStep(t_function_id id)
{
	m_steps.append(id);
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

void Chaser::removeStep(unsigned int index)
{
	Q_ASSERT(int(index) < m_steps.size());

	m_steps.removeAt(index);
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

bool Chaser::raiseStep(unsigned int index)
{
	if (int(index) > 0 && int(index) < m_steps.count())
	{
		t_function_id fid = m_steps.takeAt(index);
		m_steps.insert(index - 1, fid);
		
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);

		return true;
	}
	else
	{
		return false;
	}
}

bool Chaser::lowerStep(unsigned int index)
{
	if (int(index) < (m_steps.count() - 1))
	{
		t_function_id fid = m_steps.takeAt(index);
		m_steps.insert(index + 1, fid);
		
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);

		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	int i = 0;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Speed bus */
	tag = doc->createElement(KXMLQLCBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Direction */
	tag = doc->createElement(KXMLQLCFunctionDirection);
	root.appendChild(tag);
	text = doc->createTextNode(Function::directionToString(m_direction));
	tag.appendChild(text);

	/* Run order */
	tag = doc->createElement(KXMLQLCFunctionRunOrder);
	root.appendChild(tag);
	text = doc->createTextNode(Function::runOrderToString(m_runOrder));
	tag.appendChild(text);

	/* Steps */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		/* Step tag */
		tag = doc->createElement(KXMLQLCFunctionStep);
		root.appendChild(tag);

		/* Step number */
		tag.setAttribute(KXMLQLCFunctionNumber, i++);

		/* Step Function ID */
		str.setNum(it.next());
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}

	return true;
}

bool Chaser::loadXML(QDomDocument*, QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	int step_number = 0;
	QString str;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		cout << "Function node not found!" << endl;
		return false;
	}

	/* Load chaser contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCBus)
		{
			/* Bus */
			str = tag.attribute(KXMLQLCBusRole);
			Q_ASSERT(str == KXMLQLCBusHold);

			setBus(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionDirection)
		{
			/* Direction */
			setDirection(Function::stringToDirection(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCFunctionRunOrder)
		{
			/* Run Order */
			setRunOrder(Function::stringToRunOrder(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCFunctionStep)
		{
			step_number = 
				tag.attribute(KXMLQLCFunctionNumber).toInt();
			step_fxi = tag.text().toInt();

			if (step_number >= m_steps.size())
				m_steps.append(step_fxi);
			else
				m_steps.insert(m_steps.at(step_number),
					       step_fxi);
			
		}
		else
		{
			cout << "Unknown chaser tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::stop()
{
	m_stopped = true;
}

void Chaser::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	if (id == m_busID)
		m_holdTime = value;
}

void Chaser::slotChildStopped(t_function_id id)
{
	Function* function = _app->doc()->function(id);
	Q_ASSERT(function != NULL);

	disconnect(function, SIGNAL(stopped(t_function_id)),
		   this, SLOT(slotChildStopped(t_function_id)));
	m_childRunning = false;
}

void Chaser::arm()
{
	// There's actually no need for an eventbuffer, but
	// because FunctionConsumer does EventBuffer::get() calls, it must be
	// there... So allocate a zero length buffer.
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}

void Chaser::disarm()
{
	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::run()
{
	emit running(m_id);

	m_childRunning = false;
	m_stopped = false;

	// Get speed
	m_holdTime = Bus::value(m_busID);

	m_runTimeDirection = m_direction;

	if (m_runTimeDirection == Forward)
		m_runTimePosition = 0;
	else
		m_runTimePosition = m_steps.count() - 1;

	// Add this to function consumer
	_app->functionConsumer()->cue(this);

	while (m_stopped == false)
	{
		// Run thru this chaser, either forwards or backwards
		if (m_runTimeDirection == Forward)
		{
			while (m_runTimePosition < (int) m_steps.count() &&
			       m_stopped == false)
			{
				startMemberAt(m_runTimePosition);

				/* Going forwards, next step */
				m_runTimePosition++;
			}
		}
		else
		{
			while (m_runTimePosition >= 0 && m_stopped == false)
			{
				startMemberAt(m_runTimePosition);
				
				/* Going backwards, previous step */
				m_runTimePosition--;
			}
		}

		// Check what should be done after one round
		if (m_runOrder == SingleShot)
		{
			// That's it
			break;
		}
		else if (m_runOrder == Loop)
		{
			// Just continue as before, start from the beginning
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
}

void Chaser::startMemberAt(int index)
{
	t_function_id id = m_steps.at(index);
  	Function* function = _app->doc()->function(id);

	/* Check that the function exists */
	m_childRunning = false;
	if (function == NULL)
		return;

	/* Start the child function */
	connect(function, SIGNAL(stopped(t_function_id)),
		this, SLOT(slotChildStopped(t_function_id)));
	m_childRunning = true;
	function->start();

	/* Wait for the child function to complete or the user to stop this
	   chaser altogether. Mutexes should not be needed here, although
	   m_childRunning is set false from FunctionConsumer's context. */
	while (m_childRunning == true && m_stopped == false)
	{
		/* Give away this process' time slot since it's just waiting */
		#ifdef WIN32
			QThread::msleep(1000/KFrequency);
		#elif __APPLE__
			pthread_yield_np();
		#else
			pthread_yield();
		#endif
	}
	
	if (m_stopped == true)
		stopMemberAt(m_runTimePosition);
	else
		hold();
}

void Chaser::stopMemberAt(int index)
{
	t_function_id id = m_steps.at(index);
	Function* function = _app->doc()->function(id);

	if (function == NULL)
		return;

	function->stop();
}

void Chaser::hold()
{
	/* Don't engage sleeping at all if holdtime is zero */
	if (m_holdTime <= 0)
		return;

	m_holdStart = _app->functionConsumer()->timeCode();
	while (m_stopped == false)
	{
		m_timeCode = _app->functionConsumer()->timeCode();
		if ((m_timeCode - m_holdStart) >= m_holdTime)
		{
			break;
		}
		else
		{
#ifndef __APPLE__
#ifndef WIN32
			pthread_yield();
#endif
#else
			pthread_yield_np();
#endif
		}

	}
}
