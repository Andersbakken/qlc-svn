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

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCInputDevice::addChannel(t_input_channel channel, QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);
	m_channels.insert(channel, ich);
}
	
void QLCInputDevice::removeChannel(QLCInputChannel* ich)
{
	Q_ASSERT(ich != NULL);

	QMapIterator <t_input_channel, QLCInputChannel*> it(m_channels);
	while (it.hasNext() == true)
	{
		it.next();
		if (it.value() == ich)
		{
			m_channels.remove(it.key());
			break;
		}
	}
}

void QLCInputDevice::removeChannel(t_input_channel channel)
{
	m_channels.remove(channel);
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

void QLCInputDevice::availableTemplates(QStringList* names, QStringList* paths)
{
#ifdef WIN32
	QDir dir("C:/QLC/InputTemplates/");
#else
	QDir dir("/usr/share/inputtemplates/");
#endif
	QStringList nameFilters;
	QStringList entries;
	QStringList::iterator it;
	QString name, path;

	nameFilters << "*.qxi";
	entries = dir.entryList(nameFilters, QDir::Files | QDir::System);
	for (it = entries.begin(); it != entries.end(); ++it)
	{
		path = dir.absolutePath() + QDir::separator() + *it;
		name = QLCInputDevice::loadXMLName(path);
		
		if (path.isEmpty() == false && name.isEmpty() == false)
		{
			*names << name;
			*paths << path;
		}
	}
}

QString QLCInputDevice::loadXMLName(const QString& path)
{
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomNode node;
	QString make;
	QString model;
	int i = 0;

	if (QLCFile::readXML(path, &doc) == false)
		return QString::null;
	Q_ASSERT(doc != NULL);

	root = doc->documentElement();
	if (root.tagName() == KXMLQLCInputTemplate)
	{
		node = root.firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();
			if (tag.tagName() == KXMLQLCInputTemplateManufacturer)
			{
				make = tag.text();
				i |= 0x1;
			}
			else if (tag.tagName() == KXMLQLCInputTemplateModel)
			{
				model = tag.text();
				i |= 0x2;
			}

			/* Bail out after we've had a make & model tag */
			if (i == 0x3)
				break;
			else
				node = node.nextSibling();
		}
		
		delete doc;
		return QString("%1 - %2").arg(make).arg(model);
	}
	else
	{
		qDebug() << "Input template node not found in file!";
		delete doc;
		return QString::null;
	}
}

QLCInputDevice* QLCInputDevice::load(QObject* parent, const QString& path)
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
					addChannel(ich->channel(), ich);
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

bool QLCInputDevice::saveXML(const QString& path)
{
	return false;
}
