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

static int _nextBusID = BUS_ID_MIN;

Bus::Bus() : QObject()
{
  m_id = _nextBusID;
  _nextBusID++;
  m_value = 0;
  m_type = Generic;
  m_name.sprintf("Bus %ld", m_id - BUS_ROOT_ID);
}

Bus::~Bus()
{
}

Bus& Bus::operator=(Bus &b)
{
  if (this != &b)
    {
      m_id = b.id();
      m_value = b.value();
      m_type = b.type();
      m_name = QString(b.name());
    }

  return *this;
}

void Bus::setValue(unsigned long value)
{ 
  m_value = value;
  emit dataChanged((const Bus*) this);
}

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

  // Type
  t.setNum(m_type);
  s = QString("Type = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Value
  t.setNum(m_value);
  s = QString("Value = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());  
}

void Bus::createContents(QList <QString> &list)
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
      else if (*s == QString("Type"))
	{
	  m_type = (Type) list.next()->toInt();
	}
      else if (*s == QString("Value"))
	{
	  m_value = list.next()->toInt();
	}
    }
}
