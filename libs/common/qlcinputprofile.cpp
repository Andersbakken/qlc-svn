/*
  Q Light Controller
  qlcinputprofile.cpp

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
#include <QMap>

#include <common/qlcinputchannel.h>
#include <common/qlcinputprofile.h>
#include <common/qlcfile.h>

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputProfile::QLCInputProfile()
{
}

QLCInputProfile::QLCInputProfile(const QLCInputProfile& profile)
{
	/* Copy contents with operator=() */
	*this = profile;
}

QLCInputProfile::~QLCInputProfile()
{
	/* Delete existing channels but leave the pointers there */
	QMutableMapIterator <t_input_channel,QLCInputChannel*> it(m_channels);
	while (it.hasNext() == true)
		delete it.next().value();

	/* Clear the list of freed pointers */
	m_channels.clear();
}

QLCInputProfile& QLCInputProfile::operator=(const QLCInputProfile& profile)
{
	if (this != &profile)
	{
		m_manufacturer = profile.m_manufacturer;
		m_model = profile.m_model;
		m_path = profile.m_path;

		/* Delete existing channels */
		QMutableMapIterator <t_input_channel,QLCInputChannel*>
			old_it(m_channels);
		while (old_it.hasNext() == true)
			delete old_it.next().value();

		/* Clear the list of freed pointers */
		m_channels.clear();

		/* Copy the other profile's channels */
		QMapIterator <t_input_channel,QLCInputChannel*>
			it(profile.m_channels);
		while (it.hasNext() == true)
			addChannel(new QLCInputChannel(*(it.next().value())));
	}

	return *this;
}

/****************************************************************************
 * profile information
 ****************************************************************************/

void QLCInputProfile::setManufacturer(const QString& manufacturer)
{
	m_manufacturer = manufacturer;
}

void QLCInputProfile::setModel(const QString& model)
{
	m_model = model;
}

QString QLCInputProfile::name() const
{
	return QString("%1 - %2").arg(m_manufacturer).arg(m_model);
}

QString QLCInputProfile::path() const
{
	return m_path;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCInputProfile::addChannel(QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);

	/* Remove & delete the previous mapping if such exists */
	if (m_channels.contains(ich->channel()) == true)
		removeChannel(ich->channel());

	m_channels.insert(ich->channel(), ich);
}

void QLCInputProfile::removeChannel(QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);

	/* Just remove the channel from the list, but don't delete */
	if (m_channels.contains(ich->channel()) == true)
		m_channels.remove(ich->channel());
}

void QLCInputProfile::removeChannel(t_input_channel channel)
{
	if (m_channels.contains(channel) == true)
	{
		QLCInputChannel* ich = m_channels.take(channel);
		delete ich;

		/* Also invalidate mapping for the removed channel */
		setMapping(channel, KInputChannelInvalid);
	}
}

QLCInputChannel* QLCInputProfile::channel(t_input_channel channel) const
{
	if (m_channels.contains(channel) == true)
		return m_channels[channel];
	else
		return NULL;
}

QString QLCInputProfile::channelName(t_input_channel channel) const
{
	if (m_channels.contains(channel) == true)
		return m_channels[channel]->name();
	else
		return QString::null;
}

/****************************************************************************
 * Channel mapping
 ****************************************************************************/

void QLCInputProfile::setMapping(t_input_channel from, t_input_channel to)
{
	if (from == KInputChannelInvalid)
		return;

	/* Remove mapping if to == KInputChannelInvalid. Otherwise assign. */
	if (to == KInputChannelInvalid)
		m_mapping.remove(from);
	else
		m_mapping[from] = to;
}

t_input_channel QLCInputProfile::mapping(t_input_channel from) const
{
	if (m_mapping.contains(from) == true)
		return m_mapping[from];
	else
		return KInputChannelInvalid;
}

t_input_channel QLCInputProfile::reverseMapping(t_input_channel to) const
{
	QHashIterator <t_input_channel, t_input_channel> it(m_mapping);
	while (it.hasNext() == true)
	{
		it.next();
		if (it.value() == to)
			return it.key();
	}

	return KInputChannelInvalid;
}

const QLCInputChannel* QLCInputProfile::mappedChannel(t_input_channel to) const
{
	t_input_channel from = reverseMapping(to);
	if (from == KInputChannelInvalid)
		return NULL;
	else
		return channel(from);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

QLCInputProfile* QLCInputProfile::loader(const QString& path)
{
	QLCInputProfile* profile = NULL;
	QDomDocument* doc = NULL;

	if (QLCFile::readXML(path, &doc) != QFile::NoError)
		return false;

	profile = new QLCInputProfile();
	if (profile->loadXML(doc) == false)
	{
		delete profile;
		profile = NULL;
	}
	else
	{
		profile->m_path = path;
	}

	delete doc;
	return profile;
}

bool QLCInputProfile::loadXML(const QDomDocument* doc)
{
	t_input_channel from;
	t_input_channel to;
	QDomElement root;
	QDomElement tag;
	QDomNode node;
	QString str;

	Q_ASSERT(doc != NULL);

	root = doc->documentElement();
	if (root.tagName() == KXMLQLCInputProfile)
	{
		node = root.firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();
			if (tag.tagName() == KXMLQLCCreator)
			{
				/* Ignore */
			}
			if (tag.tagName() == KXMLQLCInputProfileManufacturer)
			{
				setManufacturer(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputProfileModel)
			{
				setModel(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputChannel)
			{
				QLCInputChannel* ich = new QLCInputChannel();
				if (ich->loadXML(&tag) == true)
					addChannel(ich);
				else
					delete ich;
			}
			else if (tag.tagName() == KXMLQLCInputProfileMap)
			{
				str = tag.attribute(KXMLQLCInputProfileMapFrom);
				if (str.isEmpty() == false)
					from = str.toInt();
				else
					from = KInputChannelInvalid;

				str = tag.attribute(KXMLQLCInputProfileMapTo);
				if (str.isEmpty() == false)
					to = str.toInt();
				else
					to = KInputChannelInvalid;

				setMapping(from, to);
			}

			node = node.nextSibling();
		}
	}
	else
	{
		qDebug() << "Input profile node not found in file!";
	}

	return true;
}

bool QLCInputProfile::saveXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	bool retval = false;

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
		return false;

	if (QLCFile::getXMLHeader(KXMLQLCInputProfile, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();

		/* Manufacturer */
		tag = doc->createElement(KXMLQLCInputProfileManufacturer);
		root.appendChild(tag);
		text = doc->createTextNode(m_manufacturer);
		tag.appendChild(text);

		/* Model */
		tag = doc->createElement(KXMLQLCInputProfileModel);
		root.appendChild(tag);
		text = doc->createTextNode(m_model);
		tag.appendChild(text);

		/* Write channels to the document */
		QMapIterator <t_input_channel, QLCInputChannel*> it(m_channels);
		while (it.hasNext() == true)
			it.next().value()->saveXML(doc, &root);

		/* Save channel mappings */
		saveXMLMappings(doc, &root);

		/* Write the document into the stream */
		m_path = fileName;
		stream << doc->toString() << "\n";

		/* Delete the XML document */
		delete doc;

		retval = true;
	}
	else
	{
		retval = false;
	}

	file.close();

	return retval;
}

bool QLCInputProfile::saveXMLMappings(QDomDocument* doc, QDomElement* root) const
{
	QDomElement tag;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCInputProfile)
	{
		qWarning() << "Not an Input profile node!";
		return false;
	}
	
	QHashIterator <t_input_channel, t_input_channel> it(m_mapping);
	while (it.hasNext() == true)
	{
		it.next();
		
		tag = doc->createElement(KXMLQLCInputProfileMap);
		tag.setAttribute(KXMLQLCInputProfileMapFrom,
				 QString("%1").arg(it.key()));
		tag.setAttribute(KXMLQLCInputProfileMapTo,
				 QString("%1").arg(it.value()));
		root->appendChild(tag);
	}

	return true;
}
