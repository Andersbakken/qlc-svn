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
BusEmitter* Bus::s_busEmitter        (      NULL );

//
// Constructor (private)
//
Bus::Bus()
{
  m_id = s_nextID++;
  m_value = 0;
  m_name = QString::null;
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
void Bus::init()
{
  if (s_busArray) delete s_busArray;
  s_busArray = new Bus[KBusCount];

  s_busArray[KBusIDDefaultFade].m_name = QString("Fade");
  s_busArray[KBusIDDefaultHold].m_name = QString("Hold");

  if (s_busEmitter) delete s_busEmitter;
  s_busEmitter = new BusEmitter();
}


//
// Set bus name (static)
//
bool Bus::setName(t_bus_id id, QString name)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      s_busArray[id].m_name = name;
      s_busEmitter->emitNameChanged(id, name);
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
      s_busEmitter->emitValueChanged(id, value);

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
// Save bus to a file (static)
//
void Bus::saveToFile(QFile &file)
{
  QString s;
  t_bus_value value;

  // Comment
  s = QString("# Bus Entries\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Bus") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
    {
      Bus::value(i, value);
      if (value != 0 || Bus::name(i) != QString::null)
	{
	  // Write only non-nil values

	  // Another comment
	  s = QString("# Bus ID, its value and its name\n");
	  file.writeBlock((const char*) s, s.length());
	  
	  // ID
	  s.sprintf("ID = %d\n", i);
	  file.writeBlock((const char*) s, s.length());
	  
	  // Value
	  s.sprintf("Value = %ld\n", value);
	  file.writeBlock((const char*) s, s.length());
	  
	  // Name
	  s = QString("Name = ") + Bus::name(i) + QString("\n");
	  file.writeBlock((const char*) s, s.length());
	}
    }
}


//
// Load bus from file (static)
//
void Bus::createContents(QPtrList <QString> &list)
{
  t_bus_id id = KBusIDInvalid;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("ID"))
	{
	  id = list.next()->toInt();
	}
      else if (*s == QString("Value"))
	{
	  Bus::setValue(id, list.next()->toInt());
	}
      else if (*s == QString("Name"))
	{
	  Bus::setName(id, *(list.next()));
	}
    }
}
