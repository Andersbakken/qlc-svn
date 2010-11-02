/*
  Q Light Controller
  scene.h

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

#ifndef SCENE_H
#define SCENE_H

#include <QList>
#include <QtXml>

#include "dmxsource.h"
#include "qlctypes.h"
#include "function.h"
#include "fixture.h"

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

/*****************************************************************************
 * SceneChannel
 *****************************************************************************/

/**
 * SceneChannel is a helper class used to store individual RUNTIME values for
 * channels as they are operated by a Scene function during Operate mode.
 *
 * Unlike SceneValue, a SceneChannel consists of an absolute DMX address,
 * starting channel value, target value and channel's current value.
 * SceneChannels are used only during Operate mode, when fixture addresses
 * cannot change anymore, so it is slightly more efficient to store absolute
 * DMX addresses than relative channel numbers as in SceneValue. When a Scene
 * starts, it takes the current values of all of its channels and stores them
 * into their respective SceneChannels' $start variable. The Scene then
 * calculates how much time it still has until the values specified in $target
 * need to be set for each involved channel and adjusts the step size
 * accordingly. The more time there's left, the smoother the fade effect. Each
 * gradual step is written to the channels' DMX addresses and also stored to
 * SceneChannels' $current variable for the next round. When $current == $target
 * the SceneChannel is seen ready and won't be touched any longer (until the
 * rest of the channels are ready as well).
 *
 * Although uchar is an UNSIGNED char (0-255), these variables must be SIGNED
 * because the floating-point calculations that Scene does don't necessarily
 * stop exactly at 0.0 and 255.0, but might go slightly over/under. If these
 * variables were unsigned, an overflow would occur in some cases, resulting in
 * UINT_MAX values and everything would go wacko.
 */
class SceneChannel
{
public:
    SceneChannel();
    SceneChannel(const SceneChannel& sch);
    ~SceneChannel();

public:
    /** The universe and channel that this object refers to */
    quint32 address;

    /** The channel group that this channel belongs to */
    QLCChannel::Group group;

    /** The value of the channel where a scene started fading from */
    qint32 start;

    /** The current value set by a scene */
    qint32 current;

    /** The target value to eventually fade to */
    qint32 target;
};

/*****************************************************************************
 * SceneValue
 *****************************************************************************/

/**
 * SceneValue is a helper class used to store individual channel TARGET values
 * for Scene functions. Each channel that is taking part in a scene is
 * represented with a SceneValue.
 *
 * A SceneValue consists of a fixture, channel and value. Fixture tells, which
 * fixture a particular value corresponds to, channel contains the particular
 * channel number from the fixture and, value tells the exact target value for
 * that channel. Channel numbers are not absolute DMX channels because the
 * fixture address can be changed at any time. Instead, channel number tells
 * the relative channel number, respective to fixture address.
 *
 * For example:
 * Let's say we have a SceneValue with channel = 5, value = 127 and, the
 * fixture assigned to the SceneValue is at DMX address 100. Thus, the
 * SceneValue represents value 127 for absolute DMX channel 105. If the address
 * of the fixture is changed, the SceneValue doesn't need to be touched at all.
 */
class SceneValue
{
public:
    /** Normal constructor */
    SceneValue(t_fixture_id fxi_id = Fixture::invalidId(),
               quint32 channel = KChannelInvalid,
               uchar value = 0);

    /** Copy constructor */
    SceneValue(const SceneValue& scv);

    /** Destructor */
    ~SceneValue();

    /** A SceneValue is not valid if .fxi == Fixture::invalidId() */
    bool isValid();

    /** Comparator function for qSort() */
    bool operator< (const SceneValue& scv) const;

    /** Comparator function for matching SceneValues */
    bool operator== (const SceneValue& scv) const;

    /** Load this SceneValue's contents from an XML tag */
    bool loadXML(const QDomElement* tag);

    /** Save this SceneValue to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* scene_root) const;

public:
    t_fixture_id fxi;
    quint32 channel;
    uchar value;
};

/*****************************************************************************
 * Scene
 *****************************************************************************/

/**
 * Scene encapsulates the values of selected channels from one or more fixture
 * instances. When a scene is started, the duration it takes for its channels
 * to reach their target values depends on the function's speed setting. Bus
 * number 1 (Default Fade) is assigned to all newly created scenes by default.
 * If the bus' value is 0 seconds, scene values are set immediately and no
 * fading occurs. Otherwise values are always faded from what they currently
 * are, to the target values defined in the scene (with SceneValue instances).
 * Channels that are not enabled in the scene will not be touched at all.
 */
class Scene : public Function, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(Scene)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Construct a new scene function, with given parent object. If the
     * parent is not a Doc* object, the debug build asserts.
     *
     * @param doc The parent object who owns the scene
     */
    Scene(Doc* doc);

    /**
     * Destroy the scene
     */
    ~Scene();

    /*********************************************************************
     * Function type
     *********************************************************************/
public:
    /** @reimpl */
    Function::Type type() const;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc);

    /** @reimpl */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Values
     *********************************************************************/
public:
    /**
     * Set the value of one fixture channel, using a predefined SceneValue
     */
    void setValue(const SceneValue& scv);

    /**
     * Set the value of one fixture channel, specify parameters separately
     */
    void setValue(t_fixture_id fxi, quint32 ch, uchar value);

    /**
     * Clear the value of one fixture channel
     */
    void unsetValue(t_fixture_id fxi, quint32 ch);

    /**
     * Get the value of one fixture channel
     */
    uchar value(t_fixture_id fxi, quint32 ch);

    /**
     * Get a list of values in this scene
     */
    QList <SceneValue> values() const;

protected:
    QList <SceneValue> m_values;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public slots:
    void slotFixtureRemoved(t_fixture_id fxi_id);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** @reimpl */
    bool loadXML(const QDomElement* root);

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** @reimpl */
    void flash(MasterTimer* timer);

    /** @reimpl */
    void unFlash(MasterTimer* timer);

    /** @reimpl from DMXSource */
    void writeDMX(MasterTimer* timer, UniverseArray* universes);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void arm();

    /** @reimpl */
    void disarm();

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /**
     * Write the scene values to OutputMap. If fxi_id is given, writes
     * values only for the specified fixture.
     *
     * The scene must be armed with arm() before calling this function or
     * otherwise nothing will be written.
     */
    virtual void writeValues(UniverseArray* universes,
                             t_fixture_id fxi_id = Fixture::invalidId());

    /**
     * Write zeros for all involved channels to OutputMap. If fxi_id is
     * given, writes zeros only for the specified fixture.
     *
     * The scene must be armed with arm() before calling this function or
     * otherwise nothing will be written.
     */
    virtual void writeZeros(UniverseArray* universes,
                            t_fixture_id fxi_id = Fixture::invalidId());

    /** Get a list of channels that have been armed for running */
    QList <SceneChannel> armedChannels() const {
        return m_armedChannels;
    }

protected:
    /** Calculate channel values for the next step. */
    uchar nextValue(SceneChannel* sch);

protected:
    QList <SceneChannel> m_armedChannels;
};

#endif
