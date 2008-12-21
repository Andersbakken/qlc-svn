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

QLCInputChannel::QLCInputChannel()
{
	m_channel = 0;
	m_type = NoType;
}

QLCInputChannel::QLCInputChannel(const QLCInputChannel& channel)
{
	*this = channel;
}

QLCInputChannel::~QLCInputChannel()
{
}

QLCInputChannel& QLCInputChannel::operator=(const QLCInputChannel& channel)
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
	QString str;

	switch (type)
	{
	case Button:
		str = QString(KXMLQLCInputChannelButton);
		break;
	case Knob:
		str = QString(KXMLQLCInputChannelKnob);
		break;
	default:
	case Slider:
		str = QString(KXMLQLCInputChannelSlider);
		break;
	}

	return str;
}

QLCInputChannel::Type QLCInputChannel::stringToType(const QString& type)
{
	if (type == KXMLQLCInputChannelButton)
		return Button;
	else if (type == KXMLQLCInputChannelKnob)
		return Knob;
	else
		return Slider;
}

QStringList QLCInputChannel::types()
{
	QStringList list;
	list << KXMLQLCInputChannelSlider;
	list << KXMLQLCInputChannelKnob;
	list << KXMLQLCInputChannelButton;
	return list;
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
	QDomElement tag;
	QDomNode node;

	Q_UNUSED(doc);
	Q_ASSERT(root != NULL);

	/* Verify that the tag contains an input channel */
	if (root->tagName() != KXMLQLCInputChannel)
	{
		qWarning() << "Channel node not found";
		return false;
	}

	/* Get the channel number */
	setChannel(root->attribute(KXMLQLCInputChannelNumber).toInt());

	/* Go thru all sub tags */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCInputChannelName)
		{
			setName(tag.text());
		}
		else if (tag.tagName() == KXMLQLCInputChannelType)
		{
			setType(stringToType(tag.text()));
		}
		else
		{
			qDebug() << "Unknown input channel tag"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool QLCInputChannel::saveXML(QDomDocument* doc, QDomElement* root) const
{
	QDomElement subtag;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* The channel tag */
	tag = doc->createElement(KXMLQLCInputChannel);
	root->appendChild(tag);

	/* Channel number attribute */
	tag.setAttribute(KXMLQLCInputChannelNumber,
			 QString("%1").arg(m_channel));

	/* Name */
	subtag = doc->createElement(KXMLQLCInputChannelName);
	tag.appendChild(subtag);
	text = doc->createTextNode(m_name);
	subtag.appendChild(text);

	/* Type */
	subtag = doc->createElement(KXMLQLCInputChannelType);
	tag.appendChild(subtag);
	text = doc->createTextNode(typeToString(m_type));
	subtag.appendChild(text);

	return true;
}
