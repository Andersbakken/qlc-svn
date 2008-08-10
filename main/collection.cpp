/*
  Q Light Controller
  collection.cpp
  
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
#include <QString>
#include <QFile>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "collection.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

using namespace std;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Collection::Collection(QObject* parent) : Function(parent, Function::Collection)
{
	m_childCount = 0;
}

void Collection::copyFrom(Collection* fc, bool append)
{
	Q_ASSERT(fc != NULL);

	Function::setName(fc->name());
	Function::setBus(fc->busID());

	if (append == false)
		m_steps.clear();

	QListIterator <t_function_id> it(fc->m_steps);
	while (it.hasNext() == true)
		m_steps.append(it.next());
}

Collection::~Collection()
{
	m_steps.clear();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Collection::saveXML(QDomDocument* doc, QDomElement* wksp_root)
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

bool Collection::loadXML(QDomDocument*, QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		cout << "Function node not found!" << endl;
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
			cout << "Unknown collection tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}

		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void Collection::addItem(t_function_id id)
{
	m_steps.append(id);
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

void Collection::removeItem(t_function_id id)
{
	m_steps.takeAt(m_steps.indexOf(id));
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Collection::arm()
{
	// There's actually no need for an eventbuffer, but
	// because FunctionConsumer does EventBuffer::get() calls, it must be
	// there... So allocate a zero length buffer.
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(0, 0);
}

void Collection::disarm()
{
	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;
}

void Collection::stop()
{
	/* TODO: this stops these functions, regardless of whether they
	   were started by this collection or not */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		Function* function = _app->doc()->function(it.next());
		if (function != NULL)
			function->stop();
	}
}

void Collection::run()
{
	emit running(m_id);

	m_childCount = 0;

	// Append this function to the list of running functions
	_app->functionConsumer()->cue(this);
  
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		Function* function = _app->doc()->function(it.next());
		if (function != NULL)
		{
			m_childCountMutex.lock();
			m_childCount++;
			m_childCountMutex.unlock();

			connect(function, SIGNAL(stopped(t_function_id)),
				this, SLOT(slotChildStopped(t_function_id)));

			function->start();
		}
	}

	// Wait for all children to stop
	while (m_childCount > 0)
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

void Collection::slotChildStopped(t_function_id fid)
{
	Function* function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	disconnect(function, SIGNAL(stopped(t_function_id)),
		   this, SLOT(slotChildStopped(t_function_id)));	
	
	m_childCountMutex.lock();
	m_childCount--;
	m_childCountMutex.unlock();
}
