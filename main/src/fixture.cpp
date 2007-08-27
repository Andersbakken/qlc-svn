/*
  Q Light Controller
  fixture.cpp

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

#include <assert.h>

#include "libs/common/qlcfixturedef.h"
#include "libs/common/qlcfixturemode.h"
#include "fixture.h"
#include "fixtureconsole.h"
#include "libs/common/types.h"
#include "app.h"
#include "doc.h"

extern App* _app;
extern QApplication* _qapp;

Fixture::Fixture(QLCFixtureDef* fixtureDef,
		 QLCFixtureMode* mode,
		 t_channel address,
		 t_channel universe,
		 QString name,
		 t_fixture_id id)
{
	m_fixtureDef = fixtureDef;
	m_fixtureMode = mode;
	m_name = name;
	m_id = id;

	m_address = (m_address & 0xFE00) | (address & 0x01FF);
	m_address = (m_address & 0x01FF) | (universe << 9);

	m_console = NULL;
}

Fixture::Fixture(t_channel address, t_channel universe, t_channel channels,
		 QString name, t_fixture_id id)
{
	m_fixtureDef = NULL;
	m_fixtureMode = NULL;
	m_name = name;
	m_id = id;
	m_channels = channels;

	m_address = (m_address & 0xFE00) | (address & 0x01FF);
	m_address = (m_address & 0x01FF) | (universe << 9);

	m_console = NULL;
}

Fixture::~Fixture()
{
}

void Fixture::setID(t_fixture_id id)
{
	m_id = id;
	emit changed(m_id);
}

t_fixture_id Fixture::id()
{
	return m_id;
}

void Fixture::setName(QString name)
{
	m_name = name;

	if (m_console != NULL)
		m_console->setCaption(m_name + " Console");

	emit changed(m_id);
}

QString Fixture::name()
{
	return m_name;
}

void Fixture::setUniverse(t_channel universe)
{
	/* The universe part is stored in the highest 7 bits */
	m_address = (m_address & 0x01FF) | (universe << 9);

	if (m_console != NULL)
	{
		slotConsoleClosed();
		viewConsole();
	}

	emit changed(m_id);
}

t_channel Fixture::universe()
{
	/* The universe part is stored in the highest 7 bits */
	return (m_address >> 9);
}

void Fixture::setAddress(t_channel address)
{
	/* The address part is stored in the lowest 9 bits */
	m_address = (m_address & 0xFE00) | (address & 0x01FF);

	if (m_console != NULL)
	{
		slotConsoleClosed();
		viewConsole();
	}

	emit changed(m_id);
}

t_channel Fixture::address()
{
	/* The address part is stored in the lowest 9 bits */
	return (m_address & 0x01FF);
}

t_channel Fixture::universeAddress()
{
	return m_address;
}

t_channel Fixture::channels()
{
	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		return m_fixtureMode->channels();
	else
		return m_channels;
}

QLCChannel* Fixture::channel(t_channel channel)
{
	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		return m_fixtureMode->channel(channel);
	else
		return NULL;
}

Fixture* Fixture::loadXML(QDomDocument* doc, QDomElement* root)
{
	Fixture* fxi = NULL;
	QLCFixtureDef* fixtureDef = NULL;
	QLCFixtureMode* fixtureMode = NULL;
	QString manufacturer;
	QString model;
	QString modeName;
	QString name;
	t_fixture_id id = KNoID;
	t_channel universe = 0;
	t_channel address = 0;
	t_channel channels = 0;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLFixture)
	{
		qWarning("Fixture instance node not found!");
		return NULL;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFixtureDefManufacturer)
			manufacturer = tag.text();
		else if (tag.tagName() == KXMLQLCFixtureDefModel)
			model = tag.text();
		else if (tag.tagName() == KXMLQLCFixtureMode)
			modeName = tag.text();
		else if (tag.tagName() == KXMLFixtureID)
			id = tag.text().toInt();
		else if (tag.tagName() == KXMLFixtureName)
			name = tag.text();
		else if (tag.tagName() == KXMLFixtureUniverse)
			universe = tag.text().toInt();
		else if (tag.tagName() == KXMLFixtureAddress)
			address = tag.text().toInt();
		else if (tag.tagName() == KXMLFixtureChannels)
			channels = tag.text().toInt();
		else
			qDebug("Unknown fixture instance tag: %s",
			       (const char*) tag.tagName());
		
		node = node.nextSibling();
	}

	/* Find the given fixture definition */
	fixtureDef = _app->fixtureDef(manufacturer, model);
	if (fixtureDef == NULL)
	{
		qWarning("Fixture definition for [%s - %s] not found!",
			 (const char*) manufacturer, (const char*) model);
	}
	else
	{
		/* Find the given fixture mode */
		fixtureMode = fixtureDef->mode(modeName);
		if (fixtureMode == NULL)
		{
			qWarning("Fixture mode [%s] for [%s - %s] not found!",
				 (const char*) modeName, (const char*) manufacturer,
				 (const char*) model);
		}
	}

	/* Number of channels */
	if (channels <= 0 || channels > KFixtureChannelsMax)
	{
		qWarning("Fixture channels %d out of bounds (%d - %d)!",
			 id, 1, KFixtureChannelsMax);
		channels = 1;
	}

	/* Make sure that address is something sensible */
	if (address > 511 || address + channels > 511)
	{
		qWarning("Fixture address %d out of DMX bounds (%d - %d)!",
			 address, 0, 511);
		address = 0;
	}

	/* Make sure that universe is something sensible */
	if (universe > KUniverseCount)
	{
		qWarning("Fixture universe %d out of bounds (%d - %d)!",
			 universe, 0, KUniverseCount);
		universe = 0;
	}

	/* Check that we have a sensible ID, otherwise we can't continue */
	if (id < 0 || id > KFixtureArraySize)
	{
		qWarning("Fixture ID %d out of bounds (%d - %d)!",
			 id, 0, KFixtureArraySize);
		return NULL;
	}

	if (fixtureDef != NULL && fixtureMode != NULL)
	{
		/* Create a normal fixture */
		fxi = new Fixture(fixtureDef, fixtureMode, address, universe,
				  name, id);
	}
	else
	{
		/* Create a generic fixture */
		fxi = new Fixture(address, universe, channels, name, id);
	}

	return fxi;
}

bool Fixture::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);

	/* Fixture Instance entry */
	root = doc->createElement(KXMLFixture);
	wksp_root->appendChild(root);

	/* Manufacturer */
	tag = doc->createElement(KXMLQLCFixtureDefManufacturer);
	root.appendChild(tag);

	if (m_fixtureDef != NULL)
		text = doc->createTextNode(m_fixtureDef->manufacturer());
	else
		text = doc->createTextNode(KXMLFixtureGeneric);

	tag.appendChild(text);

	/* Model */
	tag = doc->createElement(KXMLQLCFixtureDefModel);
	root.appendChild(tag);

	if (m_fixtureDef != NULL)
		text = doc->createTextNode(m_fixtureDef->model());
	else
		text = doc->createTextNode(KXMLFixtureGeneric);

	tag.appendChild(text);

	/* Fixture mode */
	tag = doc->createElement(KXMLQLCFixtureMode);
	root.appendChild(tag);

	if (m_fixtureMode != NULL)
		text = doc->createTextNode(m_fixtureMode->name());
	else
		text = doc->createTextNode(KXMLFixtureGeneric);
		
	tag.appendChild(text);

	/* ID */
	tag = doc->createElement(KXMLFixtureID);
	root.appendChild(tag);
	str.setNum(id());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Name */
	tag = doc->createElement(KXMLFixtureName);
	root.appendChild(tag);
	text = doc->createTextNode(m_name);
	tag.appendChild(text);

	/* Universe */
	tag = doc->createElement(KXMLFixtureUniverse);
	root.appendChild(tag);
	str.setNum(universe());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Address */
	tag = doc->createElement(KXMLFixtureAddress);
	root.appendChild(tag);
	str.setNum(address());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Channel count */
	tag = doc->createElement(KXMLFixtureChannels);
	root.appendChild(tag);
	str.setNum(channels());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	return true;
}

QString Fixture::status()
{
	QString t;
	QString info;

	// HTML header
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Fixture Information</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	// Fixture title
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");
	
	// Manufacturer
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Manufacturer</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	
	if (m_fixtureDef != NULL)
		info += m_fixtureDef->manufacturer();
	else
		info += KXMLFixtureGeneric;

	info += QString("</TD>");
	info += QString("</TR>");

	// Model
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Model</B>");
	info += QString("</TD>");
	info += QString("<TD>");

	if (m_fixtureDef != NULL)
		info += m_fixtureDef->model();
	else
		info += KXMLFixtureGeneric;

	info += QString("</TD>");
	info += QString("</TR>");
	
	// Mode
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Mode</B>");
	info += QString("</TD>");
	info += QString("<TD>");

	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		info += m_fixtureMode->name();
	else
		info += KXMLFixtureGeneric;
		
	info += QString("</TD>");
	info += QString("</TR>");

	// Type
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Type</B>");
	info += QString("</TD>");
	info += QString("<TD>");

	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		info += m_fixtureDef->type();
	else
		info += "Dimmer";

	info += QString("</TD>");
	info += QString("</TR>");

	// Universe
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Universe</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += t.sprintf("%d", universe() + 1);
	info += QString("</TD>");
	info += QString("</TR>");
	
	// Address
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Address space</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += t.sprintf("%d - %d", address() + 1, address() + channels());
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");
	
	//
	// Channels
	//
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");
	
	// Relative channel column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("Channel");
	info += QString("</FONT>");
	info += QString("</TD>");
	
	// DMX channel column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("DMX");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	// Channel name column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("Name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	
	// Fill table with device's channels
	for (t_channel ch = 0; ch < channels();	ch++)
	{
		info += QString("<TR>");

		// Relative channel
		info += QString("<TD>");
		info += t.setNum(ch + 1);
		info += QString("</TD>");

		// DMX channel
		info += QString("<TD>");
		info += t.setNum(address() + ch + 1);
		info += QString("</TD>");
		
		// Channel name
		info += QString("<TD>");
		
		if (m_fixtureDef != NULL && m_fixtureMode != NULL)
			info += channel(ch)->name();
		else
			info += "Level";

		info += QString("</TD>");
	}

	info += QString("</TR>");
	info += QString("</TABLE>");
	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
}

void Fixture::viewConsole()
{
	if (m_console == NULL)
	{
		m_console = new FixtureConsole((QWidget*) _app->workspace());
		m_console->setFixture(m_id);
		
		// Set window title
		m_console->setCaption(m_name + " Console");
		
		// Catch close event
		connect(m_console, SIGNAL(closed()),
			this, SLOT(slotConsoleClosed()));
		
		m_console->show();
		
		m_console->resize(m_console->width() + 100, 300);
	}
	else
	{
		m_console->hide();
		m_console->show();
	}
}

void Fixture::slotConsoleClosed()
{
	disconnect(m_console);
	delete m_console;
	m_console = NULL;
}
