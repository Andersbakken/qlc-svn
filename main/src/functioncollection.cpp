/*
  Q Light Controller
  functioncollection.cpp
  
  Copyright (C) Heikki Junnila
  
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

#include "functioncollection.h"
#include "functionstep.h"
#include "function.h"
#include "deviceclass.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"

#include <qstring.h>
#include <qthread.h>
#include <stdlib.h>
#include <qfile.h>
#include <qptrlist.h>

extern App* _app;

//
// Standard constructor
//
FunctionCollection::FunctionCollection(t_function_id id)
  : Function(id)
{
  m_type = Function::Collection;
  m_running = false;
}


//
// Copy constructor
//
FunctionCollection::FunctionCollection(FunctionCollection* fc) 
  : Function(fc->id())
{
  copyFrom(fc);
}


//
// Copy give function's contents to this
//
void FunctionCollection::copyFrom(FunctionCollection* fc)
{
  m_type = Function::Collection;
  m_running = fc->m_running;
  m_name = fc->m_name;

  QPtrList <FunctionStep> *il = fc->steps();

  for (FunctionStep* step = il->first(); step != NULL; step = il->next())
    {
      FunctionStep* newItem = new FunctionStep(step);
      m_steps.append(newItem);
    }
}


//
// Destructor
//
FunctionCollection::~FunctionCollection()
{
  while (m_steps.isEmpty() == false)
    {
      delete m_steps.take(0);
    }
}


//
// Save this function's contents to given file
//
void FunctionCollection::saveToFile(QFile &file)
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
  s = QString("Type = ") + typeString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  s.sprintf("ID = %d\n", id());
  file.writeBlock((const char*) s, s.length());

  // Device
  if (m_device != NULL)
    {
      s.sprintf("Device = %d\n", m_device->id());
    }
  else
    {
      s = QString("Device = 0\n");
    }
  file.writeBlock((const char*) s, s.length());

  // For global collections, write device+scene pairs
  for (FunctionStep* step = m_steps.first(); step != NULL;
       step = m_steps.next())
    {
      s.sprintf("Function = %d", step->function()->id());
      file.writeBlock((const char*) s, s.length());
    }
}


//
// Create this function's contents from a list
//
void FunctionCollection::createContents(QPtrList <QString> &list)
{
  t_device_id did = 0;
  t_function_id fid = 0;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Device"))
	{
	  did = list.next()->toULong();
	}
      else if (*s == QString("Function"))
	{
	  fid = list.next()->toULong();

	  if (did == 0)
	    {
	      Function* f = _app->doc()->searchFunction(fid);
	      if (f != NULL)
		{
		  addItem(f);
		}
	      else
		{
		  qDebug("Unable to find member for function collection <"
			 + name() + ">");
		}
	    }
	  else
	    {
	      Function* f = NULL;
	      f = _app->doc()->searchFunction(fid);
	      if (f != NULL)
		{
		  addItem(f);
		}
	      else
		{
		  qDebug("Unable to find member %d for function collection <"
			 + name() + ">", fid);
		}
	    }
	}
      else
	{
	  // Unknown keyword (at this time)
	  list.next();
	}
    }
}


//
// Add a function to this collection
//
void FunctionCollection::addItem(Function* function)
{
  ASSERT(function != NULL);

  FunctionStep* step = new FunctionStep();
  step->setFunction(function);

  m_steps.append(step);
}


//
// Remove a function from this collection (direct function pointer)
//
bool FunctionCollection::removeItem(Function* function)
{
  bool retval = false;

  for (FunctionStep* step = m_steps.first(); step != NULL; 
       step = m_steps.next())
    {
      if (step->function()->id() == function->id())
	{
	  delete m_steps.take();
	  retval = true;
	  break;
	}
    }

  return retval;
}


//
// Remove a function from this collection (with function id)
//
bool FunctionCollection::removeItem(const t_function_id functionId)
{
  bool retval = false;

  for (FunctionStep* step = m_steps.first(); step != NULL; 
       step = m_steps.next())
    {
      if (step->function()->id() == functionId)
	{
	  delete m_steps.take();
	  retval = true;
	  break;
	}
    }

  return retval;
}


//
// Initiate a speed change (from a speed bus)
//
void FunctionCollection::speedChange(long unsigned int newTimeSpan)
{
}


//
// Explicitly stop this function
//
void FunctionCollection::stop()
{
  m_running = false;
}


//
// Free run-time allocations
//
void FunctionCollection::freeRunTimeData()
{
}


//
// Initialize some run-time values
//
void FunctionCollection::init()
{
}


//
// Main producer thread
//
void FunctionCollection::run()
{
  // Calculate starting values
  init();
  
  // Append this function to running functions list
  _app->functionConsumer()->cue(this);
}
