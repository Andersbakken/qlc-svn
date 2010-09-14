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

#include <QStringList>
#include <iostream>
#include <QString>
#include <QFile>
#include <QtXml>

#include "qlcchannel.h"
#include "qlccapability.h"

QLCChannel::QLCChannel()
{
	m_name = QString::null;
	m_group = QString(KQLCChannelGroupIntensity);
	m_controlByte = 0;
}

QLCChannel::QLCChannel(const QLCChannel* channel)
{
	m_name = QString::null;
	m_group = QString(KQLCChannelGroupIntensity);
	m_controlByte = 0;

	if (channel != NULL)
		*this = *channel;
}

QLCChannel::~QLCChannel()
{
	while (m_capabilities.isEmpty() == false)
		delete m_capabilities.takeFirst();
}

QLCChannel& QLCChannel::operator=(const QLCChannel& channel)
{
	if (this != &channel)
	{
		QListIterator<QLCCapability*> it(channel.m_capabilities);

		m_name = channel.m_name;
		m_group = channel.m_group;
		m_controlByte = channel.m_controlByte;

		/* Clear old capabilities */
		while (m_capabilities.isEmpty() == false)
			delete m_capabilities.takeFirst();

		/* Copy new capabilities from the other channel */
		while (it.hasNext() == true)
			m_capabilities.append(new QLCCapability(it.next()));
	}

	return *this;
}

/*****************************************************************************
 * Channel groups
 *****************************************************************************/
QStringList QLCChannel::groupList()
{
	QStringList list;

	list.append(KQLCChannelGroupBeam);
	list.append(KQLCChannelGroupColour);
	list.append(KQLCChannelGroupEffect);
	list.append(KQLCChannelGroupGobo);
	list.append(KQLCChannelGroupIntensity);
	list.append(KQLCChannelGroupMaintenance);
	list.append(KQLCChannelGroupNothing);
	list.append(KQLCChannelGroupPan);
	list.append(KQLCChannelGroupPrism);
	list.append(KQLCChannelGroupShutter);
	list.append(KQLCChannelGroupSpeed);
	list.append(KQLCChannelGroupTilt);

	return list;
}

int QLCChannel::groupToIndex(QString group)
{
	if (group == KQLCChannelGroupBeam)
		return 0;
	else if (group == KQLCChannelGroupColour)
		return 1;
	else if (group == KQLCChannelGroupEffect)
		return 2;
	else if (group == KQLCChannelGroupGobo)
		return 3;
	else if (group == KQLCChannelGroupIntensity)
		return 4;
	else if (group == KQLCChannelGroupMaintenance)
		return 5;
	else if (group == KQLCChannelGroupNothing)
		return 6;
	else if (group == KQLCChannelGroupPan)
		return 7;
	else if (group == KQLCChannelGroupPrism)
		return 8;
	else if (group == KQLCChannelGroupShutter)
		return 9;
	else if (group == KQLCChannelGroupSpeed)
		return 10;
	else if (group == KQLCChannelGroupTilt)
		return 11;
	else
		return -1;
}

QString QLCChannel::indexToGroup(int index)
{
	switch (index)
	{
	case 0: return KQLCChannelGroupBeam;
	case 1: return KQLCChannelGroupColour;
	case 2: return KQLCChannelGroupEffect;
	case 3: return KQLCChannelGroupGobo;
	case 4: return KQLCChannelGroupIntensity;
	case 5: return KQLCChannelGroupMaintenance;
	default: case 6: return KQLCChannelGroupNothing;
	case 7: return KQLCChannelGroupPan;
	case 8: return KQLCChannelGroupPrism;
	case 9: return KQLCChannelGroupShutter;
	case 10: return KQLCChannelGroupSpeed;
	case 11: return KQLCChannelGroupTilt;
	}
}

/*****************************************************************************
 * Capabilities
 *****************************************************************************/

QLCCapability* QLCChannel::searchCapability(uchar value) const
{
	QListIterator <QLCCapability*> it(m_capabilities);
	while (it.hasNext() == true)
	{
		QLCCapability* capability = it.next();
		if (capability->min() <= value && capability->max() >= value)
			return capability;
	}

	return NULL;
}

QLCCapability* QLCChannel::searchCapability(const QString& name,
					    bool exactMatch) const
{
	QListIterator <QLCCapability*> it(m_capabilities);
	while (it.hasNext() == true)
	{
		QLCCapability* capability = it.next();
		if (exactMatch == true && capability->name() == name)
			return capability;
		else if (exactMatch == false &&
			 capability->name().contains(name) == true)
			return capability;
	}

	return NULL;
}

bool QLCChannel::addCapability(QLCCapability* cap)
{
	Q_ASSERT(cap != NULL);

	/* Check for overlapping values */
	foreach (QLCCapability* another, m_capabilities)
	{
		if (another->overlaps(cap) == true)
			return false;
	}

	m_capabilities.append(cap);
	return true;
}

bool QLCChannel::removeCapability(QLCCapability* cap)
{
	Q_ASSERT(cap != NULL);

	QMutableListIterator <QLCCapability*> it(m_capabilities);
	while (it.hasNext() == true)
	{
		if (it.next() == cap)
		{
			it.remove();
			delete cap;
			return true;
		}
	}

	return false;
}

static bool capsort(const QLCCapability* cap1, const QLCCapability* cap2)
{
	return (*cap1) < (*cap2);
}

void QLCChannel::sortCapabilities()
{
	qSort(m_capabilities.begin(), m_capabilities.end(), capsort);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool QLCChannel::saveXML(QDomDocument* doc, QDomElement* root) const
{
	QDomElement chtag;
	QDomElement grptag;
	QDomText text;
	QString str;

	Q_ASSERT(doc);
	Q_ASSERT(root);

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
	QListIterator <QLCCapability*> it(m_capabilities);
	while (it.hasNext() == true)
		it.next()->saveXML(doc, &chtag);

	return true;
}

bool QLCChannel::loadXML(const QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCChannel)
	{
		qWarning() << "Channel node not found.";
		return false;
	}

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
			/* Create a new capability and attempt to load it */
			QLCCapability* cap = new QLCCapability();
			if (cap->loadXML(&tag) == true)
			{
				/* Loading succeeded */
				if (addCapability(cap) == false)
				{
					/* Value overlaps with existing value */
					delete cap;
				}
			}
			else
			{
				/* Loading failed */
				delete cap;
			}
		}
		else if (tag.tagName() == KXMLQLCChannelGroup)
		{
			str = tag.attribute(KXMLQLCChannelGroupByte);
			setControlByte(str.toInt());
			setGroup(tag.text());
		}
		else
		{
			qDebug() << "Unknown Channel tag: " << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}
