/*
  Q Light Controller
  qlcfixturemode.cpp
  
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

#include <qstring.h>
#include <qdom.h>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/qlcphysical.h"

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef)
{
	m_fixtureDef = fixtureDef;
	
	m_physical.setBulbType(QString::null);
	m_physical.setBulbLumens(0);
	m_physical.setBulbColourTemperature(0);
	
	m_physical.setWeight(0);
	m_physical.setWidth(0);
	m_physical.setHeight(0);
	m_physical.setDepth(0);
	
	m_physical.setLensName("Other");
	m_physical.setLensDegreesMin(0);
	m_physical.setLensDegreesMax(0);
	
	m_physical.setFocusType("Fixed");
	m_physical.setFocusPanMax(0);
	m_physical.setFocusTiltMax(0);
}

QLCFixtureMode::QLCFixtureMode(QLCFixtureMode* mode)
{
	if (mode != NULL)
		*this = *mode;
}

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef, QDomElement* tag)
{
	m_fixtureDef = fixtureDef;
	
	if (tag != NULL)
		loadXML(tag);
}

QLCFixtureMode::~QLCFixtureMode()
{
	m_channels.clear();
}

QLCFixtureMode& QLCFixtureMode::operator=(QLCFixtureMode& mode)
{
	if (this != &mode)
	{
		QLCChannel* ch = NULL;
		QPtrListIterator<QLCChannel> it(mode.m_channels);
		int i = 0;
		
		m_name = mode.name();
		m_physical = mode.physical();
		m_fixtureDef = mode.fixtureDef();
		
		m_channels.clear();
		while ( (ch = it.current()) != 0 )
		{
			++it;
			insertChannel(ch, i++);
		}
	}

	return *this;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCFixtureMode::insertChannel(QLCChannel* channel, t_channel index)
{
	m_channels.insert(index, channel);
}

bool QLCFixtureMode::removeChannel(QLCChannel* channel)
{
	for (QLCChannel* ch = m_channels.first(); ch != NULL;
	     ch = m_channels.next())
	{
		if (ch == channel)
		{
			m_channels.take();
			return true;
		}
	}

	return false;
}

QLCChannel* QLCFixtureMode::searchChannel(const QString& name)
{
	QPtrListIterator<QLCChannel> it(m_channels);
	QLCChannel* ch = NULL;
	
	while ( (ch = it.current()) != 0 )
	{
		if (ch->name() == name)
			return ch;
		++it;
	}
	
	return NULL;
}

QLCChannel* QLCFixtureMode::channel(t_channel ch)
{
	if (ch > m_channels.count())
		return NULL;
	else
		return m_channels.at(ch);
}

t_channel QLCFixtureMode::channelNumber(QLCChannel* channel)
{
	QPtrListIterator<QLCChannel> it(m_channels);
	QLCChannel* ch = NULL;
	int i = 0;
	
	if (channel == NULL)
		return KChannelInvalid;
	
	while ( (ch = it.current()) != 0 )
	{
		if (ch->name() == channel->name())
			return i;
		++i;
		++it;
	}

	return KChannelInvalid;
}

void QLCFixtureMode::setPhysical(const QLCPhysical &physical)
{
	m_physical = physical;
}

const QLCPhysical QLCFixtureMode::physical()
{
	return m_physical;
}

bool QLCFixtureMode::loadXML(QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	QString ch;

	/* Get channel name */
	str = root->attribute(KXMLQLCFixtureModeName);
	if (str == QString::null)
		return false;
	else
		setName(str);
	
	/* Subtags */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFixtureModeChannel)
		{
			str = tag.attribute(KXMLQLCFixtureModeChannelNumber);
			insertChannel(m_fixtureDef->channel(tag.text()),
				      str.toInt());
		}
		else if (tag.tagName() == KXMLQLCPhysical)
		{
			QLCPhysical physical;
			physical.loadXML(&tag);
			setPhysical(physical);
		}
		else
		{
			qDebug("Unknown Mode tag: %s",
				(const char*) tag.tagName());
		}

		node = node.nextSibling();
	}

	return true;
}

bool QLCFixtureMode::saveXML(QDomDocument* doc, QDomElement* root)
{
	QPtrListIterator<QLCChannel> it(m_channels);
	QLCChannel* ch = NULL;
	QDomElement tag;
	QDomElement chtag;
	QDomText text;
	QString str;
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* Mode entry */
	tag = doc->createElement(KXMLQLCFixtureMode);
	tag.setAttribute(KXMLQLCFixtureModeName, m_name);
	root->appendChild(tag);

	m_physical.saveXML(doc, &tag);
	
	/* Channels */
	while ( (ch = it.current()) != 0 )
	{
		chtag = doc->createElement(KXMLQLCFixtureModeChannel);
		tag.appendChild(chtag);

		str.setNum(i++);
		chtag.setAttribute(KXMLQLCFixtureModeChannelNumber, str);

		text = doc->createTextNode(ch->name());
		chtag.appendChild(text);
		
		++it;
	}

	return true;
}
