/*
  Q Light Controller
  qlcfixture.cpp

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

#include <qfile.h>
#include <qstring.h>
#include <qdir.h>
#include <qdom.h>
#include <assert.h>

#include "common/qlcfixture.h"
#include "common/qlcfixturemode.h"
#include "common/qlcchannel.h"
#include "common/qlccapability.h"
#include "common/settings.h"
#include "common/filehandler.h"

// Old headers that will be removed in the future
#include "common/deviceclass.h"
#include "common/logicalchannel.h"

QLCFixture::QLCFixture()
{
	m_manufacturer = QString::null;
	m_model = QString::null;
	m_type = QString("Dimmer");
}

QLCFixture::QLCFixture(QLCFixture *fixture)
{
	m_manufacturer = QString::null;
	m_model = QString::null;
	m_type = QString("Dimmer");
	
	if (fixture != NULL)
		*this = *fixture;
}

QLCFixture::QLCFixture(const QString &fileName)
{
	QDomDocument* doc = NULL;
	QDomDocumentType doctype;
	QString errorString;

	Q_ASSERT(fileName != QString::null);

	m_manufacturer = QString::null;
	m_model = QString::null;
	m_type = QString("Dimmer");
	
	if (FileHandler::readXML(fileName, &doc) == true)
	{
		if (doc->doctype().name() == KXMLQLCFixtureDocument)
		{
			if (loadXML(doc) == false)
			{
				qWarning("%s: Not a fixture definition file",
					 fileName.ascii());
			}
		}
		else
		{
			qWarning("%s: Not a fixture definition file",
				 fileName.ascii());
		}
	}
	else
	{
		qWarning("%s: File parsing failed", fileName.ascii());
	}
}

QLCFixture::QLCFixture(DeviceClass* dc)
{
	QPtrListIterator<LogicalChannel> it(*dc->channels());
	LogicalChannel* lch = NULL;
	QLCChannel* qlcch = NULL;
	QLCFixtureMode* mode = NULL;
	
	m_manufacturer = dc->manufacturer();
	m_model = dc->model();
	m_type = dc->type();

	// Create a default mode
	mode = new QLCFixtureMode(this);
	mode->setName("Mode 1");
	addMode(mode);
	
	// Create QLCChannels from the DeviceClass' LogicalChannels
	while ((lch = it.current()) != 0)
	{
		qlcch = new QLCChannel(lch);
		addChannel(qlcch);
		mode->insertChannel(qlcch, lch->channel());
		++it;
	}
}

QLCFixture::~QLCFixture()
{
	while (m_channels.isEmpty() == false)
		delete m_channels.take(0);

	while (m_modes.isEmpty() == false)
		delete m_modes.take(0);

}

QLCFixture& QLCFixture::operator=(QLCFixture& fixture)
{
	if (this != &fixture)
	{
		QPtrListIterator<QLCChannel> chit(fixture.m_channels);
		QPtrListIterator<QLCFixtureMode> modeit(fixture.m_modes);
		QLCChannel* ch = NULL;
		QLCFixtureMode* mode = NULL;
		
		m_manufacturer = fixture.m_manufacturer;
		m_model = fixture.m_model;
		m_type = fixture.m_type;

		/* Clear all channels */
		while (m_channels.isEmpty() == false)
			delete m_channels.take(0);
		
		/* Copy channels from the other fixture */
		while ( (ch = chit.current()) != 0 )
		{
			m_channels.append(new QLCChannel(ch));
			++chit;
		}
		
		/* Clear all modes */
		while (m_modes.isEmpty() == false)
			delete m_modes.take(0);
		
		/* Copy modes from the other fixture */
		while ( (mode = modeit.current()) != 0 )
		{
			m_modes.append(new QLCFixtureMode(mode));
			++modeit;
		}
	}
	
	return *this;
}

/****************************************************************************
 * General properties
 ****************************************************************************/

void QLCFixture::setManufacturer(const QString mfg)
{
	m_manufacturer = QString(mfg);
}

void QLCFixture::setModel(const QString model)
{
	m_model = QString(model);
}

void QLCFixture::setType(const QString &type)
{
	m_type = QString(type);
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCFixture::addChannel(QLCChannel* channel)
{
	m_channels.append(channel);
}

bool QLCFixture::removeChannel(QLCChannel* channel)
{
	for (QLCChannel* ch = m_channels.first(); ch != NULL;
	     ch = m_channels.next())
	{
		if (ch == channel)
		{
			delete m_channels.take();
			return true;
		}
	}

	return false;
}

QLCChannel* QLCFixture::searchChannel(const QString &name)
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

/****************************************************************************
 * Modes
 ****************************************************************************/

void QLCFixture::addMode(QLCFixtureMode* mode)
{
	m_modes.append(mode);
}

bool QLCFixture::removeMode(QLCFixtureMode* mode)
{
	for (QLCFixtureMode* m = m_modes.first(); m != NULL; m = m_modes.next())
	{
		if (m == mode)
		{
			delete m_modes.take();
			return true;
		}
	}
	
	return false;
}

QLCFixtureMode* QLCFixture::searchMode(const QString& name)
{
	QPtrListIterator<QLCFixtureMode> it(m_modes);
	QLCFixtureMode* mode = NULL;
	
	while ( (mode = it.current()) != 0 )
	{
		if (mode->name() == name)
			return mode;
		++it;
	}
	
	return NULL;
}

/****************************************************************************
 * XML operations
 ****************************************************************************/

bool QLCFixture::saveXML(const QString &fileName)
{
	bool retval = false;
	QDomDocument* doc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	QFile file;

	file.setName(fileName);
	if (file.open(IO_WriteOnly) == false)
		return false;

	if (FileHandler::getXMLHeader(KXMLQLCFixtureDocument, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* Fixture tag */
		root = doc->documentElement();

		/* Manufacturer */
		tag = doc->createElement(KXMLQLCFixtureManufacturer);
		root.appendChild(tag);
		text = doc->createTextNode(m_manufacturer);
		tag.appendChild(text);
		
		/* Model */
		tag = doc->createElement(KXMLQLCFixtureModel);
		root.appendChild(tag);
		text = doc->createTextNode(m_model);
		tag.appendChild(text);
		
		/* Type */
		tag = doc->createElement(KXMLQLCFixtureType);
		root.appendChild(tag);
		text = doc->createTextNode(m_type);
		tag.appendChild(text);
		
		/* Channels */
		QPtrListIterator<QLCChannel> chit(m_channels);
		QLCChannel* ch = NULL;
		while ( (ch = chit.current()) != 0 )
		{
			ch->saveXML(doc, &root);
			++chit;
		}

		/* Modes */
		QPtrListIterator<QLCFixtureMode> modeit(m_modes);
		QLCFixtureMode* mode = NULL;
		while ( (mode = modeit.current()) != 0 )
		{
			mode->saveXML(doc, &root);
			++modeit;
		}

		/* Write the document into the stream */
		stream << doc->toString() << "\n";

		fprintf(stderr, "%s\n", doc->toString().ascii());

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

bool QLCFixture::loadXML(QDomDocument* doc)
{
	QDomElement root;
	QDomNode node;
	QDomElement tag;
	bool retval = false;

	assert(doc);

	root = doc->documentElement();
	if (root.tagName() == KXMLQLCFixture)
	{
		node = root.firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();

			if (tag.tagName() == KXMLQLCCreator)
				;
			else if (tag.tagName() == KXMLQLCFixtureManufacturer)
				setManufacturer(tag.text());
			else if (tag.tagName() == KXMLQLCFixtureModel)
				setModel(tag.text());
			else if (tag.tagName() == KXMLQLCFixtureType)
				setType(tag.text());
			else if (tag.tagName() == KXMLQLCChannel)
				addChannel(new QLCChannel(&tag));
			else if (tag.tagName() == KXMLQLCFixtureMode)
				addMode(new QLCFixtureMode(this, &tag));
			else
				qDebug("Unknown Fixture tag: %s",
					(const char*) tag.tagName());

			node = node.nextSibling();
		}

		retval = true;
	}
	else
	{
		qWarning("Fixture node not found in file!");
		retval = false;
	}

	return retval;
}
