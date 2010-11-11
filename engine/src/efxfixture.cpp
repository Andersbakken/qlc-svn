/*
  Q Light Controller
  efxfixture.cpp

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

#include <QDebug>
#include <math.h>

#include "universearray.h"
#include "efxfixture.h"
#include "function.h"
#include "scene.h"
#include "efx.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(EFX* parent)
{
    Q_ASSERT(parent != NULL);
    m_parent = parent;

    m_fixture = Fixture::invalidId();
    m_serialNumber = 0;
    m_direction = Function::Forward;
    m_runTimeDirection = Function::Forward;
    m_startScene = NULL;
    m_stopScene = NULL;
    m_initialized = false;
    m_ready = false;

    m_skipIterator = 0;
    m_skipThreshold = 0;
    m_iterator = 0;
    m_panValue = 0;
    m_tiltValue = 0;

    m_lsbPanChannel = QLCChannel::invalid();
    m_msbPanChannel = QLCChannel::invalid();
    m_lsbTiltChannel = QLCChannel::invalid();
    m_msbTiltChannel = QLCChannel::invalid();
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    /* Don't copy m_parent because it is assigned in constructor. */

    m_fixture = ef->m_fixture;
    m_serialNumber = ef->m_serialNumber;
    m_direction = ef->m_direction;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_startScene = ef->m_startScene;
    m_stopScene = ef->m_stopScene;
    m_initialized = ef->m_initialized;
    m_ready = ef->m_ready;

    m_skipIterator = ef->m_skipIterator;
    m_skipThreshold = ef->m_skipThreshold;
    m_iterator = ef->m_iterator;
    m_panValue = ef->m_panValue;
    m_tiltValue = ef->m_tiltValue;

    m_lsbPanChannel = ef->m_lsbPanChannel;
    m_msbPanChannel = ef->m_msbPanChannel;
    m_lsbTiltChannel = ef->m_lsbTiltChannel;
    m_msbTiltChannel = ef->m_msbTiltChannel;
}

EFXFixture::~EFXFixture()
{
}

/****************************************************************************
 * Public properties
 ****************************************************************************/

void EFXFixture::setFixture(t_fixture_id fxi_id)
{
    m_fixture = fxi_id;
}

t_fixture_id EFXFixture::fixture() const
{
    return m_fixture;
}

void EFXFixture::setDirection(Function::Direction dir)
{
    m_direction = dir;
    m_runTimeDirection = dir;
}

Function::Direction EFXFixture::direction() const
{
    return m_direction;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFXFixture::loadXML(const QDomElement* root)
{
    QDomElement tag;
    QDomNode node;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCEFXFixture)
    {
        qWarning("EFX Fixture node not found!");
        return false;
    }

    /* New file format contains sub tags */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCEFXFixtureID)
        {
            /* Fixture ID */
            setFixture(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureDirection)
        {
            /* Direction */
            Function::Direction dir;
            dir = Function::stringToDirection(tag.text());
            setDirection(dir);
        }
        else
        {
            qWarning() << "Unknown EFX Fixture tag:"
            << tag.tagName();
        }
        node = node.nextSibling();
    }

    return true;
}

bool EFXFixture::saveXML(QDomDocument* doc, QDomElement* efx_root) const
{
    QDomElement subtag;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(efx_root != NULL);

    /* EFXFixture */
    tag = doc->createElement(KXMLQLCEFXFixture);
    efx_root->appendChild(tag);

    /* Fixture ID */
    subtag = doc->createElement(KXMLQLCEFXFixtureID);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString("%1").arg(fixture()));
    subtag.appendChild(text);

    /* Direction */
    subtag = doc->createElement(KXMLQLCEFXFixtureDirection);
    tag.appendChild(subtag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    subtag.appendChild(text);

    return true;
}

/****************************************************************************
 * Protected run-time-only properties
 ****************************************************************************/

void EFXFixture::setSerialNumber(int number)
{
    m_serialNumber = number;
}

int EFXFixture::serialNumber() const
{
    return m_serialNumber;
}

void EFXFixture::setStartScene(Scene* scene)
{
    m_startScene = scene;
}

Scene* EFXFixture::startScene() const
{
    return m_startScene;
}

void EFXFixture::setStopScene(Scene* scene)
{
    m_stopScene = scene;
}

Scene* EFXFixture::stopScene() const
{
    return m_stopScene;
}

void EFXFixture::setLsbPanChannel(quint32 ch)
{
    m_lsbPanChannel = ch;
}

void EFXFixture::setMsbPanChannel(quint32 ch)
{
    m_msbPanChannel = ch;
}

void EFXFixture::setLsbTiltChannel(quint32 ch)
{
    m_lsbTiltChannel = ch;
}

void EFXFixture::setMsbTiltChannel(quint32 ch)
{
    m_msbTiltChannel = ch;
}

void EFXFixture::updateSkipThreshold()
{
    Q_ASSERT(m_parent != NULL);
    Q_ASSERT(m_parent->fixtures().size() > 0);

    /* One EFX "round" is always (pi * 2) long. Divide this "circumference"
       into as many steps as there are fixtures in this EFX. If there are
       four fixtures, these steps end up in 12 o'clock, 3 o'clock,
       6 o'clock and 9 o'clock etc.. This skip threshold will specify the
       point in the circumference, where the current EFXFixture will come
       along to the EFX. */
    m_skipThreshold = qreal(m_serialNumber) *
                      (qreal(M_PI * 2.0) / qreal(m_parent->fixtures().size()));
}

bool EFXFixture::isValid()
{
    if (m_msbPanChannel != QLCChannel::invalid() &&
            m_msbTiltChannel != QLCChannel::invalid() &&
            m_fixture != Fixture::invalidId())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void EFXFixture::reset()
{
    m_panValue = 0;
    m_tiltValue = 0;
    m_skipIterator = 0;
    m_iterator = 0;
    m_initialized = false;
    m_ready = false;
    m_runTimeDirection = m_direction;

    updateSkipThreshold();
}

bool EFXFixture::isReady() const
{
    return m_ready;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::nextStep(UniverseArray* universes)
{
    /* Bail out without doing anything if this EFX is ready
      (after single-shot), or it has no pan&tilt channels (not valid). */
    if (m_ready == true || isValid() == false)
        return;

    if (m_iterator == 0)
        updateSkipThreshold();

    if (m_parent->propagationMode() == EFX::Serial &&
            m_skipIterator < m_skipThreshold)
    {
        /* Fixture still needs to wait for its turn in serial mode */
        m_skipIterator += m_parent->m_stepSize;
    }
    else
    {
        if (m_initialized == false)
        {
            /* This fixture is now running. Initialize it. */
            m_initialized = true;
            start(universes);
        }
    }

    if (m_iterator < (M_PI * 2.0))
    {
        if (m_parent->propagationMode() != EFX::Serial ||
            m_skipIterator >= m_skipThreshold)
        {
            /* Increment for next round. TODO: This check is made
               twice (the other is just a couple of lines above,
               reversed). */
            m_iterator += m_parent->m_stepSize;
        }

        if (m_runTimeDirection == Function::Forward)
        {
            m_parent->calculatePoint(m_iterator, &m_panValue, &m_tiltValue);
        }
        else
        {
            m_parent->calculatePoint((M_PI * 2.0) - m_iterator,
                                     &m_panValue, &m_tiltValue);
        }

        /* Write this fixture's data to universes. */
        setPoint(universes);
    }
    else
    {
        if (m_parent->m_runOrder == Function::PingPong)
        {
            /* Reverse direction for ping-pong EFX. */
            if (m_runTimeDirection == Function::Forward)
            {
                m_runTimeDirection = Function::Backward;
            }
            else
            {
                m_runTimeDirection = Function::Forward;
            }
        }
        else if (m_parent->m_runOrder == Function::SingleShot)
        {
            /* De-initialize the fixture and mark as ready. */
            m_ready = true;
            stop(universes);
        }

        /* Reset iterator, since we've gone a full cycle. */
        m_iterator = 0;
    }
}

void EFXFixture::start(UniverseArray* universes)
{
    if (m_startScene != NULL)
        m_startScene->writeValues(universes, m_fixture);
}

void EFXFixture::stop(UniverseArray* universes)
{
    if (m_stopScene != NULL)
        m_stopScene->writeValues(universes, m_fixture);
}

void EFXFixture::setPoint(UniverseArray* universes)
{
    Q_ASSERT(universes != NULL);

    /* Write coarse point data to universes */
    universes->write(m_msbPanChannel, static_cast<char> (m_panValue),
                     QLCChannel::Pan);
    universes->write(m_msbTiltChannel, static_cast<char> (m_tiltValue),
                     QLCChannel::Tilt);

    /* Write fine point data to universes if applicable */
    if (m_lsbPanChannel != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((m_panValue - floor(m_panValue))
                                        * double(UCHAR_MAX));
        universes->write(m_lsbPanChannel, value, QLCChannel::Pan);
    }

    if (m_lsbTiltChannel != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((m_tiltValue - floor(m_tiltValue))
                                        * double(UCHAR_MAX));
        universes->write(m_lsbTiltChannel, value, QLCChannel::Tilt);
    }
}
