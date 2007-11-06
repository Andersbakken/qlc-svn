/*
  Q Light Controller
  doc.cpp

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

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qptrlist.h>
#include <qmessagebox.h>
#include <assert.h>
#include <qdom.h>

#include "common/qlcworkspace.h"
#include "common/qlcfixturedef.h"
#include "common/qlcfixturemode.h"
#include "common/filehandler.h"

#include "app.h"
#include "doc.h"
#include "dmxmap.h"
#include "fixture.h"
#include "function.h"
#include "scene.h"
#include "functioncollection.h"
#include "chaser.h"
#include "efx.h"
#include "monitor.h"
#include "virtualconsole.h"
#include "functionconsumer.h"
#include "fixturemanager.h"

extern App* _app;

//
// Constructor
//
Doc::Doc() : QObject()
{
	m_fileName = QString::null;

	//
	// Allocate fixture array
	//
	m_fixtureArray = (Fixture**) malloc(sizeof(Fixture*) * KFixtureArraySize);
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		m_fixtureArray[i] = NULL;
	}

	//
	// Allocate function array
	//
	m_functionArray = (Function**)
		malloc(sizeof(Function*) * KFunctionArraySize);
	for (t_function_id i = 0; i < KFunctionArraySize; i++)
	{
		m_functionArray[i] = NULL;
	}

	connect(_app, SIGNAL(modeChanged(App::Mode)),
		this, SLOT(slotModeChanged(App::Mode)));

	resetModified();
}


//
// Destructor
//
Doc::~Doc()
{
	//
	// Delete all functions
	//
	for (t_function_id i = 0; i < KFunctionArraySize; i++)
	{
		if (m_functionArray[i])
		{
			delete m_functionArray[i];
			m_functionArray[i] = NULL;

			emit functionRemoved(i);
		}
	}

	delete [] m_functionArray;

	//
	// Delete all fixture instances
	//
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] != NULL)
		{
			delete m_fixtureArray[i];
			m_fixtureArray[i] = NULL;

			emit fixtureRemoved(i);
		}
	}

	delete [] m_fixtureArray;
}

void Doc::setModified()
{
	m_modified = true;
	emit modified(true);
}

void Doc::resetModified()
{
	m_modified = false;
	emit modified(false);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Doc::loadXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomDocumentType doctype;
	QString errorString;
	bool retval = false;

	Q_ASSERT(fileName != QString::null);

	if (FileHandler::readXML(fileName, &doc) == true)
	{
		if (doc->doctype().name() == KXMLQLCWorkspace)
		{
			if (loadXML(doc) == false)
			{
				QMessageBox::warning(_app, 
						     "Unable to open file",
				     fileName + " is not a valid workspace file");
				retval = false;
			}
			else
			{
				m_fileName = fileName;
				resetModified();
				retval = true;
			}
		}
		else
		{
			QMessageBox::warning(_app, "Unable to open file",
					     fileName + 
					     " is not a workspace file");
			retval = false;
		}
	}
	else
	{
		QMessageBox::warning(_app, "Unable to open file",
			     fileName + " is not a valid workspace file");
		retval = false;
	}

	return retval;
}

bool Doc::loadXML(QDomDocument* doc)
{
	Fixture* fxi = NULL;
	Function* function = NULL;
	QDomElement root;
	QDomNode node;
	QDomElement tag;

	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	Q_ASSERT(doc != NULL);

	root = doc->documentElement();
	
	/* Load workspace background & theme */
	_app->workspace()->loadXML(doc, &root);

	if (root.tagName() != KXMLQLCWorkspace)
	{
		qWarning("Workspace node not found in file!");
		return false;
	}

	node = root.firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCCreator)
		{
			/* Ignore creator information */
		}
		else if (tag.tagName() == KXMLQLCDMXMap)
		{
			_app->dmxMap()->loadXML(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
		}
		else if (tag.tagName() == KXMLFixture)
		{
			Fixture::loader(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCFunction)
		{
			Function::loader(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCBus)
		{
			Bus::loadXML(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCMonitor)
		{
			Monitor::loader(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCFixtureManager)
		{
			FixtureManager::loader(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVirtualConsole)
		{
			VirtualConsole::loader(doc, &tag);
		}
		else
			qDebug("Unknown Workspace tag: %s",
			       (const char*) tag.tagName());
		
		node = node.nextSibling();
	}

	_app->setGeometry(x, y, w, h);
	
	return true;
}

bool Doc::saveXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QFile file;
	bool retval = false;

	file.setName(fileName);
	if (file.open(IO_WriteOnly) == false)
		return false;

	if (FileHandler::getXMLHeader(KXMLQLCWorkspace, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();
		
		/* Write DMX mapping */
		_app->dmxMap()->saveXML(doc, &root);

		/* Write background image and theme */
		_app->workspace()->saveXML(doc, &root);

		/* Write window state & size */
		FileHandler::saveXMLWindowState(doc, &root, _app);

		/* Write fixtures into an XML document */
		for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
		{
			if (m_fixtureArray[i])
			{
				m_fixtureArray[i]->saveXML(doc, &root);
			}
		}

		/* Write functions into an XML document */
		for (t_function_id i = 0; i < KFunctionArraySize; i++)
		{
			if (m_functionArray[i] != NULL)
			{
				m_functionArray[i]->saveXML(doc, &root);
			}
		}

		/* Write buses */
		Bus::saveXML(doc, &root);

		/* Write Monitor state */
		if (_app->monitor() != NULL)
			_app->monitor()->saveXML(doc, &root);

		/* Write Fixture Manager state */
		if (_app->fixtureManager() != NULL)
			_app->fixtureManager()->saveXML(doc, &root);

		/* Write virtual console */
		_app->virtualConsole()->saveXML(doc, &root);

		/* Set the current file name and write the document
		   into the stream */
		m_fileName = fileName;
		stream << doc->toString() << "\n";

		/* Mark this Doc object as unmodified */
		resetModified();

		/* Delete the XML document */
		delete doc;

		retval = true;
	}
	else
	{
		retval = false;
	}

	file.close();

	return retval;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

Fixture* Doc::newFixture(QLCFixtureDef* fixtureDef,
			 QLCFixtureMode* mode,
			 t_channel address,
			 t_channel universe,
			 QString name)
{
	Fixture* fxi = NULL;
	
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] == NULL)
		{
			fxi = new Fixture(fixtureDef, mode, address, universe,
					  name, i);
			Q_ASSERT(fxi != NULL);

			m_fixtureArray[i] = fxi;
			setModified();

			// Patch fixture change events thru Doc
			connect(fxi, SIGNAL(changed(t_fixture_id)),
				this, SLOT(slotFixtureChanged(t_fixture_id)));

			emit fixtureAdded(i);
			break;
		}
	}

	if (fxi == NULL)
	{
		QString num;
		num.setNum(KFixtureArraySize);
		QMessageBox::warning(_app, 
				     "Unable to create fixture instance",
				     "You cannot create more than "
				     + num + " fixtures.");
	}

	return fxi;
}

Fixture* Doc::newGenericFixture(t_channel address,
				t_channel universe,
				t_channel channels,
				QString name)
{
	Fixture* fxi = NULL;
	
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		if (m_fixtureArray[i] == NULL)
		{
			fxi = new Fixture(address, universe, channels, name, i);
			Q_ASSERT(fxi != NULL);

			m_fixtureArray[i] = fxi;
			setModified();

			// Patch fixture change events thru Doc
			connect(fxi, SIGNAL(changed(t_fixture_id)),
				this, SLOT(slotFixtureChanged(t_fixture_id)));

			emit fixtureAdded(i);
			break;
		}
	}

	if (fxi == NULL)
	{
		QString num;
		num.setNum(KFixtureArraySize);
		QMessageBox::warning(_app, 
				     "Unable to create fixture instance",
				     "You cannot create more than "
				     + num + " fixtures.");
	}

	return fxi;
}

bool Doc::newFixture(Fixture* fxi)
{
	t_fixture_id id = 0;

	if (fxi == NULL)
		return false;

	id = fxi->id();

	if (id < 0 || id > KFixtureArraySize)
	{
		qWarning("Fixture ID %d out of bounds (%d - %d)!",
			 id, 0, KFixtureArraySize);
		return false;
	}
	else if (m_fixtureArray[id] != NULL)
	{
		qWarning("Fixture ID %d already taken by another fixture!", id);
		return false;
	}
	else
	{
		m_fixtureArray[id] = fxi;
		
		// Patch fixture change events thru Doc
		connect(fxi, SIGNAL(changed(t_fixture_id)),
			this, SLOT(slotFixtureChanged(t_fixture_id)));
		
		emit fixtureAdded(id);

		return true;
	}
}

bool Doc::deleteFixture(t_fixture_id id)
{
	if (id < 0 || id > KFixtureArraySize)
		return false;

	if (m_fixtureArray[id] != NULL)
	{
		delete m_fixtureArray[id];
		m_fixtureArray[id] = NULL;

		// Delete all functions associated with the removed fixture
		for (int i = 0; i < KFunctionArraySize; i++)
		{
			if (m_functionArray[i] != NULL && 
			    m_functionArray[i]->fixture() == id)
			{
				deleteFunction(i);
			}
		}

		emit fixtureRemoved(id);
		setModified();
		return true;
	}
	else
	{
		qDebug("No such fixture ID:%d", id);
		return false;
	}
}

Fixture* Doc::fixture(t_fixture_id id)
{
	if (id >= 0 && id < KFixtureArraySize)
		return m_fixtureArray[id];
	else
		return NULL;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

Function* Doc::newFunction(Function::Type type, t_fixture_id fixture)
{
	Function* function = NULL;
	bool ok = false;

	//
	// Create the function
	//
	switch(type)
	{
	case Function::Scene:
		function = new Scene();
		break;

	case Function::Chaser:
		function = new Chaser();
		break;

	case Function::Collection:
		function = new FunctionCollection();
		break;

	case Function::EFX:
		function = new EFX();
		break;

	default:
		function = NULL;
	}

	//
	// If the function was created successfully, save it to function
	// array and set its position in the array as its ID
	//
	if (function == NULL)
		return NULL;

	// Find the next free space from function array
	for (t_function_id id = 0; id < KFunctionArraySize; id++)
	{
		if (m_functionArray[id] == NULL)
		{
			m_functionArray[id] = function;
			function->setID(id);
			function->setFixture(fixture);

			emit functionAdded(id);

			ok = true;
			break;
		}
	}

	if (ok == false)
	{
		qWarning("Function array is full. Cannot add any more functions!");
		delete function;
		function = NULL;
	}
	
	return function;
}

Function* Doc::newFunction(Function::Type func_type, 
			   t_function_id func_id,
			   QString func_name,
			   t_fixture_id fxi_id, 
			   QDomDocument* doc,
			   QDomElement* root)
{
	Function* function = NULL;

	Q_ASSERT(func_id >= 0 && func_id < KFunctionArraySize);
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	switch (func_type)
	{
	case Function::Scene:
		function = new Scene();
		break;

	case Function::Chaser:
		function = new Chaser();
		break;

	case Function::Collection:
		function = new FunctionCollection();
		break;

	case Function::EFX:
		function = new EFX();
		break;
		
	default:
		function = NULL;
		break;
	}
	
	/* Put the function to its place (same as its ID)
	   in the function array */
	if (m_functionArray[func_id] == NULL)
	{
		m_functionArray[func_id] = function;
		
		function->setID(func_id);
		function->setFixture(fxi_id);
		function->setName(func_name);
		
		/* Continue loading the function contents */
		if (function->loadXML(doc, root) == false)
		{
			delete function;
			function = NULL;
			m_functionArray[func_id] = NULL;
		}
		
		emit functionAdded(func_id);
	}
	else
	{
		qWarning("Function ID %d already "\
			 "taken by another function!", func_id);

		delete function;
		function = NULL;
		m_functionArray[func_id] = NULL;
	}

	return function;
}

//
// Remove a function by a given id
//
void Doc::deleteFunction(t_function_id id)
{
	if (m_functionArray[id])
	{
		delete m_functionArray[id];
		m_functionArray[id] = NULL;
      
		emit functionRemoved(id);
	}
}


//
// Return a function from the function array
//
Function* Doc::function(t_function_id id)
{
	if (id >= 0 && id < KFunctionArraySize)
	{
		return m_functionArray[id];
	}
	else
	{
		return NULL;
	}
}

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

//
// Emit a functionChanged() signal.
//
// Because Function is not a QObject, it cannot emit signals by itself.
// Therefore it must call this function to make Doc emit the function
// change signal, instead.
//
void Doc::emitFunctionChanged(t_function_id fid)
{
	emit functionChanged(fid);
}

//
// Mode changed
//
void Doc::slotModeChanged(App::Mode mode)
{
	Function* f = NULL;
	if (mode == App::Operate)
	{
		//
		// Arm all functions, allocate anything that is needed
		// during run-time.
		//
		for (int i = 0; i < KFunctionArraySize; i++)
		{
			f = m_functionArray[i];
			if (f)
			{
				f->arm();
			}
		}
	}
	else
	{
		//
		// Disarm all functions, delete anything that was
		// allocated above.
		//
		for (int i = 0; i < KFunctionArraySize; i++)
		{
			f = m_functionArray[i];
			if (f)
			{
				f->disarm();
			}
		}
	}
}

void Doc::slotFixtureChanged(t_fixture_id id)
{
	setModified();
	emit fixtureChanged(id);
}
