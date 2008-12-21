/*
  Q Light Controller
  qlcinputdevice.cpp

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
#include <common/qlcinputdevice.h>
#include <common/qlcfile.h>

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputDevice::QLCInputDevice()
{
}

QLCInputDevice::QLCInputDevice(const QLCInputDevice& device)
{
	/* Copy contents with operator=() */
	*this = device;
}

QLCInputDevice::~QLCInputDevice()
{
	/* Delete existing channels but leave the pointers there */
	QMutableMapIterator <t_input_channel,QLCInputChannel*> it(m_channels);
	while (it.hasNext() == true)
		delete it.next().value();

	/* Clear the list of freed pointers */
	m_channels.clear();
}

QLCInputDevice& QLCInputDevice::operator=(const QLCInputDevice& device)
{
	if (this != &device)
	{
		m_manufacturer = device.m_manufacturer;
		m_model = device.m_model;
		m_path = device.m_path;

		/* Delete existing channels */
		QMutableMapIterator <t_input_channel,QLCInputChannel*>
			old_it(m_channels);
		while (old_it.hasNext() == true)
			delete old_it.next().value();

		/* Clear the list of freed pointers */
		m_channels.clear();

		/* Copy the other device's channels */
		QMapIterator <t_input_channel,QLCInputChannel*>
			it(device.m_channels);
		while (it.hasNext() == true)
			addChannel(new QLCInputChannel(*(it.next().value())));
	}

	return *this;
}

/****************************************************************************
 * Device information
 ****************************************************************************/

void QLCInputDevice::setManufacturer(const QString& manufacturer)
{
	m_manufacturer = manufacturer;
}

void QLCInputDevice::setModel(const QString& model)
{
	m_model = model;
}

QString QLCInputDevice::name() const
{
	return QString("%1 - %2").arg(m_manufacturer).arg(m_model);
}

QString QLCInputDevice::path() const
{
	return m_path;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCInputDevice::addChannel(QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);

	/* Remove & delete the previous mapping if such exists */
	if (m_channels.contains(ich->channel()) == true)
		removeChannel(ich->channel());

	m_channels.insert(ich->channel(), ich);
}

void QLCInputDevice::removeChannel(QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);

	/* Just remove the channel from the list, but don't delete */
	if (m_channels.contains(ich->channel()) == true)
		m_channels.remove(ich->channel());
}

void QLCInputDevice::removeChannel(t_input_channel channel)
{
	if (m_channels.contains(channel) == true)
	{
		QLCInputChannel* ich = m_channels.take(channel);
		delete ich;
	}
}

QLCInputChannel* QLCInputDevice::channel(t_input_channel channel)
{
	if (m_channels.contains(channel) == true)
		return m_channels[channel];
	else
		return NULL;
}

QString QLCInputDevice::channelName(t_input_channel channel)
{
	if (m_channels.contains(channel) == true)
		return m_channels[channel]->name();
	else
		return QString::null;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

QLCInputDevice* QLCInputDevice::loader(const QString& path)
{
	QLCInputDevice* device = NULL;
	QDomDocument* doc = NULL;

	if (QLCFile::readXML(path, &doc) == false)
		return false;
	Q_ASSERT(doc != NULL);

	device = new QLCInputDevice();
	if (device->loadXML(doc) == false)
	{
		delete device;
		device = NULL;
	}
	else
	{
		device->m_path = path;
	}

	delete doc;
	return device;
}

bool QLCInputDevice::loadXML(QDomDocument* doc)
{
	QDomElement root;
	QDomElement tag;
	QDomNode node;

	Q_ASSERT(doc != NULL);

	root = doc->documentElement();
	if (root.tagName() == KXMLQLCInputDevice)
	{
		node = root.firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();
			if (tag.tagName() == KXMLQLCCreator)
			{
				/* Ignore */
			}
			if (tag.tagName() == KXMLQLCInputDeviceManufacturer)
			{
				setManufacturer(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputDeviceModel)
			{
				setModel(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputChannel)
			{
				QLCInputChannel* ich = new QLCInputChannel();
				if (ich->loadXML(doc, &tag) == true)
					addChannel(ich);
				else
					delete ich;
			}

			node = node.nextSibling();
		}
	}
	else
	{
		qDebug() << "Input device node not found in file!";
	}

	return true;
}

bool QLCInputDevice::saveXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	bool retval = false;

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
		return false;

	if (QLCFile::getXMLHeader(KXMLQLCInputDevice, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();

		/* Manufacturer */
		tag = doc->createElement(KXMLQLCInputDeviceManufacturer);
		root.appendChild(tag);
		text = doc->createTextNode(m_manufacturer);
		tag.appendChild(text);

		/* Model */
		tag = doc->createElement(KXMLQLCInputDeviceModel);
		root.appendChild(tag);
		text = doc->createTextNode(m_model);
		tag.appendChild(text);

		/* Write channels to the document */
		QMapIterator <t_input_channel, QLCInputChannel*> it(m_channels);
		while (it.hasNext() == true)
			it.next().value()->saveXML(doc, &root);

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
