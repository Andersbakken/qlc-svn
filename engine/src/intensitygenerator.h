/*
  Q Light Controller
  intensitygenerator.h

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

#ifndef INTENSITYGENERATOR_H
#define INTENSITYGENERATOR_H

#include <QList>

#include "qlcchannel.h"
#include "qlctypes.h"

class Fixture;
class Scene;
class Doc;

/**
 * Generates scenes and chasers using selected fixtures' intensity channel(s).
 */
class IntensityGenerator
{
public:
    /**
     * Create a new instance.
     *
     * @param doc The Doc instance that should take the created functions
     * @param fxis List of fixtures to create functions for
     */
    IntensityGenerator(Doc* doc, const QList <Fixture*>& fxis);

    /** Destructor */
    ~IntensityGenerator();

    /**
     * Create a chaser that blinks every other intensity channel
     * sequentially. Attempts to add m_odd and m_even to m_doc as well
     * as the chaser that is created.
     *
     * If just one of the functions cannot be added to m_doc, the whole
     * operation will fail and nothing gets added to m_doc.
     */
    bool createOddEvenChaser();

    /**
     * Create a chaser that blinks all intensity channels on and off
     * sequentially. Attempts to add m_full and m_zero to m_doc as well
     * as the chaser that is created.
     *
     * If just one of the functions cannot be added to m_doc, the whole
     * operation will fail and nothing gets added to m_doc.
     */
    bool createFullZeroChaser();

    /**
     * Create a chaser that runs each intensity channels on and then off
     * separately. Attempts to add the contents of m_sequence to m_doc as
     * well as the chaser that is created.
     *
     * If just one of the functions cannot be added to m_doc, the whole
     * operation will fail and nothing gets added to m_doc.
     */
    bool createSequenceChasers();

    /**
     * Create a chaser that toggles a random set of intensity channels
     * on or off sequentially. Attempts to add the contents of m_random to
     * m_doc as well as the chaser that is created.
     *
     * If just one of the functions cannot be added to m_doc, the whole
     * operation will fail and nothing gets added to m_doc.
     */
    bool createRandomChaser();

protected:
    /**
     * Create all of the scenes that are used in the various chasers
     * generated with this class.
     */
    void createScenes();

    /**
     * Create a sequence chaser and make it run either forwards or
     * backwards. All sequence steps as well as the chaser itself are
     * added to m_doc. If even one of these steps fail, false is returned
     * and nothing is then added to m_doc.
     *
     * @param forward If true, the generated sequence will run forwards,
     *                otherwise it will run backwards.
     * @return true if successful, otherwise false
     */
    bool createSequenceChaser(bool forward);

    /**
     * Find the minimum and maximum intensity values from the channel.
     * If an intensity capability is not found, $min and $max are not
     * modified at all and false is returned.
     *
     * @param channel The channel to search intensity capability from
     * @param min The minimum intensity value
     * @param max The maximum intensity value
     * @return true if successful, otherwise false.
     */
    bool findMinMax(const QLCChannel* channel, uchar* min,
                    uchar* max) const;

public:
    /**
     * Find a list of fixture channels belonging to the given group.
     *
     * @param fixture The fixture to search the channels from
     * @param group Channel group to search for
     * @return A list of channels in the given fixture, belonging
     *         to the specified channel group
     */
    static QList <quint32> findChannels(const Fixture* fixture,
                                        QLCChannel::Group group);

private:
    Doc* m_doc;
    QList <Fixture*> m_fixtures;

    Scene* m_odd;
    Scene* m_even;

    Scene* m_full;
    Scene* m_zero;

    QList <Scene*> m_sequence;
    QList <Scene*> m_random;
};

#endif
