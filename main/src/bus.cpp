/*
  Q Light Controller
  bus.cpp
  
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

#include <qfile.h>
#include <qstring.h>

#include "bus.h"
#include "app.h"
#include "function.h"

t_bus_id Bus::s_nextID               ( KBusIDMin );
Bus* Bus::s_busArray                 (      NULL );

//
// Constructor (private)
//
Bus::Bus()
{
  m_id = s_nextID++;
  m_value = 0;
  m_name.sprintf("Bus %d", m_id);
}


//
// Destructor
//
Bus::~Bus()
{
}


//
// Allocate all buses (static)
//
void Bus::initAll()
{
  s_busArray = new Bus[KBusCount];

  s_busArray[KBusIDDefaultFade].m_name = QString("Default Fade Speed");
  s_busArray[KBusIDDefaultHold].m_name = QString("Default Hold Speed");
}


//
// Set bus name (static)
//
bool Bus::setName(t_bus_id id, QString name)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      s_busArray[id].m_name = name;
      return true;
    }
  else
    {
      return false;
    }
}


//
// Set bus value (static)
//
bool Bus::setValue(t_bus_id id, t_bus_value value)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      s_busArray[id].m_value = value;

      QPtrListIterator <Function> it(s_busArray[id].m_listeners);

      while (it.current())
	{
	  it.current()->busValueChanged(id, value);
	  ++it;
	}

      return true;
    }
  else
    {
      return false;
    }
}


//
// Return bus value (static)
//
const bool Bus::value(t_bus_id id, t_bus_value& value)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      value = s_busArray[id].m_value;
      return true;
    }
  else
    {
      return false;
    }
}


//
// Return bus name (static)
//
const QString Bus::name(t_bus_id id)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      return s_busArray[id].m_name;
    }
  else
    {
      return QString::null;
    }
}


//
// Add a function to listen to changes in a bus
//
bool Bus::addListener(t_bus_id id, Function* function)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      if (s_busArray[id].m_listeners.find(function) == -1)
	{
	  s_busArray[id].m_listeners.append(function);
	  return true;
	}
      else
	{
	  return false;
	}
    }
  else
    {
      return false;
    }
}


//
// Remove a listener
//
bool Bus::removeListener(t_bus_id id, Function* function)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      int index = s_busArray[id].m_listeners.find(function);
      if (index == -1)
	{
	  return false;
	}
      else
	{
	  s_busArray[id].m_listeners.take(index);
	  return true;
	}
    }
  else
    {
      return false;
    }
}

//
// Save bus to a file
//
void Bus::saveToFile(QFile &file)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Bus Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Bus") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + m_name + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  t.setNum(m_id);
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Value
  t.setNum(m_value);
  s = QString("Value = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());  
}


//
// Load bus from file
//
void Bus::createContents(QPtrList <QString> &list)
{
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Name"))
	{
	  m_name = *(list.next());
	}
      else if (*s == QString("ID"))
	{
	  m_id = list.next()->toInt();
	}
      else if (*s == QString("Value"))
	{
	  m_value = list.next()->toInt();
	}
    }
}
