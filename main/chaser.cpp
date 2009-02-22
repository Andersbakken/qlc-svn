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
#include "chasereditor.h"
#include "fixture.h"
#include "chaser.h"
#include "doc.h"
#include "app.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(QObject* parent) : Function(parent, Function::Chaser)
{
	m_runTimeDirection = Forward;
	m_runTimePosition = 0;

	setBus(KBusIDDefaultHold);

	connect(Bus::emitter(), SIGNAL(tapped(t_bus_id)),
		this, SLOT(slotBusTapped(t_bus_id)));
}

void Chaser::copyFrom(Chaser* ch, bool append)
{
	Q_ASSERT(ch != NULL);

	Function::setName(ch->name());
	setDirection(ch->direction());
	setRunOrder(ch->runOrder());

	if (append == false)
		m_steps.clear();
	m_steps = ch->m_steps;
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
}

void Chaser::removeStep(unsigned int index)
{
	Q_ASSERT(int(index) < m_steps.size());
	m_steps.removeAt(index);
}

bool Chaser::raiseStep(unsigned int index)
{
	if (int(index) > 0 && int(index) < m_steps.count())
	{
		t_function_id fid = m_steps.takeAt(index);
		m_steps.insert(index - 1, fid);
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
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Edit
 *****************************************************************************/

int Chaser::edit()
{
	ChaserEditor editor(_app, this);
	int result = editor.exec();
	if (result == QDialog::Accepted)
		emit changed(m_id);
	return result;
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

void Chaser::slotBusTapped(t_bus_id id)
{
	if (id == m_busID)
		m_tapped = true;
}

void Chaser::arm()
{
}

void Chaser::disarm()
{
}

void Chaser::start()
{
	/* No point running this chaser if it has no steps */
	if (m_steps.count() == 0)
	{
		emit stopped(m_id);
		return;
	}

	/* Current position starts from invalid index because nextStep() is
	   called first in write(). */
	m_runTimeDirection = m_direction;
	if (m_runTimeDirection == Forward)
		m_runTimePosition = -1;
	else
		m_runTimePosition = m_steps.count();

	m_tapped = false;
	Function::start();
}

void Chaser::stop()
{
	stopMemberAt(m_runTimePosition);
	Function::stop();
}

bool Chaser::write(QByteArray* universes)
{
	Q_UNUSED(universes);

	/* TODO: With some changes to scene, chaser could basically act as a
	   proxy for its members by calling the scene's write() functions
	   by itself instead of starting/stopping them. Whether this would do
	   any good, is not clear. */

	if (m_elapsed == 0)
	{
		/* Get the next step index */
		nextStep();

		/* Start the current function */
		startMemberAt(m_runTimePosition);

		/* Mark one cycle being elapsed */
		m_elapsed = 1;
	}
	else if (m_elapsed >= Bus::value(m_busID) || m_tapped == true)
	{
		/* Reset tapped flag even if it wasn't true */
		m_tapped = false;

		/* Stop current function */
		stopMemberAt(m_runTimePosition);

		/* Get the next step index */
		nextStep();

		/* Check, whether we have gone a full cycle */
		if (roundCheck() == false)
			return false;

		/* Start the next step immediately */
		startMemberAt(m_runTimePosition);

		/* Mark one cycle being elapsed */
		m_elapsed = 1;
	}
	else
	{
		/* Just waiting for hold time to pass */
		m_elapsed++;
	}

	return true;
}

bool Chaser::roundCheck()
{
	/* Check if one complete chase round has been completed. */
	if ((m_runTimeDirection == Backward && m_runTimePosition == -1)
	    || (m_runTimeDirection == Forward && m_runTimePosition == m_steps.count()))
	{
		if (m_runOrder == SingleShot)
		{
			/* One round complete. Terminate. */
			return false;
		}
		else if (m_runOrder == Loop)
		{
			/* Loop around at either end */
			if (m_runTimeDirection == Forward)
				m_runTimePosition = 0;
			else
				m_runTimePosition = m_steps.count() - 1;
		}
		else /* Ping Pong */
		{
			/* Change running direction, don't run the step at
			   each end twice, so -1/+1 */
			if (m_runTimeDirection == Forward)
			{
				m_runTimePosition = m_steps.count() - 2;
				m_runTimeDirection = Backward;
			}
			else
			{
				m_runTimePosition = 1;
				m_runTimeDirection = Forward;
			}
		}
	}

	/* Let's continue */
	return true;
}

void Chaser::nextStep()
{
	if (m_runTimeDirection == Forward)
		m_runTimePosition++;
	else
		m_runTimePosition--;
}

void Chaser::startMemberAt(int index)
{
  	Function* function;
	t_function_id fid;

	fid = m_steps.at(index);
	Q_ASSERT(fid != KNoID);

	function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	function->start();
}

void Chaser::stopMemberAt(int index)
{
  	Function* function;
	t_function_id fid;

	fid = m_steps.at(index);
	Q_ASSERT(fid != KNoID);

	function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	function->stop();
}
