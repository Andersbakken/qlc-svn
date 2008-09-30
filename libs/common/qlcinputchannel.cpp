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
	m_channel = 0;
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
		m_channel = channel.m_channel;
	}

	return *this;
}
	
/****************************************************************************
 * Channel number
 ****************************************************************************/

void QLCInputChannel::setChannel(t_input_channel channel)
{
	m_channel = channel;
}

/****************************************************************************
 * Type
 ****************************************************************************/

void QLCInputChannel::setType(Type type)
{
	m_type = type;
}

QString QLCInputChannel::typeToString(Type type)
{
	switch (type)
	{
	default:
	case AbsoluteFader:
		return QString(KXMLQLCInputChannelAbsoluteFader);
	case RelativeFader:
		return QString(KXMLQLCInputChannelRelativeFader);
	case Button:
		return QString(KXMLQLCInputChannelButton);
	}
}

QLCInputChannel::Type QLCInputChannel::stringToType(const QString& type)
{
	if (type == KXMLQLCInputChannelButton)
		return Button;
	else if (type == KXMLQLCInputChannelRelativeFader)
		return RelativeFader;
	else
		return AbsoluteFader;
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

bool QLCInputChannel::loadXML(QDomDocument* doc, QDomElement* root)
{
	return FALSE;
}

bool QLCInputChannel::saveXML(QDomDocument* doc, QDomElement* dev_root)
{
	return FALSE;
}
