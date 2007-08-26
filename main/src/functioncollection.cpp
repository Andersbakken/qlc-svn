/*
  Q Light Controller
  functioncollection.cpp
  
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

#include <qapplication.h>
#include <qstring.h>
#include <qthread.h>
#include <stdlib.h>
#include <qfile.h>
#include <qptrlist.h>
#include <assert.h>

#include "common/filehandler.h"
#include "functioncollection.h"
#include "function.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "eventbuffer.h"

extern App* _app;

//
// Standard constructor
//
FunctionCollection::FunctionCollection()
	: Function(Function::Collection),

	  m_childCount      (     0 ),
	  m_childCountMutex ( false )
{
}


//
// Copy give function's contents to this
//
void FunctionCollection::copyFrom(FunctionCollection* fc, bool append)
{
	assert(fc);

	Function::setName(fc->name());
	Function::setBus(fc->busID());
  
	if (append == false)
	{
		m_steps.clear();
	}
  
	QValueList <t_function_id>::iterator it;
	for (it = fc->m_steps.begin(); it != fc->m_steps.end(); ++it)
	{
		m_steps.append(*it);
	}
}


//
// Destructor
//
FunctionCollection::~FunctionCollection()
{
	stop();
	while(m_running) pthread_yield();

	m_steps.clear();
}


//
// Save function's contents to an XML document
//
bool FunctionCollection::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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
	root.setAttribute(KXMLQLCFunctionFixture, fixture());
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Steps */
	QValueList <t_function_id>::iterator it;
	for (it = m_steps.begin(); it != m_steps.end(); ++it)
	{
		/* Step tag */
		tag = doc->createElement(KXMLQLCFunctionStep);
		root.appendChild(tag);
		
		/* Step number */
		tag.setAttribute(KXMLQLCFunctionNumber, i++);

		/* Step Function ID */
		str.setNum(*it);
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}

	return true;
}


bool FunctionCollection::loadXML(QDomDocument* doc, QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning("Function node not found!");
		return false;
	}

	/* Load collection contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFunctionStep)
		{
			step_fxi = tag.text().toInt();
			m_steps.append(step_fxi);
		}
		else
		{
			qWarning("Unknown collection tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}


//
// Add a function to this collection
//
bool FunctionCollection::addItem(t_function_id id)
{
	m_startMutex.lock();

	if (!m_running)
	{
		m_steps.append(id);

		m_startMutex.unlock();
		return true;
	}

	m_startMutex.unlock();
	return false;
}


//
// Remove a function from this collection (direct function pointer)
//
bool FunctionCollection::removeItem(t_function_id id)
{
	m_startMutex.lock();

	if (!m_running)
	{
		m_steps.remove(id);
      
		m_startMutex.unlock();
		return true;
	}

	m_startMutex.unlock();
	return false;
}


//
// Initiate a speed change (from a speed bus)
//
void FunctionCollection::speedChange()
{
}


//
// Allocate some stuff for run-time
//
void FunctionCollection::arm()
{
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}


//
// Delete run-time allocations
//
void FunctionCollection::disarm()
{
	if (m_eventBuffer) delete m_eventBuffer;
	m_eventBuffer = NULL;
}


//
// Stop
//
void FunctionCollection::stop()
{
	QValueList <t_function_id>::iterator it;
	for (it = m_steps.begin(); it != m_steps.end(); ++it)
	{
		Function* f = _app->doc()->function(*it);
		if (f)
		{
			f->stop();
		}
	}
}


//
// Initialize some run-time values
//
void FunctionCollection::init()
{
	m_childCountMutex.lock();
	m_childCount = 0;
	m_childCountMutex.unlock();

	m_stopped = false;
	m_removeAfterEmpty = false;

	// Append this function to running functions list
	_app->functionConsumer()->cue(this);
}


//
// Main producer thread
//
void FunctionCollection::run()
{
	QValueList <t_function_id>::iterator it;

	// Calculate starting values
	init();
  
	for (it = m_steps.begin(); it != m_steps.end() && !m_stopped; ++it)
	{
		Function* f = _app->doc()->function(*it);
		if (f && f->engage(this))
		{
			m_childCountMutex.lock();
			m_childCount++;
			m_childCountMutex.unlock();
		}
		else
		{
			qDebug("Collection unable to start function <id:%d>", *it);
		}
	}

	// Wait for all children to stop
	//m_childCountMutex.lock();
	while (m_childCount > 0)
	{
		//m_childCountMutex.unlock();
		pthread_yield();
	}

	m_removeAfterEmpty = true;
}

//
// Do some post-run cleanup
//
void FunctionCollection::cleanup()
{
	ASSERT(m_childCount == 0);

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


void FunctionCollection::childFinished()
{
	m_childCountMutex.lock();
	m_childCount--;
	m_childCountMutex.unlock();
}

