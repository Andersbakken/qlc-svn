/*
  Q Light Controller
  chaserrunner.cpp

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

#include "universearray.h"
#include "chaserrunner.h"
#include "fadechannel.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

ChaserRunner::ChaserRunner(Doc* doc, QList <Function*> steps,
                           quint32 holdBusId,
                           Function::Direction direction,
                           Function::RunOrder runOrder,
                           QObject* parent)
    : QObject(parent)
    , m_doc(doc)
    , m_steps(steps)
    , m_holdBusId(holdBusId)
    , m_originalDirection(direction)
    , m_runOrder(runOrder)

    , m_autoStep(true)
    , m_direction(direction)
    , m_elapsed(0)
    , m_next(false)
    , m_previous(false)
    , m_currentStep(0)
    , m_newCurrent(-1)
{
    reset();
}

ChaserRunner::~ChaserRunner()
{
}

void ChaserRunner::next()
{
    m_next = true;
    m_previous = false;
}

void ChaserRunner::previous()
{
    m_next = false;
    m_previous = true;
}

void ChaserRunner::setCurrentStep(int step)
{
    if (step >= 0 && step < m_steps.size())
    {
        m_newCurrent = step;
        m_next = false;
        m_previous = false;
    }
}

int ChaserRunner::currentStep() const
{
    return m_currentStep;
}

void ChaserRunner::setAutoStep(bool autoStep)
{
    m_autoStep = autoStep;
}

bool ChaserRunner::isAutoStep() const
{
    return m_autoStep;
}

void ChaserRunner::reset()
{
    // Restore original direction since Ping-Pong switches m_direction
    m_direction = m_originalDirection;

    if (m_direction == Function::Backward)
        m_currentStep = m_steps.size() - 1;
    else
        m_currentStep = 0;
    m_elapsed = 0;
    m_next = false;
    m_previous = false;
    m_channelMap.clear();
}

bool ChaserRunner::write(UniverseArray* universes)
{
    // Nothing to do
    if (m_steps.size() == 0)
        return false;

    if (m_newCurrent != -1)
    {
        // Manually-set current step
        m_currentStep = m_newCurrent;
        m_newCurrent = -1;

        // No need to do roundcheck here, since manually-set steps are
        // always within m_steps limits.

        m_elapsed = 1;
        m_channelMap = createFadeChannels(universes, true);

        emit currentStepChanged(m_currentStep);
    }
    else if (m_elapsed == 0)
    {
        // First step
        m_elapsed = 1;
        m_channelMap = createFadeChannels(universes);

        emit currentStepChanged(m_currentStep);
    }
    else if ((isAutoStep() && m_elapsed >= Bus::instance()->value(m_holdBusId))
             || m_next == true || m_previous == true)
    {
        // Next step
        if (m_direction == Function::Forward)
        {
            // "Previous" for a forwards chaser is -1
            if (m_previous == true)
                m_currentStep--;
            else
                m_currentStep++;
        }
        else
        {
            // "Previous" for a backwards scene is +1
            if (m_previous == true)
                m_currentStep++;
            else
                m_currentStep--;
        }

        if (roundCheck() == false)
            return false;

        m_elapsed = 1;
        m_next = false;
        m_previous = false;
        m_channelMap = createFadeChannels(universes, true);

        emit currentStepChanged(m_currentStep);
    }
    else
    {
        // Current step
        m_elapsed++;
    }

    QMutableMapIterator <quint32,FadeChannel> it(m_channelMap);
    while (it.hasNext() == true)
    {
        Scene* scene = qobject_cast<Scene*> (m_steps.at(m_currentStep));
        if (scene == NULL)
            continue;

        quint32 fadeTime = Bus::instance()->value(scene->busID());

        FadeChannel& channel(it.next().value());
        if (channel.current() == channel.target() && channel.group() != QLCChannel::Intensity)
        {
            /* Write the final value to LTP channels only once */
        }
        else
        {
            universes->write(channel.address(),
                             channel.calculateCurrent(fadeTime, m_elapsed),
                             channel.group());
        }
    }

    return true;
}

bool ChaserRunner::roundCheck()
{
    if (m_currentStep < m_steps.size() && m_currentStep >= 0)
        return true; // In the middle of steps. No need to go any further.

    if (m_runOrder == Function::SingleShot)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                return false; // Forwards SingleShot has been completed.
            else
                m_currentStep = 0; // No wrapping
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                return false; // Backwards SingleShot has been completed.
            else
                m_currentStep = m_steps.size() - 1; // No wrapping
        }
    }
    else if (m_runOrder == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 0;
            else
                m_currentStep = m_steps.size() - 1;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 1;
            else
                m_currentStep = 0;
        }
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_steps.size())
                m_currentStep = 1;
            else
                m_currentStep = m_steps.size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_steps.size() - 2;
            else
                m_currentStep = 1;
            m_direction = Function::Forward;
        }
    }

    // Let's continue
    return true;
}

QMap <quint32,FadeChannel> ChaserRunner::createFadeChannels(const UniverseArray* universes,
                                                            bool handover) const
{
    QMap <quint32,FadeChannel> map;
    if (m_currentStep >= m_steps.size() || m_currentStep < 0)
        return map;

    // If the step is not a scene, don't attempt to create fade channels
    Scene* scene = qobject_cast<Scene*> (m_steps.at(m_currentStep));
    if (scene == NULL)
        return map;

    QListIterator <SceneValue> it(scene->values());
    while (it.hasNext() == true)
    {
        SceneValue value(it.next());
        Fixture* fxi = m_doc->fixture(value.fxi);
        if (fxi == NULL || fxi->channel(value.channel) == NULL)
            continue;

        FadeChannel channel;
        channel.setAddress(fxi->universeAddress() + value.channel);
        channel.setGroup(fxi->channel(value.channel)->group());
        channel.setTarget(value.value);

        channel.setStart(uchar(universes->preGMValues()[channel.address()]));
        if (handover && m_channelMap.contains(channel.address()))
            channel.setStart(m_channelMap[channel.address()].current());
        channel.setCurrent(channel.start());

        map[channel.address()] = channel;
    }

    return map;
}
