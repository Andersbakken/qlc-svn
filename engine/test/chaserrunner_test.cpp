/*
  Q Light Controller - Unit test
  chaserrunner_test.cpp

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

#include <QtTest>
#include <QMap>

#include "chaserrunner_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "universearray.h"
#include "fadechannel.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "bus.h"
#include "doc.h"

#define private public
#include "chaserrunner.h"
#undef private

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void ChaserRunner_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void ChaserRunner_Test::init()
{
    m_doc = new Doc(this, m_cache);

    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    const QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(m_doc);
    QVERIFY(fxi != NULL);
    fxi->setFixtureDefinition(def, mode);
    fxi->setName("Test Fixture");
    fxi->setAddress(0);
    fxi->setUniverse(0);
    m_doc->addFixture(fxi);

    m_scene1 = new Scene(m_doc);
    QVERIFY(m_scene1 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene1->setValue(fxi->id(), i, 255 - i);
    m_doc->addFunction(m_scene1);

    m_scene2 = new Scene(m_doc);
    QVERIFY(m_scene2 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene2->setValue(fxi->id(), i, 127 - i);
    m_doc->addFunction(m_scene2);

    m_scene3 = new Scene(m_doc);
    QVERIFY(m_scene3 != NULL);
    for (quint32 i = 0; i < fxi->channels(); i++)
        m_scene3->setValue(fxi->id(), i, 0 + i);
    m_doc->addFunction(m_scene3);
}

void ChaserRunner_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void ChaserRunner_Test::initial()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);
    QCOMPARE(cr.m_doc, m_doc);
    QCOMPARE(cr.m_steps, steps);
    QCOMPARE(cr.m_holdBusId, Bus::defaultHold());
    QCOMPARE(cr.m_direction, Function::Forward);
    QCOMPARE(cr.m_originalDirection, Function::Forward);
    QCOMPARE(cr.m_runOrder, Function::SingleShot);
    QVERIFY(cr.m_channelMap.isEmpty() == true);
    QCOMPARE(cr.m_elapsed, quint32(0));
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_currentStep, 0);
    QCOMPARE(cr.m_newCurrent, -1);

    ChaserRunner cr2(m_doc, steps, Bus::defaultFade(), Function::Backward,
                    Function::Loop);
    QCOMPARE(cr2.m_doc, m_doc);
    QCOMPARE(cr2.m_steps, steps);
    QCOMPARE(cr2.m_holdBusId, Bus::defaultFade());
    QCOMPARE(cr2.m_direction, Function::Backward);
    QCOMPARE(cr2.m_originalDirection, Function::Backward);
    QCOMPARE(cr2.m_runOrder, Function::Loop);
    QVERIFY(cr2.m_channelMap.isEmpty() == true);
    QCOMPARE(cr2.m_elapsed, quint32(0));
    QCOMPARE(cr2.m_next, false);
    QCOMPARE(cr2.m_currentStep, 2);
    QCOMPARE(cr.m_newCurrent, -1);
}

void ChaserRunner_Test::nextPrevious()
{
    QList <Function*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    cr.next();
    QCOMPARE(cr.m_next, true);
    QCOMPARE(cr.m_previous, false);

    cr.next();
    QCOMPARE(cr.m_next, true);
    QCOMPARE(cr.m_previous, false);

    cr.previous();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, true);

    cr.previous();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, true);

    cr.reset();
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, false);
}

void ChaserRunner_Test::autoStep()
{
    QList <Function*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    QCOMPARE(cr.isAutoStep(), true);
    cr.setAutoStep(true);
    QCOMPARE(cr.m_autoStep, true);
    QCOMPARE(cr.isAutoStep(), true);
    cr.setAutoStep(false);
    QCOMPARE(cr.m_autoStep, false);
    QCOMPARE(cr.isAutoStep(), false);
    cr.setAutoStep(false);
    QCOMPARE(cr.m_autoStep, false);
    QCOMPARE(cr.isAutoStep(), false);
}

void ChaserRunner_Test::roundCheckSingleShotForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 4;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckSingleShotBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::SingleShot);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = -2;
    QVERIFY(cr.roundCheck() == false);
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckLoopForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around back to index 0
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    // Loops around to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
}

void ChaserRunner_Test::roundCheckLoopBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::Loop);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around back to index 2
    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    // Loops around to index 0
    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
}

void ChaserRunner_Test::roundCheckPingPongForward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::PingPong);

    QCOMPARE(cr.currentStep(), 0);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Forward);
}

void ChaserRunner_Test::roundCheckPingPongBackward()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::PingPong);

    QCOMPARE(cr.currentStep(), 2);
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 2;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.m_currentStep = 3;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = 0;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_direction, Function::Backward);

    cr.m_currentStep = -1;
    QVERIFY(cr.roundCheck() == true);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(cr.m_direction, Function::Forward);

    cr.reset();
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(cr.m_direction, Function::Backward);
}

void ChaserRunner_Test::createFadeChannels()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);
    QMap <quint32,FadeChannel> map;
    FadeChannel ch;

    // No handover
    QCOMPARE(cr.currentStep(), 0);
    map = cr.createFadeChannels(&ua, false);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(255));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(254));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(253));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(252));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(251));
    QCOMPARE(ch.current(), uchar(0));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(0));
    QCOMPARE(ch.target(), uchar(250));
    QCOMPARE(ch.current(), uchar(0));

    // Handover with previous values
    map[0].setCurrent(map[0].target());
    map[1].setCurrent(map[1].target());
    map[2].setCurrent(map[2].target());
    map[3].setCurrent(map[3].target());
    map[4].setCurrent(map[4].target());
    map[5].setCurrent(map[5].target());
    cr.m_channelMap = map;
    cr.m_currentStep = 1;
    map = cr.createFadeChannels(&ua, true);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(255));
    QCOMPARE(ch.target(), uchar(127));
    QCOMPARE(ch.current(), uchar(255));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(254));
    QCOMPARE(ch.target(), uchar(126));
    QCOMPARE(ch.current(), uchar(254));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(253));
    QCOMPARE(ch.target(), uchar(125));
    QCOMPARE(ch.current(), uchar(253));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(252));
    QCOMPARE(ch.target(), uchar(124));
    QCOMPARE(ch.current(), uchar(252));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(251));
    QCOMPARE(ch.target(), uchar(123));
    QCOMPARE(ch.current(), uchar(251));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(250));
    QCOMPARE(ch.target(), uchar(122));
    QCOMPARE(ch.current(), uchar(250));

    // Handover attempt without previous values -> runner takes them from UA
    ua.write(0, 1, QLCChannel::Intensity);
    ua.write(1, 2, QLCChannel::Intensity);
    ua.write(2, 3, QLCChannel::Intensity);
    ua.write(3, 4, QLCChannel::Intensity);
    ua.write(4, 5, QLCChannel::Intensity);
    ua.write(5, 6, QLCChannel::Intensity);
    cr.m_channelMap.clear();
    cr.m_currentStep = 2;
    map = cr.createFadeChannels(&ua, true);

    QVERIFY(map.contains(0) == true);
    ch = map[0];
    QCOMPARE(ch.address(), quint32(0));
    QCOMPARE(ch.start(), uchar(1));
    QCOMPARE(ch.target(), uchar(0));
    QCOMPARE(ch.current(), uchar(1));

    QVERIFY(map.contains(1) == true);
    ch = map[1];
    QCOMPARE(ch.address(), quint32(1));
    QCOMPARE(ch.start(), uchar(2));
    QCOMPARE(ch.target(), uchar(1));
    QCOMPARE(ch.current(), uchar(2));

    QVERIFY(map.contains(2) == true);
    ch = map[2];
    QCOMPARE(ch.address(), quint32(2));
    QCOMPARE(ch.start(), uchar(3));
    QCOMPARE(ch.target(), uchar(2));
    QCOMPARE(ch.current(), uchar(3));

    QVERIFY(map.contains(3) == true);
    ch = map[3];
    QCOMPARE(ch.address(), quint32(3));
    QCOMPARE(ch.start(), uchar(4));
    QCOMPARE(ch.target(), uchar(3));
    QCOMPARE(ch.current(), uchar(4));

    QVERIFY(map.contains(4) == true);
    ch = map[4];
    QCOMPARE(ch.address(), quint32(4));
    QCOMPARE(ch.start(), uchar(5));
    QCOMPARE(ch.target(), uchar(4));
    QCOMPARE(ch.current(), uchar(5));

    QVERIFY(map.contains(5) == true);
    ch = map[5];
    QCOMPARE(ch.address(), quint32(5));
    QCOMPARE(ch.start(), uchar(6));
    QCOMPARE(ch.target(), uchar(5));
    QCOMPARE(ch.current(), uchar(6));

    cr.m_currentStep = 3;
    map = cr.createFadeChannels(&ua, false);
    QVERIFY(map.isEmpty() == true);

    cr.m_currentStep = -1;
    map = cr.createFadeChannels(&ua, false);
    QVERIFY(map.isEmpty() == true);
}

void ChaserRunner_Test::writeNoSteps()
{
    QList <Function*> steps;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);

    QVERIFY(cr.write(&ua) == false);
}

void ChaserRunner_Test::writeMissingFixture()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    m_scene3->setValue(500, 0, 255);
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultFade(), 5);
    Bus::instance()->setValue(Bus::defaultHold(), 5);

    for (int i = 0; i < 120; i++)
        QVERIFY(cr.write(&ua) == true);
}

void ChaserRunner_Test::writeHoldZero()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 0);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    QVERIFY(cr.write(&ua) == true);
    QCOMPARE(cr.m_elapsed, quint32(1));
    QVERIFY(cr.m_channelMap.isEmpty() == false);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
    QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
    QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
    QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
    QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
    QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));

    ua.zeroIntensityChannels();

    QVERIFY(cr.write(&ua) == true);
    QCOMPARE(cr.m_elapsed, quint32(1));
    QVERIFY(cr.m_channelMap.isEmpty() == false);
    QCOMPARE(cr.currentStep(), 1);
    QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
    QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
    QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
    QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
    QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
    QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));

    ua.zeroIntensityChannels();

    QVERIFY(cr.write(&ua) == true);
    QCOMPARE(cr.m_elapsed, quint32(1));
    QVERIFY(cr.m_channelMap.isEmpty() == false);
    QCOMPARE(cr.currentStep(), 2);
    QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
    QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
    QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
    QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
    QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
    QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));

    ua.zeroIntensityChannels();

    QVERIFY(cr.write(&ua) == true);
    QCOMPARE(cr.m_elapsed, quint32(1));
    QVERIFY(cr.m_channelMap.isEmpty() == false);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
    QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
    QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
    QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
    QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
    QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
}

void ChaserRunner_Test::writeForwardLoopHoldFiveNextPrevious()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 5);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        if (i == 4)
        {
            cr.previous();
            break;
        }

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        if (i == 4)
        {
            cr.next();
            break;
        }

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 1);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }
}

void ChaserRunner_Test::writeBackwardLoopHoldFiveNextPrevious()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Backward,
                    Function::Loop);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 5);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        if (i == 4)
        {
            cr.previous();
            break;
        }

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        if (i == 4)
        {
            cr.next();
            break;
        }

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 1);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }
}

void ChaserRunner_Test::writeForwardSingleShotHoldFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::SingleShot);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 5);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 1);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));
    }

    for (quint32 i = 0; i < Bus::instance()->value(Bus::defaultHold()); i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    // SingleShot is completed
    QVERIFY(cr.write(&ua) == false);
}

void ChaserRunner_Test::writeNoAutoStepHoldFive()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    cr.setAutoStep(false);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 5);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    for (quint32 i = 0; i < 10; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    cr.previous();

    for (quint32 i = 0; i < 25; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    cr.next();

    for (quint32 i = 0; i < 35; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    cr.next();

    for (quint32 i = 0; i < 15; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 1);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));
    }
}

void ChaserRunner_Test::writeNoAutoSetCurrentStep()
{
    QList <Function*> steps;
    steps << m_scene1 << m_scene2 << m_scene3;
    ChaserRunner cr(m_doc, steps, Bus::defaultHold(), Function::Forward,
                    Function::Loop);
    cr.setAutoStep(false);
    UniverseArray ua(512);

    Bus::instance()->setValue(Bus::defaultHold(), 5);
    Bus::instance()->setValue(Bus::defaultFade(), 0);

    for (quint32 i = 0; i < 10; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    cr.next();
    QCOMPARE(cr.m_next, true);
    cr.setCurrentStep(15);
    QCOMPARE(cr.m_newCurrent, -1);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_next, true);
    QCOMPARE(cr.m_previous, false);

    cr.previous();
    QCOMPARE(cr.m_previous, true);
    cr.setCurrentStep(4);
    QCOMPARE(cr.m_newCurrent, -1);
    QCOMPARE(cr.currentStep(), 0);
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, true);

    cr.setCurrentStep(2);
    QCOMPARE(cr.m_newCurrent, 2);
    QCOMPARE(cr.m_next, false);
    QCOMPARE(cr.m_previous, false);

    for (quint32 i = 0; i < 25; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_newCurrent, -1);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 2);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(0));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(1));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(2));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(3));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(4));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(5));
    }

    cr.setCurrentStep(0);
    QCOMPARE(cr.m_newCurrent, 0);

    for (quint32 i = 0; i < 35; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_newCurrent, -1);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 0);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(255));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(254));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(253));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(252));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(251));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(250));
    }

    cr.setCurrentStep(1);
    QCOMPARE(cr.m_newCurrent, 1);

    for (quint32 i = 0; i < 15; i++)
    {
        ua.zeroIntensityChannels();

        QVERIFY(cr.write(&ua) == true);
        QCOMPARE(cr.m_newCurrent, -1);
        QCOMPARE(cr.m_elapsed, quint32(i + 1));
        QVERIFY(cr.m_channelMap.isEmpty() == false);
        QCOMPARE(cr.currentStep(), 1);
        QCOMPARE(uchar(ua.preGMValues().data()[0]), uchar(127));
        QCOMPARE(uchar(ua.preGMValues().data()[1]), uchar(126));
        QCOMPARE(uchar(ua.preGMValues().data()[2]), uchar(125));
        QCOMPARE(uchar(ua.preGMValues().data()[3]), uchar(124));
        QCOMPARE(uchar(ua.preGMValues().data()[4]), uchar(123));
        QCOMPARE(uchar(ua.preGMValues().data()[5]), uchar(122));
    }
}
