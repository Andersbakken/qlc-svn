/*
  Q Light Controller
  qlcinputchannel.cpp
  
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

#include <QString>
#include <QtXml>

#include "qlcinputchannel.h"
#include "qlcinputdevice.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputChannel::QLCInputChannel(QLCInputDevice* parent) : QObject(parent)
{
	Q_ASSERT(parent != NULL);
}

QLCInputChannel::~QLCInputChannel()
{
}

QLCInputChannel& QLCInputChannel::operator=(QLCInputChannel& channel)
{
	if (this != &channel)
	{
		m_name = channel.m_name;
		m_type = channel.m_type;
	}

	return *this;
}
	
/****************************************************************************
 * Type
 ****************************************************************************/

void QLCInputChannel::setType(Type type)
{
	m_type = type;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void QLCInputChannel::setName(const QString& name)
{
	m_name = name;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

void QLCInputChannel::loadXML(QDomDocument* doc, QDomElement* root)
{
}

void QLCInputChannel::saveXML(QDomDocument* doc, QDomElement* dev_root)
{
}
