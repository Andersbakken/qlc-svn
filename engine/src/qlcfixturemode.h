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

/**
 * QLCFixtureMode is essentially a collection of QLCChannels, arranged in such
 * order that they represent the channel configuration of an actual fixture.
 * Damn that sentence was hard to formulate... In many simple cases, fixtures
 * have only one mode and therefore a separate QLCFixtureMode is rather useless.
 * However, since many fixtures DO use different modes (16bit & 8bit movement
 * modes have a different number of channels etc..) this class is very much
 * needed to prevent the need for having to create a separate definition files
 * for each different mode. To ease the user in selecting the proper mode,
 * each mode can also have a name.
 *
 * Since fixture modes represent different settings for a fixture, it is only
 * natural to assume that also the physical properties of a fixture can be
 * changed. Therefore, each QLCFixtureMode contains also a QLCPhysical object
 * that defines the physical properties of a fixture in a certain mode.
 *
 * QLCFixtureDef owns the channel instances and deletes them when it is deleted
 * itself. QLCFixtureModes do not delete their channels because they might be
 * shared between multiple modes.
 */
class QLCFixtureMode
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
    virtual ~QLCFixtureMode();

    /** Assignment operator */
    QLCFixtureMode& operator=(const QLCFixtureMode& mode);

    /*********************************************************************
     * Name
     *********************************************************************/
public:
    /** Set the name of the fixture mode */
    void setName(const QString &name);

    /** Get the name of the fixture mode */
    QString name() const;

protected:
    QString m_name;

    /*********************************************************************
     * Fixture definition
     *********************************************************************/
public:
    /** Get the fixture that this mode is associated to */
    QLCFixtureDef* fixtureDef() const;

protected:
    QLCFixtureDef* m_fixtureDef;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /**
     * Insert a channel at the given position. The channel must belong
     * to m_fixtureDef or it won't get added to the mode. Each channel can
     * occupy exactly one index in a mode.
     *
     * @param channel The channel to add
     * @param index The position to insert the channel at
     * @return true, if successful, otherwise false
     */
    bool insertChannel(QLCChannel* channel, quint32 index);

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
    QLCChannel* channel(quint32 ch) const;

    /**
     * Get an ordered list of channels in a mode. Returns a copy of the list;
     * Any modifications to the list won't end up in the mode, but
     * modifications to channels are possible (discouraged).
     *
     * @return A list of channels in the mode.
     */
    QList <QLCChannel*> channels() const;

    /**
     * Get a channel's index (i.e. the DMX channel number) within a mode.
     * If the channel is not part of the mode, KChannelInvalid is returned.
     *
     * @param channel The channel, whose number to get
     * @return Channel number or KChannelInvalid
     */
    quint32 channelNumber(QLCChannel* channel) const;

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
    QLCPhysical physical() const;

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
