/*
  Q Light Controller
  efx.h

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

#ifndef EFX_H
#define EFX_H

#include <QVector>
#include <QPoint>
#include <QList>

#include "qlctypes.h"

#include "efxfixture.h"
#include "function.h"

class QDomDocument;
class QDomElement;
class QString;
class Fixture;

#define KXMLQLCEFXPropagationMode "PropagationMode"
#define KXMLQLCEFXPropagationModeParallel "Parallel"
#define KXMLQLCEFXPropagationModeSerial "Serial"
#define KXMLQLCEFXAlgorithm "Algorithm"
#define KXMLQLCEFXWidth "Width"
#define KXMLQLCEFXHeight "Height"
#define KXMLQLCEFXRotation "Rotation"
#define KXMLQLCEFXAxis "Axis"
#define KXMLQLCEFXOffset "Offset"
#define KXMLQLCEFXFrequency "Frequency"
#define KXMLQLCEFXPhase "Phase"
#define KXMLQLCEFXChannel "Channel"
#define KXMLQLCEFXX "X"
#define KXMLQLCEFXY "Y"
#define KXMLQLCEFXStartScene "StartScene"
#define KXMLQLCEFXStopScene "StopScene"

#define KXMLQLCEFXCircleAlgorithmName "Circle"
#define KXMLQLCEFXEightAlgorithmName "Eight"
#define KXMLQLCEFXLineAlgorithmName "Line"
#define KXMLQLCEFXDiamondAlgorithmName "Diamond"
#define KXMLQLCEFXLissajousAlgorithmName "Lissajous"

/**
 * An EFX (effects) function that is used to create
 * more complex automation especially for moving lights
 */
class EFX : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(EFX)

    friend class EFXFixture;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    EFX(Doc* doc);
    ~EFX();

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

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Algorithm
     *********************************************************************/
public:
    enum Algorithm
    {
        Circle,
        Eight,
        Line,
        Diamond,
        Lissajous
    };

    /** Get the current algorithm */
    Algorithm algorithm() const;

    /** Set the current algorithm */
    void setAlgorithm(Algorithm algo);

    /** Get the supported algorithms in a string list */
    static QStringList algorithmList();

    /** Convert an algorithm type to a string */
    static QString algorithmToString(Algorithm algo);

    /** Convert a string to an algorithm type */
    static Algorithm stringToAlgorithm(const QString& str);

    /**
     * Get a preview of the current algorithm. Puts 128 points to the
     * given polygon, 255px wide and 255px high at maximum, that represent
     * roughly the path of the pattern on a flat surface directly in front
     * of a moving (head/mirror) fixture.
     *
     * @param polygon The polygon to fill with preview points
     */
    bool preview(QVector <QPoint>& polygon) const;

    /**
     * Calculate a single point with the currently selected algorithm,
     * based on the value of iterator (which is basically a step number).
     *
     * @param iterator Step number (input)
     * @param x Used to store the calculated X coordinate (output)
     * @param y Used to store the calculated Y coordinate (output)
     */
    void calculatePoint(qreal iterator, qreal* x, qreal* y) const;

    /**
     * Rotate a point of the pattern by rot degrees and scale the point
     * within w/h and xOff/yOff.
     *
     * @param x Holds the calculated X coordinate
     * @param y Holds the calculated Y coordinate
     * @param w The width to scale to
     * @param h The height to scale to
     * @param xOff X offset of the pattern
     * @param yOff Y offset of the pattern
     * @param rotation Degrees to rotate
     */
    void rotateAndScale(qreal *x, qreal *y) const;

protected:
    /** Current algorithm used by the EFX */
    Algorithm m_algorithm;

    /*********************************************************************
     * Width
     *********************************************************************/
public:
    /**
     * Set the pattern width
     *
     * @param width Pattern width (0-255)
     */
    void setWidth(int width);

    /**
     * Get the pattern width
     *
     * @return Pattern width (0-255)
     */
    int width() const;

protected:
    /**
     * Pattern width, see setWidth()
     */
    qreal m_width;

    /*********************************************************************
     * Height
     *********************************************************************/
public:
    /**
     * Set the pattern height
     *
     * @param height Pattern height (0-255)
     */
    void setHeight(int height);

    /**
     * Get the pattern height
     *
     * @return Pattern height (0-255)
     */
    int height() const;

protected:
    /**
     * Pattern height, see setHeight()
     */
    qreal m_height;

    /*********************************************************************
     * Rotation
     *********************************************************************/
public:
    /**
     * Set the pattern rotation
     *
     * @param rot Pattern rotation (0-359)
     */
    void setRotation(int rot);

    /**
     * Get the pattern rotation
     *
     * @return Pattern rotation (0-359)
     */
    int rotation() const;

protected:
    /**
     * Pattern rotation, see setRotation()
     */
    int m_rotation;

    /*********************************************************************
     * Offset
     *********************************************************************/
public:
    /**
     * Set the pattern offset on the X-axis
     *
     * @param offset Pattern offset (0-255; 127 is middle)
     */
    void setXOffset(int offset);

    /**
     * Get the pattern offset on the X-axis
     *
     * @return Pattern offset (0-255; 127 is middle)
     */
    int xOffset() const;

    /**
     * Set the pattern offset on the Y-axis
     *
     * @param offset Pattern offset (0-255; 127 is middle)
     */
    void setYOffset(int offset);

    /**
     * Get the pattern offset on the Y-axis
     *
     * @return Pattern offset (0-255; 127 is middle)
     */
    int yOffset() const;

protected:
    /**
     * Pattern X offset, see setXOffset()
     */
    qreal m_xOffset;

    /**
     * Pattern Y offset, see setXOffset()
     */
    qreal m_yOffset;

    /*********************************************************************
     * Frequency
     *********************************************************************/
public:
    /**
     * Set the lissajous pattern frequency on the X-axis
     *
     * @param freq Pattern frequency (0-5)
     */
    void setXFrequency(int freq);

    /**
     * Get the lissajous pattern frequency on the X-axis
     *
     * @return Pattern frequency (0-5)
     */
    int xFrequency() const;

    /**
     * Set the lissajous pattern frequency on the Y-axis
     *
     * @param freq Pattern frequency (0-5)
     */
    void setYFrequency(int freq);

    /**
     * Get the lissajous pattern frequency on the Y-axis
     *
     * @return Pattern frequency (0-5)
     */
    int yFrequency() const;

    /**
     * Returns true when lissajous has been selected
     */
    bool isFrequencyEnabled();

protected:
    /**
     * Lissajous pattern X frequency, see setXFrequency()
     */
    qreal m_xFrequency;

    /**
     * Lissajous pattern Y frequency, see setYFrequency()
     */
    qreal m_yFrequency;

    /*********************************************************************
     * Phase
     *********************************************************************/
public:
    /**
     * Set the lissajous pattern phase on the X-axis
     *
     * @param phase Pattern phase (0-359)
     */
    void setXPhase(int phase);

    /**
     * Get the lissajous pattern phase on the X-axis
     *
     * @return Pattern phase (0-359)
     */
    int xPhase() const;

    /**
     * Set the lissajous pattern phase on the Y-axis
     *
     * @param phase Pattern phase (0-359)
     */
    void setYPhase(int phase);

    /**
     * Get the lissajous pattern phase on the Y-axis
     *
     * @return Pattern phase (0-359)
     */
    int yPhase() const;

    /**
     * Returns true when lissajous has been selected
     */
    bool isPhaseEnabled() const;

protected:
    /**
     * Lissajous pattern X phase, see setXPhase()
     */
    qreal m_xPhase;

    /**
     * Lissajous pattern Y phase, see setYPhase()
     */
    qreal m_yPhase;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public:
    /** Add a new fixture to this EFX */
    bool addFixture(EFXFixture* ef);

    /** Remove the designated fixture from this EFX but don't delete it */
    bool removeFixture(EFXFixture* ef);

    /** Raise a fixture in the serial order to an earlier position */
    bool raiseFixture(EFXFixture* ef);

    /** Lower a fixture in the serial order to a later position */
    bool lowerFixture(EFXFixture* ef);

    /** Get a list of fixtures taking part in this EFX */
    const QList <EFXFixture*> fixtures() const;

public slots:
    /** Slot that captures Doc::fixtureRemoved signals */
    void slotFixtureRemoved(t_fixture_id fxi_id);

protected:
    QList <EFXFixture*> m_fixtures;

    /*********************************************************************
     * Fixture propagation mode
     *********************************************************************/
public:
    /**
     * Parallel mode means that all fixtures move exactly like others.
     * Serial mode means that fixtures start moving one after the other,
     * a bit delayed, creating a more dynamic effect.
     */
    enum PropagationMode { Parallel, Serial };

    /** Set the EFX's fixture propagation mode (see the enum above) */
    void setPropagationMode(PropagationMode mode);

    /** Get the EFX's fixture propagation mode */
    PropagationMode propagationMode() const;

    /** Convert the propagation mode setting to a string */
    static QString propagationModeToString(PropagationMode mode);

    /** Convert a string to a propagation mode setting */
    static PropagationMode stringToPropagationMode(QString str);

protected:
    PropagationMode m_propagationMode;

    /*********************************************************************
     * Start & Stop Scenes
     *********************************************************************/
public:
    /** Set the start scene */
    void setStartScene(t_function_id id);

    /** Get the start scene */
    t_function_id startScene() const;

    /** Set start scene enabled status */
    void setStartSceneEnabled(bool set);

    /** Get start scene enabled status */
    bool startSceneEnabled() const;

    /** Set the stop scene */
    void setStopScene(t_function_id id);

    /** Get the stop scene */
    t_function_id stopScene() const;

    /** Set stop scene enabled status */
    void setStopSceneEnabled(bool set);

    /** Get stop scene enabled status */
    bool stopSceneEnabled() const;

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(t_function_id function);

protected:
    /** Start (initialisation) scene */
    t_function_id m_startSceneID;
    bool m_startSceneEnabled;

    /** Stop (de-initialisation) scene */
    t_function_id m_stopSceneID;
    bool m_stopSceneEnabled;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
    bool loadXML(const QDomElement* root);

protected:
    /** Load an axis' contents from an XML document*/
    bool loadXMLAxis(const QDomElement* root);

    /*********************************************************************
     * Bus
     *********************************************************************/
public slots:
    /**
     * This is called by buses for each function when the
     * bus value is changed.
     *
     * @param id ID of the bus that has changed its value
     * @param value Bus' new value
     */
    void slotBusValueChanged(quint32 id, quint32 value);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void arm();

    /** @reimpl */
    void disarm();

    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

protected:
    /**
     * The size of one step derived from m_cycleDuration. If m_cycleDuration
     * is 64, then this is 1/64.
     */
    qreal m_stepSize;
};

#endif
