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

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlctypes.h"

#include "fixture.h"
#include "doc.h"

#define KInvalidFixtureID -1
#define KInvalidFixtureChannel USHRT_MAX

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Fixture::Fixture(QObject* parent) : QObject(parent)
{
	m_id = KInvalidFixtureID;

	m_address = 0;
	m_channels = 0;

	m_fixtureDef = NULL;
	m_fixtureMode = NULL;

	m_genericChannel = NULL;
	createGenericChannel();
}

Fixture::~Fixture()
{
	if (m_genericChannel != NULL)
		delete m_genericChannel;
	m_genericChannel = NULL;
}

bool Fixture::operator<(const Fixture& fxi)
{
	if (m_address < fxi.m_address)
		return true;
	else
		return false;
}

/*****************************************************************************
 * Fixture ID
 *****************************************************************************/

void Fixture::setID(t_fixture_id id)
{
	m_id = id;
	emit changed(m_id);
}

t_fixture_id Fixture::id() const
{
	return m_id;
}

t_fixture_id Fixture::invalidId()
{
	return KInvalidFixtureID;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Fixture::setName(const QString& name)
{
	m_name = name;
	emit changed(m_id);
}

QString Fixture::name() const
{
	return m_name;
}

/*****************************************************************************
 * Fixture type
 *****************************************************************************/

QString Fixture::type() const
{
	if (m_fixtureDef != NULL)
		return m_fixtureDef->type();
	else
		return QString(KXMLFixtureDimmer);
}

bool Fixture::isDimmer() const
{
	if (m_fixtureDef != NULL)
		return false;
	else
		return true;
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

t_channel Fixture::universe() const
{
	/* The universe part is stored in the highest 7 bits */
	return (m_address >> 9);
}

/*****************************************************************************
 * Address
 *****************************************************************************/

void Fixture::setAddress(t_channel address)
{
	/* Don't allow more than 512 channels per universe */
	if (address > 511)
		return;

	/* The address part is stored in the lowest 9 bits */
	m_address = (m_address & 0xFE00) | (address & 0x01FF);

	emit changed(m_id);
}

t_channel Fixture::address() const
{
	/* The address part is stored in the lowest 9 bits */
	return (m_address & 0x01FF);
}

t_channel Fixture::universeAddress() const
{
	return m_address;
}

/*****************************************************************************
 * Channels
 *****************************************************************************/

void Fixture::setChannels(t_channel channels)
{
	m_channels = channels;
}

t_channel Fixture::channels() const
{
	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		return m_fixtureMode->channels().size();
	else
		return m_channels;
}

const QLCChannel* Fixture::channel(t_channel channel) const
{
	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		return m_fixtureMode->channel(channel);
	else if (channel < channels())
		return m_genericChannel;
	else
		return NULL;
}

int Fixture::channelAddress(t_channel channel) const
{
	if (channel < channels())
		return universeAddress() + channel;
	else
		return invalidChannel();
}

t_channel Fixture::channel(const QString& name, Qt::CaseSensitivity cs,
			   const QString& group) const
{
	if (m_fixtureDef == NULL && m_fixtureMode == NULL)
	{
		/* There's just one generic channel object with "Intensity" as
		   its name that is the same for all channel numbers. So
		   there's really no point in returning 0 or any otherwise
		   valid channel number here. Which one of them the user would
		   want to get? */
		return invalidChannel();
	}
	else
	{
		/* Search for the channel name (and group) from our list */
		for (t_channel i = 0; i < m_fixtureMode->channels().size(); i++)
		{
			const QLCChannel* ch;

			ch = m_fixtureMode->channel(i);
			Q_ASSERT(ch != NULL);

			if (group != QString::null && ch->group() != group)
			{
				/* Given group name doesn't match */
				continue;
			}
			else if (ch->name().contains(name, cs) == true)
			{
				/* Found the channel */
				return i;
			}
		}

		/* Went thru all channels but a match was not found */
		return invalidChannel();
	}
}

t_channel Fixture::invalidChannel()
{
	return KInvalidFixtureChannel;
}

void Fixture::createGenericChannel()
{
	if (m_genericChannel == NULL)
	{
		m_genericChannel = new QLCChannel();
		Q_ASSERT(m_genericChannel != NULL);
		m_genericChannel->setGroup(KQLCChannelGroupIntensity);
		m_genericChannel->setName(KQLCChannelGroupIntensity);
		m_genericChannel->addCapability(new QLCCapability(0, 255,
						KQLCChannelGroupIntensity));
	}
}

/*****************************************************************************
 * DMX display style
 *****************************************************************************/

bool Fixture::isDMXZeroBased() const
{
	QSettings settings;
	QVariant value;
	QString key;

	key = QString("/outputmap/universe%1/dmxzerobased").arg(universe());
	value = settings.value(key);
	if (value.isValid() == true)
		return value.toBool();
	else
		return false;
}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

void Fixture::setFixtureDefinition(const QLCFixtureDef* fixtureDef,
				   const QLCFixtureMode* fixtureMode)
{
	if (fixtureDef != NULL && fixtureMode != NULL)
	{
		m_fixtureDef = fixtureDef;
		m_fixtureMode = fixtureMode;

		if (m_genericChannel != NULL)
			delete m_genericChannel;
		m_genericChannel = NULL;
	}
	else
	{
		m_fixtureDef = NULL;
		m_fixtureMode = NULL;
		createGenericChannel();
	}

	emit changed(m_id);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Fixture::loader(const QDomElement* root, Doc* doc)
{
	bool result = false;

	Fixture* fxi = new Fixture(doc);
	Q_ASSERT(fxi != NULL);

	if (fxi->loadXML(root, doc->fixtureDefCache()) == true)
	{
		if (doc->addFixture(fxi, fxi->id()) == true)
		{
			/* Success */
			result = true;
		}
		else
		{
			/* Doc is full */
			qWarning() << "Fixture" << fxi->name()
				   << "cannot be created.";
			delete fxi;
		}
	}
	else
	{
		qWarning() << "Fixture" << fxi->name()
			   << "cannot be loaded.";
		delete fxi;
	}

	return result;
}

bool Fixture::loadXML(const QDomElement* root,
		      const QLCFixtureDefCache& fixtureDefCache)
{
	const QLCFixtureDef* fixtureDef = NULL;
	const QLCFixtureMode* fixtureMode = NULL;
	QString manufacturer;
	QString model;
	QString modeName;
	QString name;
	t_fixture_id id = KInvalidFixtureID;
	t_channel universe = 0;
	t_channel address = 0;
	t_channel channels = 0;

	QDomElement tag;
	QDomNode node;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLFixture)
	{
		qDebug() << "Fixture node not found!";
		return false;
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
		else
		{
			qDebug() << "Unknown fixture instance tag:"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	/* Find the given fixture definition */
	fixtureDef = fixtureDefCache.fixtureDef(manufacturer, model);
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
			/* Set this also NULL so that a generic dimmer will be
			   created instead as a backup. */
			fixtureDef = NULL;
		}
	}

	/* Number of channels */
	if (channels <= 0 || channels > KFixtureChannelsMax)
	{
		qDebug() << QString("Fixture [%1] channels %2 out of bounds "
				"(%3 - %4).").arg(name).arg(channels).arg(1)
				.arg(KFixtureChannelsMax);
		channels = KFixtureChannelsMax;
	}

	/* Make sure that address is something sensible */
	if (address > 511 || address + (channels - 1) > 511)
	{
		qDebug() << QString("Fixture channel range %1 - %2 out of DMX "
				    "bounds (%3 - %4).").arg(address)
				    .arg(address + channels).arg(0).arg(511);
		address = 0;
	}

	/* Make sure that universe is something sensible */
	if (universe >= KUniverseCount)
	{
		qDebug() << QString("Fixture universe %1 out of bounds "
				    "(%2 - %3).").arg(universe).arg(0)
				    .arg(KUniverseCount - 1);
		universe = 0;
	}

	/* Check that we have a sensible ID, otherwise we can't continue */
	if (id < 0 || id >= KFixtureArraySize)
	{
		qDebug() << QString("Fixture ID %1 out of bounds (%2 - %3).")
				    .arg(id).arg(0).arg(KFixtureArraySize - 1);
		return false;
	}

	if (fixtureDef != NULL && fixtureMode != NULL)
	{
		/* Assign fixtureDef & mode only if BOTH are not NULL */
		setFixtureDefinition(fixtureDef, fixtureMode);
	}
	else
	{
		/* Otherwise set just the channel count */
		setChannels(channels);
	}

	setAddress(address);
	setUniverse(universe);
	setName(name);
	setID(id);

	return true;
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

/*****************************************************************************
 * Status
 *****************************************************************************/

QString Fixture::status()
{
	QString info;
	QString t;

	QPalette pal;
	QColor hlBack(pal.color(QPalette::Highlight));
	QColor hlText(pal.color(QPalette::HighlightedText));

	QString title("<TR><TD CLASS='hilite' COLSPAN='3'>%1</TD></TR>");
	QString subTitle("<TR><TD CLASS='subhi' COLSPAN='3'>%1</TD></TR>");
	QString genInfo("<TR><TD CLASS='emphasis' COLSPAN='2'>%1</TD><TD>%2</TD></TR>");

	// HTML header
	info += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
	info += "<HTML><HEAD></HEAD><STYLE>";
	info += QString(".hilite {" \
			"	background-color: %1;" \
			"	color: %2;" \
			"	font-size: x-large;" \
			"}").arg(hlBack.name()).arg(hlText.name());
	info += QString(".subhi {" \
			"	background-color: %1;" \
			"	color: %2;" \
			"	font-weight: bold;" \
			"}").arg(hlBack.name()).arg(hlText.name());
	info += QString(".emphasis {" \
			"	font-weight: bold;" \
			"}");
	info += "</STYLE><BODY>";

	/********************************************************************
	 * General info
	 ********************************************************************/

	info += "<TABLE COLS='3' WIDTH='100%'>";

	// Fixture title
	info += title.arg(name());

	// Manufacturer
	if (isDimmer() == false)
	{
		info += genInfo.arg(tr("Manufacturer")).arg(m_fixtureDef->manufacturer());
		info += genInfo.arg(tr("Model")).arg(m_fixtureDef->model());
		info += genInfo.arg(tr("Mode")).arg(m_fixtureMode->name());
		info += genInfo.arg(tr("Type")).arg(m_fixtureDef->type());
	}
	else
	{
		info += genInfo.arg(tr("Type")).arg(tr("Generic Dimmer"));
	}

	// Universe
	info += genInfo.arg(tr("Universe")).arg(universe() + 1);

	// Address
	info += genInfo.arg(tr("Address")).arg(address() + channels());

	// Binary address
	info += genInfo.arg(tr("Binary Address (DIP)"))
			.arg(QString("%1").arg(address() + 1, 9, 2, QChar('0')));

	/********************************************************************
	 * Channels
	 ********************************************************************/

	// Title row
	info += QString("<TR><TD CLASS='subhi'>%1</TD>").arg(tr("Channel"));
	info += QString("<TD CLASS='subhi'>%1</TD>").arg(tr("DMX"));
	info += QString("<TD CLASS='subhi'>%1</TD></TR>").arg(tr("Name"));

	// Fill table with the fixture's channels
	for (t_channel ch = 0; ch < channels();	ch++)
	{
		QString chInfo("<TR><TD>%1</TD><TD>%2</TD><TD>%3</TD></TR>");
		info += chInfo.arg(ch + 1).arg(address() + ch + 1)
					  .arg(channel(ch)->name());
	}

	/********************************************************************
	 * Extended device information for non-dimmers
	 ********************************************************************/

	if (isDimmer() == false)
	{
		QLCPhysical physical = m_fixtureMode->physical();
		info += title.arg(tr("Physical"));

		QString mm("%1mm");
		QString W("%1W");
		info += genInfo.arg(tr("Width")).arg(mm.arg(physical.width()));
		info += genInfo.arg(tr("Height")).arg(mm.arg(physical.height()));
		info += genInfo.arg(tr("Depth")).arg(mm.arg(physical.depth()));
		info += genInfo.arg(tr("Weight")).arg(mm.arg(physical.weight()));
		info += genInfo.arg(tr("Power consumption")).arg(W.arg(physical.powerConsumption()));
		info += genInfo.arg(tr("DMX Connector")).arg(physical.dmxConnector());

		// Bulb
		QString K("%1K");
		QString lm("%1lm");
		info += subTitle.arg(tr("Bulb"));
		info += genInfo.arg(tr("Type")).arg(physical.bulbType());
		info += genInfo.arg(tr("Luminous Flux")).arg(lm.arg(physical.bulbLumens()));
		info += genInfo.arg(tr("Colour Temperature")).arg(K.arg(physical.bulbColourTemperature()));

		// Lens
		QString angle("%1&deg; - %2&deg;");
		info += subTitle.arg(tr("Lens"));
		info += genInfo.arg(tr("Name")).arg(physical.lensName());
		info += genInfo.arg(tr("Beam Angle"))
			.arg(angle.arg(physical.lensDegreesMin())
				  .arg(physical.lensDegreesMax()));

		// Focus
		QString range("%1&deg;");
		info += subTitle.arg(tr("Focus"));
		info += genInfo.arg(tr("Type")).arg(physical.focusType());
		info += genInfo.arg(tr("Pan Range")).arg(range.arg(physical.focusPanMax()));
		info += genInfo.arg(tr("Tilt Range")).arg(range.arg(physical.focusTiltMax()));
	}

	// HTML document & table closure
	info += "</TABLE></BODY></HTML>";

	return info;
}
