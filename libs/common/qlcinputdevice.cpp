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

QLCInputDevice::QLCInputDevice(QObject* parent) : QObject(parent)
{
}

QLCInputDevice::QLCInputDevice(const QLCInputDevice& device)
	: QObject (device.parent())
{
	setManufacturer(device.m_manufacturer);
	setModel(device.m_model);
	
	QMapIterator <t_input_channel, QLCInputChannel*> it(device.m_channels);
	while (it.hasNext() == true)
	{
		it.next();
		addChannel(new QLCInputChannel(*(it.value())));
	}
}
	
QLCInputDevice::~QLCInputDevice()
{
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

/****************************************************************************
 * Load & Save
 ****************************************************************************/

QLCInputDevice* QLCInputDevice::loader(QObject* parent, const QString& path)
{
	QLCInputDevice* device = NULL;
	QDomDocument* doc = NULL;

	if (QLCFile::readXML(path, &doc) == false)
		return false;
	Q_ASSERT(doc != NULL);

	device = new QLCInputDevice(parent);
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
	if (root.tagName() == KXMLQLCInputTemplate)
	{
		node = root.firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();
			if (tag.tagName() == KXMLQLCCreator)
			{
				/* Ignore */
			}
			if (tag.tagName() == KXMLQLCInputTemplateManufacturer)
			{
				setManufacturer(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputTemplateModel)
			{
				setModel(tag.text());
			}
			else if (tag.tagName() == KXMLQLCInputChannel)
			{
				QLCInputChannel* ich;
				ich = new QLCInputChannel(this);
				if (ich->loadXML(doc, &tag) == TRUE)
					addChannel(ich);
				else
					delete ich;
			}

			node = node.nextSibling();
		}
	}
	else
	{
		qDebug() << "Input template node not found in file!";
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

	if (QLCFile::getXMLHeader(KXMLQLCInputTemplate, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();

		/* Manufacturer */
		tag = doc->createElement(KXMLQLCInputTemplateManufacturer);
		text = doc->createTextNode(m_manufacturer);
		tag.appendChild(text);

		/* Model */
		tag = doc->createElement(KXMLQLCInputTemplateModel);
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
