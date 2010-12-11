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

#include "fadechannel.h"
#include "scenevalue.h"
#include "dmxsource.h"
#include "qlctypes.h"
#include "function.h"
#include "fixture.h"

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

    /**
     * Clear all values
     */
    void clear();

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
    QList <FadeChannel> armedChannels() const;

protected:
    QList <FadeChannel> m_armedChannels;
};

#endif
