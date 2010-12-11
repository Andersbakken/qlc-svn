/*
  Q Light Controller - Unit test
  chaserrunner_test.h

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

#ifndef CHASERRUNNER_TEST_H
#define CHASERRUNNER_TEST_H

#include <QObject>
#include "qlcfixturedefcache.h"

class Scene;
class Doc;

class ChaserRunner_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initial();
    void nextPrevious();
    void autoStep();
    void roundCheckSingleShotForward();
    void roundCheckSingleShotBackward();
    void roundCheckLoopForward();
    void roundCheckLoopBackward();
    void roundCheckPingPongForward();
    void roundCheckPingPongBackward();
    void createFadeChannels();

    void writeNoSteps();
    void writeMissingFixture();
    void writeHoldZero();

    void writeForwardLoopHoldFiveNextPrevious();
    void writeBackwardLoopHoldFiveNextPrevious();
    void writeForwardSingleShotHoldFive();
    void writeNoAutoStepHoldFive();
    void writeNoAutoSetCurrentStep();

private:
    Doc* m_doc;
    QLCFixtureDefCache m_cache;
    Scene* m_scene1;
    Scene* m_scene2;
    Scene* m_scene3;
};

#endif
