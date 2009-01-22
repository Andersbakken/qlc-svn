/*
  Q Light Controller
  fixture.h

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

#ifndef FIXTURE_H
#define FIXTURE_H

#include <QObject>
#include <QList>
#include "common/qlctypes.h"

#define KXMLFixture "Fixture"
#define KXMLFixtureName "Name"
#define KXMLFixtureUniverse "Universe"
#define KXMLFixtureAddress "Address"
#define KXMLFixtureID "ID"
#define KXMLFixtureGeneric "Generic"
#define KXMLFixtureChannels "Channels"
#define KXMLFixtureDimmer "Dimmer"

class QFile;
class QString;
class QDomDocument;
class QDomElement;

class QLCFixtureDef;
class QLCFixtureMode;
class QLCChannel;
class FixtureConsole;

class Fixture : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new fixture instance using a fixture definition and
	 * the given mode. Also, optionally assign the fixture a name.
	 *
	 * @param fixture A QLCFixture definition that the fixture instance is
	 *                based on.
	 * @param mode One of the modes specified for the fixture
	 * @param address This instance's DMX address
	 * @param universe This instance's DMX universe
	 * @param name Friendly name that identifies the new instance
	 * @param id A unique fixture ID assigned by @ref Doc
	 */
	Fixture(QLCFixtureDef* fixtureDef, QLCFixtureMode* mode,
		t_channel address, t_channel universe, QString name,
		t_fixture_id id);

	/**
	 * Create a generic dimmer-style fixture instance, that has no special
	 * abilities, except channels that can have normal DMX values.
	 *
	 * @param address This instance's DMX address
	 * @param universe This instance's DMX universe
	 * @param channels Number of channels to use for the fixture
	 * @param name Friendly name that identifies the new instance
	 * @param id A unique fixture ID assigned by class @ref Doc
	 */
	Fixture(t_channel address, t_channel universe, t_channel channels,
		QString name, t_fixture_id id);

	/**
	 * Create a new fixture instance from an XML substructure.
	 *
	 * @param doc An XML document containing the given tag
	 * @param tag An XML tag that contains a fixture instance structure
	 */
	Fixture(QDomDocument* doc, QDomElement* tag);

	/**
	 * Destructor
	 */
	~Fixture();

	/** Less-than operator for qSort() */
	bool operator<(const Fixture& fxi);
	
private:
	Q_DISABLE_COPY(Fixture)

	/*********************************************************************
	 * Fixture ID
	 *********************************************************************/
protected:
	/**
	 * Change the fixture instance's fixture ID. This is generally VERY
	 * dangerous, since all functions will cease to work unless their
	 * fixture ID's are changed as well. Also, the fixture will have to
	 * be moved within Doc's fixture array to reflect the new ID, which
	 * is essentially the fixture's position in that array.
	 *
	 * @param id The new fixture id
	 */
	void setID(t_fixture_id id);

public:
	/**
	 * Get the fixture instance's fixture ID.
	 *
	 * @return The instance's fixture ID
	 */
	t_fixture_id id() const;

protected:
	/** Fixture ID */
	t_fixture_id m_id;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/**
	 * Change the fixture instance's friendly name.
	 *
	 * @param name The new name to assign for the instance
	 */
	void setName(QString name);

	/**
	 * Get the fixture instance's friendly name.
	 *
	 * @return The instance's friendly name
	 */
	QString name() const;

protected:
	/** Friendly name */
	QString m_name;

	/*********************************************************************
	 * Fixture type
	 *********************************************************************/
public:
	/**
	 * Get the fixture's type
	 *
	 */
	QString type() const;

	/*********************************************************************
	 * Universe
	 *********************************************************************/
public:
	/**
	 * Set the fixture instance's DMX universe
	 *
	 * @param universe A zero-based DMX universe (i.e. 0-7; not 1-8)
	 */
	void setUniverse(t_channel universe);

	/**
	 * Get the fixture instance's DMX universe
	 *
	 * @return A zero-based DMX address (i.e. 0-511; not 1-512)
	 */
	t_channel universe() const;

	/*********************************************************************
	 * Address
	 *********************************************************************/
public:
	/**
	 * Set the fixture instance's DMX address
	 *
	 * @param address A zero-based DMX address (i.e. 0-511; not 1-512)
	 */
	void setAddress(t_channel address);

	/**
	 * Get the fixture instance's DMX address
	 *
	 * @return A zero-based DMX address (i.e. 0-511; not 1-512)
	 */
	t_channel address() const;

public:
	/**
	 * Get the fixture instance's DMX address & universe as one
	 *
	 * @return The fixture's address & universe
	 */
	t_channel universeAddress() const;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/**
	 * Change the number of channels. Valid only for generic dimmers, whose
	 * $fixtureDef == NULL && $fixtureMode == NULL.
	 *
	 * @param channels The new number of channels
	 */
	void setChannels(t_channel channels);

	/**
	 * Get the number of channels occupied by this fixture instance.
	 * This takes also the selected mode into account, as different modes
	 * can have different channel sets.
	 *
	 * @return Number of channels
	 */
	t_channel channels() const;

	/**
	 * Get a specific channel object by the channel's number. For generic
	 * dimmers, the returned QLCChannel is the same for all channel numbers.
	 *
	 * @param channel The channel number to get
	 * @return A QLCChannel* instance that should not be modified
	 */
	const QLCChannel* channel(t_channel channel);

	/**
	 * Get a fixture's channel's DMX address.
	 *
	 */
	int channelAddress(t_channel channel) const;

	/**
	 * Get a channel by its name. Comparison is done as a "contains"
	 * operation, i.e. the given name can be a substring of a longer name.
	 * If group is QString::null, it is ignored.
	 */
	t_channel channel(const QString& name,
			  Qt::CaseSensitivity cs = Qt::CaseSensitive,
			  const QString& group = QString::null) const;

protected:
	/**
	 * Create a QLCChannel for generic dimmers if such a channel doesn't
	 * exist yet. Returns just the one channel.
	 *
	 * @return The generic QLCChannel 
	 */
	const QLCChannel* createGenericChannel();

protected:
	/** DMX address & universe */
	t_channel m_address;

	/** Number of channels (ONLY for dimmer fixtures!) */
	t_channel m_channels;

	/** Generic intensity channel for dimmer fixtures */
	QLCChannel* m_genericChannel;

	/*********************************************************************
	 * Fixture definition
	 *********************************************************************/
public:
	/**
	 * Change the fixture's definition & mode. Use this when changing an
	 * existing fixture to use another def.
	 *
	 * @param fixtureDef The new fixture definition
	 * @param fixtureMode The new fixture mode (member of $fixtureDef)
	 */
	void setFixtureDefinition(QLCFixtureDef* fixtureDef,
				  QLCFixtureMode* fixtureMode);

	/**
	 * Get the fixture definition that this fixture instance is based on.
	 *
	 * @return A QLCFixture definition
	 */
	QLCFixtureDef* fixtureDef() const { return m_fixtureDef; }

	/**
	 * Get the fixture mode that this fixture instance is based on.
	 *
	 * @return A QLCFixtureMode definition
	 */
	QLCFixtureMode* fixtureMode() const { return m_fixtureMode; }

protected:
	/** The fixture definition that this instance is based on */
	QLCFixtureDef* m_fixtureDef;

	/** The mode within the fixture definition that this instance uses */
	QLCFixtureMode* m_fixtureMode;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Load a single fixture instance from an XML document, under
	 * the specified fixture tag.
	 *
	 * @param doc The XML document to load from
	 * @param root An XML subtree containing a single fixture instance
	 * @return A newly-created Fixture instance or NULL if loading failed
	 */
	static Fixture* loader(QDomDocument* doc, QDomElement* root);

	/**
	 * Save the fixture instance into an XML document, under the given
	 * XML element (tag).
	 *
	 * @param doc The master XML document to save to.
	 * @param wksp_root The workspace root element
	 */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/**
	 * Get the fixture instance's status info for Fixture Manager
	 *
	 * @return A sort-of HTML-RTF-gibberish for Fixture Manager
	 */
	QString status();

	/*********************************************************************
	 * Console
	 *********************************************************************/
public:
	/** View the fixture's console */
	void viewConsole();

public slots:
	/**
	 * Callback for console close signals. This has to be public because
	 * this is used from fixture manager
	 */
	void slotConsoleDestroyed(QObject* object);

protected:
	/** The fixture's console */
	FixtureConsole* m_console;

	/*********************************************************************
	 * Signals
	 *********************************************************************/
signals:
	void changed(t_fixture_id);
};

#endif

