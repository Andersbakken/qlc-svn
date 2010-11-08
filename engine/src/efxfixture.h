/*
  Q Light Controller
  efxfixture.h

  Copyright (c) Heikki Junnila

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

#ifndef EFXFIXTURE_H
#define EFXFIXTURE_H

#include "qlctypes.h"
#include "function.h"

class EFXFixture;
class Scene;
class EFX;

#define KXMLQLCEFXFixture "Fixture"
#define KXMLQLCEFXFixtureID "ID"
#define KXMLQLCEFXFixtureDirection "Direction"

class EFXFixture
{
    friend class EFX;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Constructor */
    EFXFixture(EFX* parent);

    /** Destructor */
    ~EFXFixture();

    /** Copy contents from another EFXFixture */
    void copyFrom(const EFXFixture* ef);

protected:
    /** The EFX function that this fixture belongs to */
    EFX* m_parent;

    /********************************************************************
     * Public properties
     ********************************************************************/
public:
    /** Set the fixture that this EFXFixture represents */
    void setFixture(t_fixture_id fxi_id);

    /** Get the fixture that this EFXFixture represents */
    t_fixture_id fixture() const;

    /** Set this fixture's direction */
    void setDirection(Function::Direction dir);

    /** Get this fixture's direction */
    Function::Direction direction() const;

protected:
    /** The ID of the fixture this EFXFixture represents */
    t_fixture_id m_fixture;

    /** This fixture's original running direction */
    Function::Direction m_direction;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load public properties from an EFXFixture node */
    bool loadXML(const QDomElement* root);

    /** Save public properties under an EFX node */
    bool saveXML(QDomDocument* doc, QDomElement* efx_root) const;

    /********************************************************************
    * Protected run-time-only properties
    ********************************************************************/
protected:
    /** Set the order number in serial propagation mode */
    void setSerialNumber(int number);

    /** Get the order number in serial propagation mode */
    int serialNumber() const;

    /** Set the scene that is used to initialize the fixture */
    void setStartScene(Scene* scene);

    /** Get the scene that is used to initialize the fixture */
    Scene* startScene() const;

    /** Set the scene that is used to de-initialize the fixture */
    void setStopScene(Scene* scene);

    /** Get the scene that is used to de-initialize the fixture */
    Scene* stopScene() const;

    /** Set the low byte channel for pan movement */
    void setLsbPanChannel(quint32 ch);

    /** Set the high byte channel for pan movement */
    void setMsbPanChannel(quint32 ch);

    /** Set the low byte channel for tilt movement */
    void setLsbTiltChannel(quint32 ch);

    /** Set the high byte channel for pan movement */
    void setMsbTiltChannel(quint32 ch);

    /** Update the waiting threshold value for serial operation */
    void updateSkipThreshold();

    /** Check that this object has a fixture ID and at least LSB channels
        for pan and tilt. */
    bool isValid();

    /* Run the start scene if necessary */
    void start(UniverseArray* universes);

    /* Run the stop scene if necessary */
    void stop(UniverseArray* universes);

    /** Reset the fixture when the EFX is stopped */
    void reset();

    /** Check, whether this EFXFixture is ready (no more events).
        This can happen basically only if SingleShot mode is enabled. */
    bool isReady() const;

protected:
    /** This fixture's order number in serial propagation mode */
    int m_serialNumber;

    /** This fixture's current run-time direction */
    Function::Direction m_runTimeDirection;

    /** The scene that is used to initialize the fixtures involved */
    Scene* m_startScene;

    /** The scene that is used to de-initialize the fixtures involved */
    Scene* m_stopScene;

    /** When the start scene is run, the EFXFixture has been initialized */
    bool m_initialized;

    /** When running in single shot mode, the fixture is marked ready
        after it has completed a full cycle. */
    bool m_ready;

    /**
     * This fixture's current position in the pattern (a point on a
     * circle's circumference)
     */
    qreal m_iterator;

    /**
     * This iterator is incremented until it is >= m_skipThreshold.
     * After that, m_iterator is incremented. Used for serial propagation.
     */
    qreal m_skipIterator;

    /**
     * This is basically the index of a point in the EFX's pattern,
     * where this fixture will start doing its stuff. Used for serial
     * propagation.
     */
    qreal m_skipThreshold;

    /**
     * The current pan value
     */
    qreal m_panValue;

    /**
     * The current tilt value
     */
    qreal m_tiltValue;

    /**
     * Universe channel for LSB pan data
     */
    quint32 m_lsbPanChannel;

    /**
     * Universe channel for MSB pan data
     */
    quint32 m_msbPanChannel;

    /**
     * Universe channel for LSB tilt data
     */
    quint32 m_lsbTiltChannel;

    /**
     * Universe channel for MSB tilt data
     */
    quint32 m_msbTiltChannel;

    /*********************************************************************
     * Running
     *********************************************************************/
protected:
    /** Calculate the next step data for this fixture */
    void nextStep(UniverseArray* universes);

    /** Write this EFXFixture's channel data to universes */
    void setPoint(UniverseArray* universes);
};

#endif
