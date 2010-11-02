/*
  Q Light Controller
  vcxypadfixture.h

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

#ifndef VCXYPADFIXTURE
#define VCXYPADFIXTURE

#include <QStringList>
#include <QVariant>
#include <QString>

#include "qlctypes.h"

class VCXYPadFixture;
class UniverseArray;
class QDomDocument;
class QDomElement;

#define KXMLQLCVCXYPadFixture "Fixture"
#define KXMLQLCVCXYPadFixtureID "ID"

#define KXMLQLCVCXYPadFixtureAxis "Axis"
#define KXMLQLCVCXYPadFixtureAxisID "ID"
#define KXMLQLCVCXYPadFixtureAxisX "X"
#define KXMLQLCVCXYPadFixtureAxisY "Y"
#define KXMLQLCVCXYPadFixtureAxisLowLimit "LowLimit"
#define KXMLQLCVCXYPadFixtureAxisHighLimit "HighLimit"
#define KXMLQLCVCXYPadFixtureAxisReverse "Reverse"

class VCXYPadFixture
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadFixture();
    VCXYPadFixture(const QVariant& variant);
    VCXYPadFixture(const VCXYPadFixture& vc_fxi);
    ~VCXYPadFixture();

    /** Assignment operator */
    VCXYPadFixture& operator=(const VCXYPadFixture& fxi);

    /** Comparing operator */
    bool operator==(const VCXYPadFixture& fxi);

    /* Serialization operator for VCXYPadFixtureEditor */
    operator QVariant() const;

    /********************************************************************
     * Fixture
     ********************************************************************/
public:
    void setFixture(t_fixture_id fxi_id);
    t_fixture_id fixture() const {
        return m_fixture;
    }

    QString name() const;

    void arm();
    void disarm();

    /** Write the value using x & y multipliers for the actual range */
    void writeDMX(double xmul, double ymul, UniverseArray* universes);

protected:
    t_fixture_id m_fixture;

    /********************************************************************
     * X-Axis
     ********************************************************************/
public:
    void setX(double min, double max, bool reverse);

    double xMin() const {
        return m_xMin;
    }
    double xMax() const {
        return m_xMax;
    }
    bool xReverse() const {
        return m_xReverse;
    }

    /** min% - max% for displaying X limits in tree widget */
    QString xBrief() const;

protected:
    double m_xMin;
    double m_xMax;
    bool m_xReverse;

    quint32 m_xLSB;
    quint32 m_xMSB;

    /********************************************************************
     * Y-Axis
     ********************************************************************/
public:
    void setY(double min, double max, bool reverse);

    double yMin() const {
        return m_yMin;
    }
    double yMax() const {
        return m_yMax;
    }
    bool yReverse() const {
        return m_yReverse;
    }

    /** min% - max% for displaying Y limits in tree widget */
    QString yBrief() const;

protected:
    double m_yMin;
    double m_yMax;
    bool m_yReverse;

    quint32 m_yLSB;
    quint32 m_yMSB;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool saveXML(QDomDocument* doc, QDomElement* root);
};

#endif
