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

#include "function.h"
#include "functionstep.h"
#include "bus.h"

//
// Initialize function id
//
t_function_id Function::_nextFunctionID = KFunctionIDMin;

//
// Standard constructor
// id can be one of the following:
// KFunctionIDTemp: Don't assign an id (used for editing)
// KFunctionIDAuto: Assign an id automatically (default)
// Everything else: Assign the given id to this function (reading from a file)
//
Function::Function(t_function_id id) : QThread()
{
  if (id == KFunctionIDTemp)
    {
      m_id = id;
    }
  else if (id == KFunctionIDAuto)
    {
      m_id = _nextFunctionID;
      _nextFunctionID++;

      if (_nextFunctionID == KFunctionIDMax)
	{
	  // In case we ever go beyond the limit
	  _nextFunctionID = KFunctionIDMin;
	}
    }
  else
    {
      m_id = id;
      if (id >= _nextFunctionID)
	{
	  _nextFunctionID = id + 1;
	}

      if (_nextFunctionID == KFunctionIDMax)
	{
	  // In case we ever go beyond the limit
	  _nextFunctionID = KFunctionIDMin;
	}
    }

  m_name = QString::null;
  m_type = Function::Undefined;
  m_device = NULL;
  m_running = false;
  m_eventBuffer = NULL;
  m_virtualController = NULL;
  m_parentFunction = NULL;
  m_stopped = false;
  m_busID = KBusIDInvalid;
}


//
// Standard destructor
//
Function::~Function()
{
}


//
// Return the type as a string
//
QString Function::typeString() const
{
  switch (m_type)
    {
    case Collection:
      return QString("Collection");
      break;

    case Scene:
      return QString("Scene");
      break;

    case Chaser:
      return QString("Chaser");
      break;

    case Sequence:
      return QString("Sequence");
      break;

    default:
      return QString("Undefined");
      break;
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
      m_startMutex.unlock();
      return true;
    }
}


//
// Assign a device to this function (or vice versa, whichever feels
// familiar to you)
//
bool Function::setDevice(Device* device)
{
  m_startMutex.lock();
  if (m_running)
    {
      m_startMutex.unlock();
      return false;
    }
  else
    {
      m_device = device;
      m_startMutex.unlock();
      return true;
    }
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
      Bus::removeListener(m_busID, this);

      m_busID = id;

      Bus::addListener(m_busID, this);

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
