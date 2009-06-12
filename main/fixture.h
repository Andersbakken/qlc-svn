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

class QLCFixtureDefCache;
class QLCFixtureMode;
class FixtureConsole;
class QLCFixtureDef;
class QLCChannel;
class Doc;

class Fixture : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new fixture instance with the given QObject parent.
	 */
	Fixture(QObject* parent);

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
public:
	/**
	 * Change the fixture instance's fixture ID. This is generally VERY
	 * dangerous, since all functions (using this fixture) will cease to
	 * work unless their fixture references are changed as well. Also, the
	 * fixture will have to be moved within Doc's fixture array to reflect
	 * the new ID, which is essentially the fixture's position in the array.
	 *
	 * @param id The new fixture id
	 */
	void setID(t_fixture_id id);

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
	void setFixtureDefinition(const QLCFixtureDef* fixtureDef,
				  const QLCFixtureMode* fixtureMode);

	/**
	 * Get the fixture definition that this fixture instance is based on.
	 *
	 * @return A QLCFixture definition
	 */
	const QLCFixtureDef* fixtureDef() const { return m_fixtureDef; }

	/**
	 * Get the fixture mode that this fixture instance is based on.
	 *
	 * @return A QLCFixtureMode definition
	 */
	const QLCFixtureMode* fixtureMode() const { return m_fixtureMode; }

protected:
	/** The fixture definition that this instance is based on */
	const QLCFixtureDef* m_fixtureDef;

	/** The mode within the fixture definition that this instance uses */
	const QLCFixtureMode* m_fixtureMode;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Load a new fixture from an XML tag and add it to the given doc
	 * object, if loading was successful.
	 *
	 * @param root An XML subtree containing a single fixture instance
	 * @param doc The QLC document object, that owns all fixtures
	 * @param fixtureDefCache Get the fixture's definition from this cache
	 */
	static void loader(const QDomElement* root, Doc* doc,
			   const QLCFixtureDefCache& fixtureDefCache);

	/**
	 * Load a fixture's contents from the given XML node.
	 *
	 * @param root An XML subtree containing a single fixture instance
	 * @return true if the fixture was loaded successfully, otherwise false
	 */
	bool loadXML(const QDomElement* root,
		     const QLCFixtureDefCache& fixtureDefCache);

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
	 * Signals
	 *********************************************************************/
signals:
	void changed(t_fixture_id);
};

#endif

