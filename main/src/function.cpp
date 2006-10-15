/*
  Q Light Controller
  function.cpp

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

#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <assert.h>
#include <qpixmap.h>

#include "bus.h"
#include "app.h"
#include "doc.h"
#include "function.h"
#include "functioncollection.h"
#include "chaser.h"
#include "scene.h"

extern App* _app;

const QString KCollectionString ( "Collection" );
const QString KSceneString      (      "Scene" );
const QString KChaserString     (     "Chaser" );
const QString KSequenceString   (   "Sequence" );
const QString KEFXString        (        "EFX" );
const QString KUndefinedString  (  "Undefined" );

//
// Standard constructor (protected)
//
Function::Function(Type type) :
  QThread(),
  m_name              ( QString::null ),
  m_type              (          type ),
  m_id                (         KNoID ),
  m_deviceID          (         KNoID ),
  m_busID             ( KBusIDInvalid ),
  m_channels          (             0 ),
  m_eventBuffer       (          NULL ),
  m_virtualController (          NULL ),
  m_parentFunction    (          NULL ),
  m_running           (         false ),
  m_stopped           (         false ),
  m_removeAfterEmpty  (         false ),
  m_startMutex        (         false ),
  m_listener          (          NULL )
{
}


//
// Standard destructor
//
Function::~Function()
{
  delete m_listener;
  m_listener = NULL;
}

//
// Set the ID
//
void Function::setID(t_function_id id)
{
  m_id = id;

  if (m_listener) delete m_listener;
  m_listener = new FunctionNS::BusListener(m_id);
}

//
// Return the type as a string
//
QString Function::typeToString(Type type)
{
  switch (type)
    {
    case Collection:
      return KCollectionString;
      break;

    case Scene:
      return KSceneString;
      break;

    case Chaser:
      return KChaserString;
      break;

    case Sequence:
      return KSequenceString;
      break;

    case EFX:
      return KEFXString;
      break;

    case Undefined:
    default:
      return KUndefinedString;
      break;
    }
}


//
// Return the given string as a type enum
//
Function::Type Function::stringToType(QString string)
{
  if (string == KCollectionString)
    {
      return Collection;
    }
  else if (string == KSceneString)
    {
      return Scene;
    }
  else if (string == KChaserString)
    {
      return Chaser;
    }
  else if (string == KSequenceString)
    {
      return Sequence;
    }
  else if (string == KEFXString)
    {
      return EFX;
    }
  else
    {
      return Undefined;
    }
}

//
// Set a name to this function
//
bool Function::setName(QString name)
{
  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      return false;
    }
  else
    {
      m_name = QString(name);
      _app->doc()->setModified(true);
      m_startMutex.unlock();

      _app->doc()->emitFunctionChanged(m_id);

      return true;
    }
}


//
// Assign a device to this function (or vice versa, whichever feels
// familiar to you)
//
bool Function::setDevice(t_device_id id)
{
  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      return false;
    }
  else
    {
      m_deviceID = id;
      _app->doc()->setModified(true);
      m_startMutex.unlock();

      _app->doc()->emitFunctionChanged(m_id);

      return true;
    }
}

//
// Get a textual representation of the function's bus (ID: Name)
//
QString Function::busName()
{
	QString text;

	if (busID() != KNoID)
	{
		text.sprintf("%.2d: ", busID() + 1);
		text += Bus::name(busID());
	}
	else
	{
		text = QString("N/A");
	}

	return text;
}

//
// Set the speed bus
//
bool Function::setBus(t_bus_id id)
{
  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      return false;
    }
  else
    {
      if (id < KBusIDMin || id >= KBusCount)
	{
	  if (m_type == Scene)
	    {
	      m_busID = KBusIDDefaultFade;
	    }
	  else if (m_type == Chaser || m_type == Sequence)
	    {
	      m_busID = KBusIDDefaultHold;
	    }
	  else
	    {
	      m_busID = KNoID;
	    }
	}
      else
	{
	  m_busID = id;
	}

      _app->doc()->setModified(true);

      _app->doc()->emitFunctionChanged(m_id);
      
      m_startMutex.unlock();
      return true;
    }
}


////////////////////////
// Start the function //
////////////////////////

//
// This function is used by VCButton to pass itself as a virtual controller
// to this function. m_virtualController is signaled when this function stops.
//
bool Function::engage(QObject* virtualController)
{
  ASSERT(virtualController);

  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      qDebug("Function " + name() + " is already running!");
      return false;
    }
  else
    {
      m_virtualController = virtualController;
      m_running = true;
      start();
      m_startMutex.unlock();

      return true;
    }
}

//
// This function is used by Chaser & Collection to pass themselves as
// a parent function to this function. m_parentFunction->childFinished()
// is called when this function stops.
//
bool Function::engage(Function* parentFunction)
{
  ASSERT(parentFunction);

  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      qDebug("Function " + name() + " is already running!");
      return false;
    }
  else
    {
      m_parentFunction = parentFunction;
      m_running = true;
      start();
      m_startMutex.unlock();

      return true;
    }
}


//
// Stop this function
//
void Function::stop()
{
  m_stopped = true;
}


//
// Create a function from a file entry
//
Function* Function::create(QPtrList <QString> &list)
{
  Function* function = NULL;

  QString name;
  Type type = Undefined;
  t_function_id fid = KNoID;
  t_device_id did = KNoID;
  t_bus_id busid = KNoID;

  //
  // Read basic information
  //
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());
	}
      else if (*s == QString("Type"))
	{
	  type = Function::stringToType(*(list.next()));
	}
      else if (*s == QString("ID"))
	{
	  fid = list.next()->toInt();
	}
      else if (*s == QString("Bus"))
	{
	  busid = list.next()->toInt();
	}
      else if (*s == QString("Device"))
	{
	  did = list.next()->toInt();
	  break;
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }

  //
  // Create the function and its contents
  //
  function = _app->doc()->newFunction(type, did, fid);
  if (function)
    {
      function->setName(name);
      function->setBus(busid);
      function->createContents(list);
    }
  else
    {
      QString msg;
      msg = QString("Unable to create function:\n");
      msg += QString("No more free function slots!");
    }

  return function;
}

//
// Get a pixmap representing the function's type to be used in lists etc.
//
QPixmap Function::pixmap()
{
	switch (m_type)
	{
		case Scene:
			return QPixmap(QString(PIXMAPS) +
					QString("/scene.png"));
		case Chaser:
			return QPixmap(QString(PIXMAPS) +
					QString("/chaser.png"));
		case Collection:
			return QPixmap(QString(PIXMAPS) +
					QString("/collection.png"));
		case Sequence:
			return QPixmap(QString(PIXMAPS) +
					QString("/sequence.png"));
		case EFX:
			return QPixmap(QString(PIXMAPS) +
					QString("/efx.png"));
		default:
			return QPixmap(QString(PIXMAPS) +
					QString("/function.png"));
	}
}



///////////////////////////
// namespace FunctionNS ///
///////////////////////////

//
// Bus Listener Constructor
//
FunctionNS::BusListener::BusListener(t_function_id id)
  : m_functionID(id)
{
  connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	  this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));
}

//
// Bus Listener destructor
//
FunctionNS::BusListener::~BusListener()
{
  disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	     this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));
}


//
// Bus Listener slot
//
void FunctionNS::BusListener::slotBusValueChanged(t_bus_id id,
						  t_bus_value value)
{
  Function* f = _app->doc()->function(m_functionID);
  assert(f);
  f->busValueChanged(id, value);
}
