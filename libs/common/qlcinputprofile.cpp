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
		{
			it.next();

			t_input_channel number = it.key();
			QLCInputChannel* ich = it.value();

			insertChannel(number, new QLCInputChannel(*ich));
		}
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
	return QString("%1 %2").arg(m_manufacturer).arg(m_model);
}

QString QLCInputProfile::path() const
{
	return m_path;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCInputProfile::insertChannel(t_input_channel channel,
				    QLCInputChannel* ich)
{
	if (ich != NULL && m_channels.contains(channel) == false)
	{
		m_channels.insert(channel, ich);
		return true;
	}
	else
	{
		return false;
	}
}

bool QLCInputProfile::removeChannel(t_input_channel channel)
{
	if (m_channels.contains(channel) == true)
	{
		QLCInputChannel* ich = m_channels.take(channel);
		Q_ASSERT(ich != NULL);
		delete ich;
		return true;
	}
	else
	{
		return false;
	}
}

bool QLCInputProfile::remapChannel(QLCInputChannel* ich, t_input_channel number)
{
	if (ich == NULL)
		return false;

	t_input_channel old = channelNumber(ich);
	if (old != KInputChannelInvalid && m_channels.contains(number) == false)
	{
		m_channels.take(old);
		insertChannel(number, ich);
		return true;
	}
	else
	{
		return false;
	}
}

QLCInputChannel* QLCInputProfile::channel(t_input_channel channel) const
{
	if (m_channels.contains(channel) == true)
		return m_channels[channel];
	else
		return NULL;
}

t_input_channel QLCInputProfile::channelNumber(
					const QLCInputChannel* channel) const
{
	if (channel == NULL)
		return KInputChannelInvalid;

	QMapIterator <t_input_channel,QLCInputChannel*> it(m_channels);
	while (it.hasNext() == true)
	{
		it.next();
		if (it.value() == channel)
			return it.key();
	}

	return KInputChannelInvalid;
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
	QLCInputChannel* ich;
	t_input_channel ch;
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
				str = tag.attribute(KXMLQLCInputChannelNumber);
				if (str.isEmpty() == false)
				{
					ch = str.toInt();
					ich = new QLCInputChannel();
					if (ich->loadXML(&tag) == true)
						insertChannel(ch, ich);
					else
						delete ich;
				}
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
		{
			it.next();
			it.value()->saveXML(doc, &root, it.key());
		}

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
