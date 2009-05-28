/*
  Q Light Controller
  qlcfixturemode.h

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

#ifndef QLCFIXTUREMODE_H
#define QLCFIXTUREMODE_H

#include <QString>
#include <QList>

#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlctypes.h"

#define KXMLQLCFixtureMode              "Mode"
#define KXMLQLCFixtureModeName          "Name"
#define KXMLQLCFixtureModeChannel       "Channel"
#define KXMLQLCFixtureModeChannelNumber "Number"

class QDomDocument;
class QDomElement;
class QLCFixtureMode;
class QLCFixtureDef;
class QLCPhysical;
class QLCChannel;

class QLC_DECLSPEC QLCFixtureMode
{
public:
	/**
	 * Create a new QLCFixtureMode for the given QLCFixtureDef. Added
	 * channels must belong to the fixture definition.
	 *
	 * @param fixtureDef The parent fixture definition
	 */
	QLCFixtureMode(QLCFixtureDef* fixtureDef);

	/**
	 * Create a copy of the given mode, taking channels from the given
	 * parent fixture definition.
	 *
	 * @param fixtureDef The parent fixture definition, who owns channels
	 *                   that belong to this mode.
	 * @param mode The mode to copy
	 */
	QLCFixtureMode(QLCFixtureDef* fixtureDef, const QLCFixtureMode* mode);

	/** Create contents from an XML tag */
	QLCFixtureMode(QLCFixtureDef* fixtureDef, const QDomElement* tag);

	/** Destructor */
	~QLCFixtureMode();

	/** Assignment operator */
	QLCFixtureMode& operator=(const QLCFixtureMode& mode);

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	void setName(const QString &name) { m_name = name; }
	QString name() const { return m_name; }

protected:
	QString m_name;

	/*********************************************************************
	 * Fixture definition
	 *********************************************************************/
public:
	/** Get the fixture that this mode is associated to */
	QLCFixtureDef* fixtureDef() const { return m_fixtureDef; }

protected:
	QLCFixtureDef* m_fixtureDef;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/**
	 * Insert a channel at the given position. The channel must belong
	 * to m_fixtureDef or it won't get added to the mode.
	 *
	 * @param channel The channel to add
	 * @param index The position to insert the channel at
	 */
	void insertChannel(QLCChannel* channel, t_channel index);

	/**
	 * Remove a channel from this mode. The channel is only removed from
	 * m_channels list, but it's not deleted, since it might be used by
	 * other modes, and in any case, the fixtureDef owns the channel.
	 *
	 * @param channel The channel to remove
	 * @return true if the channel was found and removed. Otherwise false.
	 */
	bool removeChannel(const QLCChannel* channel);

	/**
	 * Get a channel by its name. If there are more than one channels with
	 * the same name in a mode, only the first one is returned (although
	 * channel names should be unique and this should never happen).
	 *
	 * @param name The name of the channel to get
	 * @return The channel or NULL if not found
	 */
	QLCChannel* channel(const QString& name) const;

	/**
	 * Get a channel by its index (channel number). One DMX channel is
	 * represented by exactly one QLCChannel.
	 *
	 * @param ch The number of the channel to get
	 * @return The channel or NULL if ch >= size.
	 */
	QLCChannel* channel(t_channel ch) const;

	/**
	 * Get a list of channels in a mode. Returns a shallow copy of the
	 * list. Any modifications to the list won't end up in the mode.
	 *
	 * @return A list of channels in the mode.
	 */
	QList <QLCChannel*> channels() const { return m_channels; }

	/**
	 * Get a channel's index (i.e. the DMX channel number) within a mode.
	 * If the channel is not part of the mode, KChannelInvalid is returned.
	 *
	 * @param channel The channel, whose number to get
	 * @return Channel number or KChannelInvalid
	 */
	t_channel channelNumber(QLCChannel* channel) const;

protected:
	/** List of channels (not owned) */
	QList <QLCChannel*> m_channels;

	/*********************************************************************
	 * Physical
	 *********************************************************************/
public:
	/**
	 * Set a mode's physical properties.
	 *
	 * @param physical Properties to set
	 */
	void setPhysical(const QLCPhysical& physical);

	/**
	 * Get physical properties for a mode. The returned properties is a
	 * copy of the original, so any modifications won't end up in the mode.
	 *
	 * @return Mode's physical properties
	 */
	QLCPhysical physical();

protected:
	QLCPhysical m_physical;

	/*********************************************************************
	 * Physical
	 *********************************************************************/
public:
	/** Load a mode's properties from an XML tag */
	bool loadXML(const QDomElement* root);

	/** Save a mode to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* root);
};

#endif
