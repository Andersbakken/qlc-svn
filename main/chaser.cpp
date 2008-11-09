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
#include <QDebug>
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

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser() : Function(0, Function::Chaser)
{
	moveToThread(this);
	m_childRunning = false;

	m_holdTime = 0;
	m_holdStart = 0;

	m_runTimeDirection = Forward;
	m_runTimePosition = 0;

	setBus(KBusIDDefaultHold);

	connect(this, SIGNAL(stopping()),
	        this, SLOT(slotStop()));
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
		qDebug() << "Function node not found!";
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
			qDebug() << "Unknown chaser tag:" << tag.tagName();
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
	//pass control to the thread the chaser is running in.
	emit stopping();
}


void Chaser::slotStop()
{
	m_timer->stop();

	//If we are still running the child, stop it.
	if (m_childRunning)
	{
		t_function_id id = m_steps.at(m_runTimePosition);
		Function* function = _app->doc()->function(id);
		if (function != NULL)
			function->stop();
	}

	exit();
}

void Chaser::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	//does this affect us?
	if (id == m_busID)
	{
		m_holdTime = value;

		//if we have an active time we will have to change its length.
		if (m_timer->isActive())
		{
			updateTimer();
		}
	}	
}

void Chaser::updateTimer()
{
	t_bus_value newEndTimeCode = m_holdStart + m_holdTime;
	t_bus_value currentTimeCode = _app->functionConsumer()->timeCode();

	//should we have finished already?
	if (currentTimeCode >= newEndTimeCode)
	{
		//Don't bother with a timer, just advance.
		unsetTimer();
		advance();
	}
	else
	{
		bool advanceOnChange = false; //TODO make this a proper setting.
		if (advanceOnChange)
		{
			advance();
		}
		else
		{
			setTimer(newEndTimeCode - currentTimeCode);
		}
	}
}

void Chaser::slotChildStopped(t_function_id id)
{
	//finish with child function
	Function* function = _app->doc()->function(id);
	Q_ASSERT(function != NULL);

	disconnect(function, SIGNAL(stopped(t_function_id)),
		   this, SLOT(slotChildStopped(t_function_id)));

	//now we hold this function for m_holdTime
	m_childRunning = false;

	if (m_holdTime == 0)
		advance();
	else
		startTimer(m_holdTime);
}

void Chaser::slotTimerTimeout() 
{
	advance();
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

void Chaser::run()
{
	emit running(m_id);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()),
	        this, SLOT(slotTimerTimeout()));

	m_childRunning = false;

	// Get speed
	m_holdTime = Bus::value(m_busID);

	m_runTimeDirection = m_direction;

	if (m_runTimeDirection == Forward)
		m_runTimePosition = 0;
	else
		m_runTimePosition = m_steps.count() - 1;

	// Add this to function consumer
	_app->functionConsumer()->cue(this);

	startMemberAt(m_runTimePosition);

	exec();
}

void Chaser::startMemberAt(int index)
{
	if (m_childRunning)
	{
		qDebug() << 
		  "Tried to start a function when we are already running";
	}

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

	/* We will be told via slots when the child finishes or if this
	   chaser is stopped altogether. These slots will be responsible
	   for holding or stopping */
}

/**
 * If we have any more scenes to show, do so, if not terminate. 
 * Either way this returns quite quickly, 
 * we will be informed via a slot of the the next action to perform.
 */
void Chaser::advance()
{
	// Have we completed a run
	boolean finishedForward = (m_runTimeDirection == Forward
	                        && m_runTimePosition == m_steps.count() - 1)
	boolean finishedBackwards = (m_runTimeDirection == Backward
	                          && m_runTimePosition == 0)

	if (finishedForwards || finishedBackwards)
	{
		// Check what should be done after one round
		if (m_runOrder == SingleShot)
		{
			// That's it
			exit();
		}
		else if (m_runOrder == Loop)
		{
			// Just continue as before, start from the beginning
			if (m_runTimeDirection == Forward)
			{
				m_runTimePosition = 0;
			}
			else
			{
				m_runTimePosition = m_steps.count() - 1;
			}
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
	else
	{
		//we havn't finished, so advance.
		if (m_runTimeDirection == Forward)
		{
			/* Going forwards, next step */
			m_runTimePosition++;
		}
		else
		{
			/* Going backwards, previous step */
			m_runTimePosition--;
		}
	}

	//we want to continue, otherwise we would have returned/exited.
	startMemberAt(m_runTimePosition);
}

void Chaser::unsetTimer()
{
	m_timer->stop();
}

void Chaser::setTimer(t_bus_value time)
{
	//This restarts the timer if necessary
	m_timer->start(time * 1000 / KFrequency);
}

void Chaser::startTimer(t_bus_value time)
{
	m_holdStart = _app->functionConsumer()->timeCode();
	setTimer(time);
}
