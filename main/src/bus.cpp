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

t_bus_id Bus::m_nextBusID = KBusIDMin;
Bus* Bus::m_defaultFadeBus = new Bus(KBusIDDefaultFade);
Bus* Bus::m_defaultHoldBus = new Bus(KBusIDDefaultHold);

//
// Constructor
//
Bus::Bus(t_bus_id id) : QObject()
{
  if (id == KBusIDDefaultFade)
    {
      m_id = id;
      m_name = QString("Default Fade Time");
      m_value = 64;
      m_static = true;
    }
  else if (id == KBusIDDefaultHold)
    {
      m_id = id;
      m_name = QString("Default Hold Time");
      m_value = 0;
      m_static = true;
    }
  else
    {
      m_id = m_nextBusID;
      m_nextBusID++;
      m_value = 0;
      m_name.sprintf("Bus %d", m_id);
      m_static = false;
    }
}


//
// Destructor
//
Bus::~Bus()
{
}


//
// (Shallow?) Copy
//
Bus& Bus::operator=(Bus &b)
{
  if (this != &b)
    {
      m_id = b.id();
      m_value = b.value();
      m_name = QString(b.name());
    }

  return *this;
}


//
// Info string displayed in device manager
//
QString Bus::infoText()
{
  QString t;

  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Bus Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
  str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
  str += name() + QString("</FONT></TD></TR></TABLE>");
  str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
  str += QString("</TABLE>");

  if (m_id == KBusIDDefaultFade)
    {
      str += QString("<P><B>") + name() + 
	QString("</B> is assigned by default to all ");
      str += QString("<B>scenes</B>. You can use it ");
      str += QString("to set the <I>fade time</I>.</P>");
      str += QString("<H3>NOTE</H3>");
      str += QString("<P><I>This bus cannot be removed</I>.</P>");
    }

  if (m_id == KBusIDDefaultHold)
    {
      str += QString("<P><B>") + name() + 
	QString("</B> is assigned by default to all ");
      str += QString("<B>chasers</B>. You can use this to set ");
      str += QString("<I>hold time</I> between individual <I>scenes</I>.</P>");
      str += QString("<H3>NOTE</H3>");
      str += QString("<P><I>This bus cannot be removed</I>.</P>");
    }

  str += QString("</BODY></HTML>");

  return str;
}


//
// Set bus name
//
void Bus::setName(QString name)
{
  if (m_static)
    {
      return;
    }
  else
    {
      m_name = name;
    }
}


//
// Set bus value
//
void Bus::setValue(t_bus_value value)
{
  m_value = value;
  emit dataChanged((const Bus*) this);
}


//
// Save bus to a file
//
void Bus::saveToFile(QFile &file)
{
  QString s;
  QString t;

  if (m_id == KBusIDDefaultHold || m_id == KBusIDDefaultFade)
    {
      // Don't save default buses
      return;
    }

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
