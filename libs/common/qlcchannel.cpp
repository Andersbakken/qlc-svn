/*
  Q Light Controller
  qlcchannel.cpp
  
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
#include <qfile.h>
#include <qdom.h>
#include <qstringlist.h>
#include <assert.h>

#include "common/qlcchannel.h"
#include "common/qlccapability.h"

// Old headers that will be removed in the future
#include "common/logicalchannel.h"
#include "common/capability.h"

QLCChannel::QLCChannel()
{
	m_name = QString::null;
	m_group = QString(KIntensityGroup);
	m_controlByte = 0;
}

QLCChannel::QLCChannel(QLCChannel* channel)
{
	m_name = QString::null;
	m_group = QString(KIntensityGroup);
	m_controlByte = 0;

	if (channel != NULL)
		*this = *channel;
}

QLCChannel::QLCChannel(QDomElement* tag)
{
	m_name = QString::null;
	m_group = QString(KIntensityGroup);
	m_controlByte = 0;

	loadXML(tag);
}

QLCChannel::QLCChannel(LogicalChannel* lch)
{
	Capability* cap = NULL;
	QPtrListIterator<Capability> it(*lch->capabilities());
	
	m_name = lch->name();
	m_group = QString(KIntensityGroup);
	m_controlByte = 0;
	
	while ((cap = it.current()) != 0)
	{
		addCapability(new QLCCapability(cap));
		++it;
	}
}

QLCChannel::~QLCChannel()
{
	while (m_capabilities.isEmpty() == false)
		delete m_capabilities.take(0);
}

QLCChannel& QLCChannel::operator=(QLCChannel& channel)
{
	if (this != &channel)
	{
		QPtrListIterator<QLCCapability> it(channel.m_capabilities);
		QLCCapability* cap = NULL;

		m_name = channel.m_name;
		m_group = channel.m_group;

		/* Clear old capabilities */
		while (m_capabilities.isEmpty() == false)
			delete m_capabilities.take(0);
		
		/* Copy new capabilities from the other channel */
		while ((cap = it.current()) != 0)
		{
			++it;
			m_capabilities.append(new QLCCapability(cap));
		}
	}

	return *this;
}

/*****************************************************************************
 * Channel groups
 *****************************************************************************/
QStringList QLCChannel::groupList()
{
	QStringList list;

	list.append("Intensity");
	list.append("Colour");
	list.append("Gobo");
	list.append("Speed");
	list.append("Effect");
	list.append("Beam");
	list.append("Pan");
	list.append("Tilt");

	return list;
}

/*****************************************************************************
 * Capabilities
 *****************************************************************************/

QLCCapability* QLCChannel::searchCapability(t_value value)
{
	for (QLCCapability* c = m_capabilities.first(); c != NULL; 
	     c = m_capabilities.next())
	{
		if (c->min() <= value && c->max() >= value)
		{
			return c;
		}
	}

	return NULL;
}

QLCCapability* QLCChannel::searchCapability(QString name)
{
	for (QLCCapability* c = m_capabilities.first(); c != NULL;
	     c = m_capabilities.next())
	{
		if (c->name() == name)
		{
			return c;
		}
	}

	return NULL;
}

bool QLCChannel::addCapability(QLCCapability* cap)
{
	QPtrListIterator<QLCCapability> it(m_capabilities);
	QLCCapability* temp = NULL;
	
	Q_ASSERT(cap != NULL);
	
	/* Check for overlapping values */
	while ( (temp = it.current()) != 0 )
	{
		if ((temp->min() <= cap->min() && temp->max() > cap->min()) ||
		    (temp->min() < cap->max() && temp->max() >= cap->max()) ||
		    (temp->min() >= cap->min() && temp->max() <= cap->max()))
		{
			return false;
		}
		
		++it;
	}
	
	m_capabilities.append(cap);
	return true;
}

bool QLCChannel::removeCapability(QLCCapability* cap)
{
	Q_ASSERT(cap != NULL);

	for (QLCCapability* c = m_capabilities.first(); c != NULL;
	     c = m_capabilities.next())
	{
		if (c == cap)
		{
			delete m_capabilities.take();
			return true;
		}
	}

	return false;
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

void QLCChannel::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement chtag;
	QDomElement grptag;
	QDomText text;
	QString str;

	assert(doc);
	assert(root);

	/* Channel entry */
	chtag = doc->createElement(KXMLQLCChannel);
	chtag.setAttribute(KXMLQLCChannelName, m_name);
	root->appendChild(chtag);

	/* Group */
	grptag = doc->createElement(KXMLQLCChannelGroup);
	text = doc->createTextNode(m_group);
	grptag.appendChild(text);
	
	/* Group control byte */
	str.setNum(m_controlByte);
	grptag.setAttribute(KXMLQLCChannelGroupByte, str);

	chtag.appendChild(grptag);

	/* Capabilities */
	for (QLCCapability* c = m_capabilities.first(); c != NULL;
	     c = m_capabilities.next())
	{
		c->saveXML(doc, &chtag);
	}
}

bool QLCChannel::loadXML(QDomElement* root)
{
	QLCCapability* cap = NULL;
	QDomNode node;
	QDomElement tag;
	QString str;

	ASSERT(root != NULL);

	/* Get channel name */
	str = root->attribute(KXMLQLCChannelName);
	if (str == QString::null)
		return false;
	else
		setName(str);
	
	/* Subtags */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCCapability)
		{
			cap = new QLCCapability(&tag);
			addCapability(cap);
		}
		else if (tag.tagName() == KXMLQLCChannelGroup)
		{
			str = tag.attribute(KXMLQLCChannelGroupByte);
			setControlByte(str.toInt());
			setGroup(tag.text());
		}
		else
		{
			qDebug("Unknown Channel tag: %s",
				(const char*) tag.tagName());
		}

		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Groups 
 *****************************************************************************/

void QLCChannel::groups(QStringList& list)
{
	list.clear();
	list.append(KIntensityGroup);
	list.append(KColourGroup);
	list.append(KGoboGroup);
	list.append(KSpeedGroup);
	list.append(KEffectGroup);
	list.append(KBeamGroup);
	list.append(KPanGroup);
	list.append(KTiltGroup);
}

