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

#include <QMdiSubWindow>
#include <QMdiArea>
#include <QPalette>
#include <QString>
#include <QDebug>
#include <QtXml>
#include <QIcon>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"
#include "common/qlccapability.h"
#include "common/qlcchannel.h"
#include "common/qlctypes.h"

#include "fixtureconsole.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Fixture::Fixture(QLCFixtureDef* fixtureDef, QLCFixtureMode* mode,
		 t_channel address, t_channel universe, QString name,
		 t_fixture_id id)
{
	m_fixtureDef = fixtureDef;
	m_fixtureMode = mode;
	m_name = name;
	m_id = id;

	m_address = (m_address & 0xFE00) | (address & 0x01FF);
	m_address = (m_address & 0x01FF) | (universe << 9);

	m_console = NULL;
	m_genericChannel = NULL;
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
	m_genericChannel = NULL;
}

Fixture::~Fixture()
{
	if (m_console != NULL)
		delete m_console;

	if (m_genericChannel != NULL)
		delete m_genericChannel;
}

/*****************************************************************************
 * Fixture ID
 *****************************************************************************/

void Fixture::setID(t_fixture_id id)
{
	m_id = id;
	emit changed(m_id);
}

t_fixture_id Fixture::id()
{
	return m_id;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Fixture::setName(QString name)
{
	m_name = name;

	if (m_console != NULL)
		m_console->parentWidget()->setWindowTitle(m_name + " Console");

	emit changed(m_id);
}

QString Fixture::name()
{
	return m_name;
}

/*****************************************************************************
 * Fixture type
 *****************************************************************************/
QString Fixture::type()
{
	if (m_fixtureDef != NULL)
		return m_fixtureDef->type();
	else
		return QString(KXMLFixtureDimmer);
}

/*****************************************************************************
 * Universe
 *****************************************************************************/

void Fixture::setUniverse(t_channel universe)
{
	/* The universe part is stored in the highest 7 bits */
	m_address = (m_address & 0x01FF) | (universe << 9);

	emit changed(m_id);
}

t_channel Fixture::universe()
{
	/* The universe part is stored in the highest 7 bits */
	return (m_address >> 9);
}

/*****************************************************************************
 * Address
 *****************************************************************************/

void Fixture::setAddress(t_channel address)
{
	/* The address part is stored in the lowest 9 bits */
	m_address = (m_address & 0xFE00) | (address & 0x01FF);

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

/*****************************************************************************
 * Channels
 *****************************************************************************/

void Fixture::setChannels(t_channel channels)
{
	Q_ASSERT(m_fixtureDef == NULL && m_fixtureMode == NULL);
	m_channels = channels;
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
		return createGenericChannel();
}

int Fixture::channelAddress(t_channel channel)
{
	return universeAddress() + channel;
}

QLCChannel* Fixture::createGenericChannel()
{
	if (m_genericChannel == NULL)
	{
		m_genericChannel = new QLCChannel();
		Q_ASSERT(m_genericChannel != NULL);
		m_genericChannel->setGroup(KQLCChannelGroupIntensity);
		m_genericChannel->setName(KQLCChannelGroupIntensity);
		m_genericChannel->addCapability(
			new QLCCapability(0, 255, KQLCChannelGroupIntensity));
	}

	return m_genericChannel;
}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

void Fixture::setFixtureDefinition(QLCFixtureDef* fixtureDef,
				   QLCFixtureMode* fixtureMode)
{
	m_fixtureDef = fixtureDef;
	m_fixtureMode = fixtureMode;

	if (m_console != NULL)
		delete m_console;
	m_console = NULL;

	/* In case the old def was a dimmer and the new one is not, delete
	   the generic channel as possibly useless. It is created automatically
	   if it is again needed. */
	if (m_genericChannel != NULL)
		delete m_genericChannel;
	m_genericChannel = NULL;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

Fixture* Fixture::loader(QDomDocument* doc, QDomElement* root)
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
	QDomElement consoletag;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLFixture)
	{
		qDebug() << "Fixture instance node not found!";
		return NULL;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFixtureDefManufacturer)
		{
			manufacturer = tag.text();
		}
		else if (tag.tagName() == KXMLQLCFixtureDefModel)
		{
			model = tag.text();
		}
		else if (tag.tagName() == KXMLQLCFixtureMode)
		{
			modeName = tag.text();
		}
		else if (tag.tagName() == KXMLFixtureID)
		{
			id = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLFixtureName)
		{
			name = tag.text();
		}
		else if (tag.tagName() == KXMLFixtureUniverse)
		{
			universe = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLFixtureAddress)
		{
			address = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLFixtureChannels)
		{
			channels = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCFixtureConsole)
		{
			consoletag = tag;
		}
		else
		{
			qDebug() << "Unknown fixture instance tag:"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	/* Find the given fixture definition */
	fixtureDef = _app->fixtureDef(manufacturer, model);
	if (fixtureDef == NULL)
	{
		qDebug() << QString("No fixture definition for [%1 - %2]")
					.arg(manufacturer).arg(model);
	}
	else
	{
		/* Find the given fixture mode */
		fixtureMode = fixtureDef->mode(modeName);
		if (fixtureMode == NULL)
		{
			qDebug() << QString("Fixture mode [%1] for [%2 - %3] "
					    "not found!").arg(modeName)
					    .arg(manufacturer).arg(model);
		}
	}

	/* Number of channels */
	if (channels <= 0 || channels > KFixtureChannelsMax)
	{
		qDebug() << QString("Fixture [%1] channels %2 out of bounds "
				"(%3 - %4).").arg(name).arg(channels).arg(1)
				.arg(KFixtureChannelsMax);
		channels = 1;
	}

	/* Make sure that address is something sensible */
	if (address > 511 || address + (channels - 1) > 511)
	{
		qDebug() << QString("Fixture channel range %1 - %2 out of DMX "
				    "bounds (%3 - %4).").arg(address + 1)
				    .arg(address + channels).arg(1).arg(512);
		address = 0;
	}

	/* Make sure that universe is something sensible */
	if (universe > KUniverseCount)
	{
		qDebug() << QString("Fixture universe %1 out of bounds "
				    "(%2 - %3).").arg(universe).arg(0)
				    .arg(KUniverseCount);
		universe = 0;
	}

	/* Check that we have a sensible ID, otherwise we can't continue */
	if (id < 0 || id > KFixtureArraySize)
	{
		qDebug() << QString("Fixture ID %1 out of bounds (%2 - %3).")
				    .arg(id).arg(0).arg(KFixtureArraySize);
		return NULL;
	}

	/* Create the fixture */
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

	/* Insert the fixture to Doc's fixture array */
	if (_app->doc()->newFixture(fxi) == false)
	{
		delete fxi;
		fxi = NULL;
	}
	else
	{
		/* Load the fixture's console settings */
		if (consoletag.tagName() == KXMLQLCFixtureConsole)
		{
			fxi->viewConsole();
			fxi->m_console->loadXML(doc, &tag);
		}
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

	if (m_console != NULL)
		m_console->saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString Fixture::status()
{
	QPalette pal;
	QString info;
	QString t;

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
	info += pal.color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += pal.color(QPalette::HighlightedText).name();
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
		info += KXMLFixtureDimmer;

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
	info += pal.color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += pal.color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("<B>Channel</B>");
	info += QString("</FONT>");
	info += QString("</TD>");
	
	// DMX channel column title
	info += QString("<TD BGCOLOR=\"");
	info += pal.color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += pal.color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("<B>DMX</B>");
	info += QString("</FONT>");
	info += QString("</TD>");

	// Channel name column title
	info += QString("<TD BGCOLOR=\"");
	info += pal.color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += pal.color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"3\">");
	info += QString("<B>Name</B>");
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

/*****************************************************************************
 * Console
 *****************************************************************************/

void Fixture::viewConsole()
{
	if (m_console == NULL)
	{
		QMdiSubWindow* sub;

		sub = new QMdiSubWindow(_app->centralWidget());
		m_console = new FixtureConsole(_app);
		m_console->setChannelsCheckable(false);
		m_console->setFixture(m_id);

		sub->setWidget(m_console);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(":/console.png"));
		sub->setWindowTitle(m_name + " console");

		qobject_cast <QMdiArea*>
			(_app->centralWidget())->addSubWindow(sub);

		m_console->show();
		sub->show();

		connect(m_console, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotConsoleDestroyed(QObject*)));
	}
}

void Fixture::slotConsoleDestroyed(QObject*)
{
	m_console = NULL;
}
