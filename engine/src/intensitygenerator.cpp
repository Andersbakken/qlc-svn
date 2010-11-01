/*
  Q Light Controller
  intensitygenerator.cpp

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

#include <QDateTime>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "intensitygenerator.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

IntensityGenerator::IntensityGenerator(Doc* doc, const QList <Fixture*>& fxiList)
        : m_doc(doc)
        , m_fixtures(fxiList)
        , m_odd(NULL)
        , m_even(NULL)
        , m_full(NULL)
        , m_zero(NULL)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(fxiList.size() != 0);

    // Initialize random seed
    srand(QDateTime::currentDateTime().toTime_t());

    // Create member scenes
    createScenes();
}

IntensityGenerator::~IntensityGenerator()
{
    // Destroy all functions that have NOT been added to Doc
    if (m_doc->function(m_odd->id()) == NULL)
        delete m_odd;
    m_odd = NULL;

    if (m_doc->function(m_even->id()) == NULL)
        delete m_even;
    m_even = NULL;

    if (m_doc->function(m_full->id()) == NULL)
        delete m_full;
    m_full = NULL;

    if (m_doc->function(m_zero->id()) == NULL)
        delete m_zero;
    m_zero = NULL;

    QListIterator <Scene*> seqit(m_sequence);
    while (seqit.hasNext() == true)
    {
        Scene* scene(seqit.next());
        if (m_doc->function(scene->id()) == NULL)
            delete scene;
    }
    m_sequence.clear();

    QListIterator <Scene*> rndit(m_random);
    while (rndit.hasNext() == true)
    {
        Scene* scene(rndit.next());
        if (m_doc->function(scene->id()) == NULL)
            delete scene;
    }
    m_random.clear();
}

bool IntensityGenerator::createOddEvenChaser()
{
    if (m_odd == NULL || m_even == NULL)
        return false;

    // Abort if doc can't fit the chaser and its two members
    if (m_doc->functionsFree() < 3)
        return false;

    // Create the chaser only if both steps contain something
    if (m_odd->values().size() != 0 && m_even->values().size() != 0)
    {
        // Abort if doc won't take the scenes
        if (m_doc->addFunction(m_odd) == false)
            return false;
        if (m_doc->addFunction(m_even) == false)
            return false;

        Chaser* chaser = new Chaser(m_doc);
        chaser->setName("Intensity - Even/Odd");
        chaser->addStep(m_odd->id());
        chaser->addStep(m_even->id());

        // Abort if doc won't take the chaser
        if (m_doc->addFunction(chaser) == false)
        {
            delete chaser;
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

bool IntensityGenerator::createFullZeroChaser()
{
    if (m_full == NULL || m_zero == NULL)
        return false;

    // Abort if doc can't fit the chaser and its two members
    if (m_doc->functionsFree() < 3)
        return false;

    // Create the chaser only if both steps contain something
    if (m_full->values().size() != 0 && m_zero->values().size() != 0)
    {
        // Abort if doc won't take the scenes
        if (m_doc->addFunction(m_full) == false)
            return false;
        if (m_doc->addFunction(m_zero) == false)
            return false;

        Chaser* chaser = new Chaser(m_doc);
        chaser->setName("Intensity - Full/Zero");
        chaser->addStep(m_full->id());
        chaser->addStep(m_zero->id());

        if (m_doc->addFunction(chaser) == false)
        {
            delete chaser;
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

bool IntensityGenerator::createSequenceChasers()
{
    if (m_sequence.size() == 0)
        return false;

    // Abort if doc can't fit the two chasers and their sequence members
    if (m_doc->functionsFree() < quint32(m_sequence.size() + 2))
        return false;

    // Abort immediately if doc won't take all sequence steps
    QListIterator <Scene*> it(m_sequence);
    while (it.hasNext() == true)
    {
        Scene* scene(it.next());
        if (m_doc->addFunction(scene) == false)
            return false;
    }

    // Forward
    if (createSequenceChaser(true) == false)
        return false;

    // Backward
    if (createSequenceChaser(false) == false)
        return false;

    return true;
}

bool IntensityGenerator::createRandomChaser()
{
    if (m_random.size() == 0)
        return false;

    // Abort if doc can't fit the chaser and its members
    if (m_doc->functionsFree() < quint32(m_random.size() + 1))
        return false;

    // Abort immediately if doc won't take all sequence steps
    QListIterator <Scene*> it(m_random);
    while (it.hasNext() == true)
    {
        if (m_doc->addFunction(it.next()) == false)
            return false;
    }

    // Create the random chaser
    Chaser* chaser = new Chaser(m_doc);
    chaser->setName("Intensity - Random");
    if (m_doc->addFunction(chaser) == false)
    {
        // Abort if doc won't accept the chaser
        delete chaser;
        return false;
    }
    else
    {
        for (int i = 0; i < m_random.size(); i++)
            chaser->addStep(m_random[i]->id());
        return true;
    }
}

void IntensityGenerator::createScenes()
{
    m_odd = new Scene(m_doc);
    m_odd->setName("Intensity - Odd");

    m_even = new Scene(m_doc);
    m_even->setName("Intensity - Even");

    m_full = new Scene(m_doc);
    m_full->setName("Intensity - Full");

    m_zero = new Scene(m_doc);
    m_zero->setName("Intensity - Zero");

    // Create sequence & random scene lists
    int i = 0;
    QListIterator <Fixture*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        Fixture* fxi(it.next());
        Q_ASSERT(fxi != NULL);

        Scene* sq = new Scene(m_doc);
        sq->setName(QString("Intensity - ") + fxi->name());
        m_sequence << sq;

        sq = new Scene(m_doc);
        sq->setName(QString("Intensity - Random - %1").arg(++i));
        m_random << sq;
    }

    // Go thru all fixtures
    for (int i = 0; i < m_fixtures.size(); i++)
    {
        Fixture* fxi = m_fixtures[i];
        Q_ASSERT(fxi != NULL);

        // Find such channels from the fixture that belong to the
        // given channel group.
        QList <quint32> channels = findChannels(fxi, QLCChannel::Intensity);

        // Insert values to member scenes for each found channel
        for (int j = 0; j < channels.size(); j++)
        {
            quint32 ch = channels.at(j);
            const QLCChannel* channel = fxi->channel(ch);
            Q_ASSERT(channel != NULL);

            uchar min = 0;
            uchar max = UCHAR_MAX;
            int modulo = i;

            // Find the minimum and maximum intensity values for
            // the current channel
            findMinMax(channel, &min, &max);

            // Set all intensity channels to max in the $full scene
            m_full->setValue(fxi->id(), ch, max);

            // Set all intensity channels to min in the $zero scene
            m_zero->setValue(fxi->id(), ch, min);

            // Create even & odd values
            if (fxi->isDimmer() == false)
                modulo = i; // For each intelligent fixture
            else
                modulo = j; // For each dimmer channel

            if ((modulo % 2) == 0)
            {
                m_even->setValue(fxi->id(), ch, max);
                m_odd->setValue(fxi->id(), ch, min);
            }
            else
            {
                m_even->setValue(fxi->id(), ch, min);
                m_odd->setValue(fxi->id(), ch, max);
            }

            // Create sequence and random values
            for (int s = 0; s < m_sequence.size(); s++)
            {
                if (s == i)
                    m_sequence[s]->setValue(fxi->id(), ch, max);
                else
                    m_sequence[s]->setValue(fxi->id(), ch, min);

                if ((rand() % 2) == 0)
                    m_random[s]->setValue(fxi->id(), ch, max);
                else
                    m_random[s]->setValue(fxi->id(), ch, min);
            }
        }
    }
}

bool IntensityGenerator::createSequenceChaser(bool forward)
{
    // Create a sequence chaser
    Chaser* chaser = new Chaser(m_doc);
    if (forward == true)
    {
        chaser->setName("Intensity - Sequence Forward");
        chaser->setDirection(Function::Forward);
    }
    else
    {
        chaser->setName("Intensity - Sequence Backward");
        chaser->setDirection(Function::Backward);
    }

    if (m_doc->addFunction(chaser) == false)
    {
        // Abort if doc won't accept the chaser
        delete chaser;
        return false;
    }
    else
    {
        for (int i = 0; i < m_sequence.size(); i++)
            chaser->addStep(m_sequence[i]->id());
        return true;
    }
}

bool IntensityGenerator::findMinMax(const QLCChannel* channel, uchar* min,
                                    uchar* max) const
{
    Q_ASSERT(channel != NULL);
    Q_ASSERT(min != NULL);
    Q_ASSERT(max != NULL);

    // The intensity channel contains also something else than just a
    // dimmer if it has more than one capability and a search is necessary.
    // Try to be smart and guess which capability provides dimmer
    // intensity. If a suitable capability is not found, values are not
    // modified.
    if (channel->capabilities().size() > 1)
    {
        const QLCCapability* cap = NULL;

        // Search for (I|i)ntensity or (D|d)immer capability
        cap = channel->searchCapability("ntensity", false);
        if (cap == NULL)
            cap = channel->searchCapability("immer", false);

        if (cap != NULL)
        {
            *min = cap->min();
            *max = cap->max();

            return true;
        }
    }

    return false;
}

QList <quint32> IntensityGenerator::findChannels(const Fixture* fixture,
                                                 QLCChannel::Group group)
{
    QList <quint32> channels;

    Q_ASSERT(fixture != NULL);
    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel(fixture->channel(ch));
        Q_ASSERT(channel != NULL);
        if (channel->group() == group)
            channels << ch;
    }

    return channels;
}
