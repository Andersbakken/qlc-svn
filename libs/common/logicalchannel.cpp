/*
  Q Light Controller
  logicalchannel.cpp
  
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

#include <qstring.h>
#include <qfile.h>

#include "logicalchannel.h"
#include "capability.h"

LogicalChannel::LogicalChannel()
{
	m_channel = 0;
	m_name = QString::null;
}

LogicalChannel::LogicalChannel(LogicalChannel* lc)
{
	m_channel = lc->channel();
	m_name = QString(lc->name());

	for (Capability* c = lc->capabilities()->first(); c != NULL;
	     c = lc->capabilities()->next())
	{
		m_capabilities.append(new Capability(c));
	}
}

LogicalChannel::~LogicalChannel()
{
	while (m_capabilities.isEmpty() == false)
	{
		Capability* c = m_capabilities.take(0);
		delete c;
	}
}


Capability* LogicalChannel::searchCapability(t_value value)
{
	for (Capability* c = m_capabilities.first(); c != NULL; 
	     c = m_capabilities.next())
	{
		if (c->lo() <= value && c->hi() >= value)
		{
			return c;
		}
	}

	return NULL;
}

Capability* LogicalChannel::searchCapability(QString name)
{
	for (Capability* c = m_capabilities.first(); c != NULL;
	     c = m_capabilities.next())
	{
		if (c->name() == name)
		{
			return c;
		}
	}

	return NULL;
}

void LogicalChannel::saveToFile(QFile &file)
{
	QString s;
	QString t;

	s = QString("Entry = Channel\n");
	file.writeBlock((const char*) s, s.length());
	
	t.setNum(m_channel);
	s = QString("Number = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	s = QString("Name = ") + m_name + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	s = QString("# Capability Entries\n");
	file.writeBlock((const char*) s, s.length());
	
	for (Capability* c = m_capabilities.first(); c != NULL;
	     c = m_capabilities.next())
	{
		c->saveToFile(file);
	}
}

