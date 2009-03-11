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

#include <iostream>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcphysical.h"

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

QLCFixtureMode::QLCFixtureMode(const QLCFixtureMode* mode)
{
	if (mode != NULL)
		*this = *mode;
}

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef,
				const QDomElement* tag)
{
	m_fixtureDef = fixtureDef;

	if (tag != NULL)
		loadXML(tag);
}

QLCFixtureMode::~QLCFixtureMode()
{
	m_channels.clear();
}

QLCFixtureMode& QLCFixtureMode::operator=(const QLCFixtureMode& mode)
{
	if (this != &mode)
	{
		QListIterator <QLCChannel*> it(mode.m_channels);
		int i = 0;

		m_name = mode.m_name;
		m_physical = mode.m_physical;
		m_fixtureDef = mode.m_fixtureDef;

		m_channels.clear();
		while (it.hasNext() == true)
			insertChannel(it.next(), i++);
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
	QMutableListIterator <QLCChannel*> it(m_channels);
	while (it.hasNext() == true)
	{
		if (it.next() == channel)
		{
			/* Don't delete the channel since QLCFixtureModes
			   don't own them. QLCFixtureDefs do. */
			it.remove();
			return true;
		}
	}

	return false;
}

QLCChannel* QLCFixtureMode::channel(const QString& name)
{
	QListIterator <QLCChannel*> it(m_channels);
	QLCChannel* ch = NULL;

	while (it.hasNext() == true)
	{
		ch = it.next();
		if (ch->name() == name)
			return ch;
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
	QListIterator <QLCChannel*> it(m_channels);
	int i = 0;

	if (channel == NULL)
		return KChannelInvalid;

	while (it.hasNext() == true)
	{
		if (it.next()->name() == channel->name())
			return i;
		++i;
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

bool QLCFixtureMode::loadXML(const QDomElement* root)
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
			qDebug() << "Unknown Mode tag: " << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool QLCFixtureMode::saveXML(QDomDocument* doc, QDomElement* root)
{
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
	QListIterator <QLCChannel*> it(m_channels);
	while (it.hasNext() == true)
	{
		chtag = doc->createElement(KXMLQLCFixtureModeChannel);
		tag.appendChild(chtag);

		str.setNum(i++);
		chtag.setAttribute(KXMLQLCFixtureModeChannelNumber, str);

		text = doc->createTextNode(it.next()->name());
		chtag.appendChild(text);
	}

	return true;
}
