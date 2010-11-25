/*
  Q Light Controller - Unit test
  efxfixture_test.cpp

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
#include <QtXml>
#include <QList>

#include "efxfixture_test.h"
#include "scene_stub.h"

#include "universearray.h"
#include "function.h"
#include "fixture.h"
#include "doc.h"

/* Expose protected members to the unit test */
#define protected public
#include "efxfixture.h"
#include "efx.h"
#undef protected

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void EFXFixture_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void EFXFixture_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void EFXFixture_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void EFXFixture_Test::initial()
{
    EFX e(m_doc);

    EFXFixture ef(&e);
    QVERIFY(ef.fixture() == Fixture::invalidId());
    QVERIFY(ef.direction() == EFX::Forward);
    QVERIFY(ef.serialNumber() == 0);
    QVERIFY(ef.startScene() == NULL);
    QVERIFY(ef.stopScene() == NULL);
    QVERIFY(ef.isValid() == false);
    QVERIFY(ef.isReady() == false);

    QVERIFY(ef.m_fixture == Fixture::invalidId());
    QVERIFY(ef.m_direction == EFX::Forward);
    QVERIFY(ef.m_serialNumber == 0);
    QVERIFY(ef.m_runTimeDirection == EFX::Forward);
    QVERIFY(ef.m_startScene == NULL);
    QVERIFY(ef.m_stopScene == NULL);
    QVERIFY(ef.m_initialized == false);
    QVERIFY(ef.m_ready == false);
    QVERIFY(ef.m_iterator == 0);
    QVERIFY(ef.m_skipIterator == 0);
    QVERIFY(ef.m_skipThreshold == 0);
    QVERIFY(ef.m_panValue == 0);
    QVERIFY(ef.m_tiltValue == 0);
    QVERIFY(ef.m_lsbPanChannel == QLCChannel::invalid());
    QVERIFY(ef.m_msbPanChannel == QLCChannel::invalid());
    QVERIFY(ef.m_lsbTiltChannel == QLCChannel::invalid());
    QVERIFY(ef.m_msbTiltChannel == QLCChannel::invalid());
}

void EFXFixture_Test::copyFrom()
{
    EFX e(m_doc);

    EFXFixture ef(&e);
    ef.m_fixture = 15;
    ef.m_direction = EFX::Backward;
    ef.m_serialNumber = 25;
    ef.m_runTimeDirection = EFX::Backward;
    ef.m_startScene = reinterpret_cast<Scene*>(0xDEADBEEF);
    ef.m_stopScene = reinterpret_cast<Scene*>(0x00BADA55);
    ef.m_initialized = true;
    ef.m_ready = true;
    ef.m_iterator = 313.37;
    ef.m_skipIterator = 4.2;
    ef.m_skipThreshold = 6.9;
    ef.m_panValue = 127.15;
    ef.m_tiltValue = 240.99;
    ef.m_lsbPanChannel = 1;
    ef.m_msbPanChannel = 5;
    ef.m_lsbTiltChannel = 2;
    ef.m_msbTiltChannel = 6;

    EFXFixture copy(&e);
    copy.copyFrom(&ef);
    QVERIFY(ef.m_fixture == 15);
    QVERIFY(ef.m_direction == EFX::Backward);
    QVERIFY(ef.m_serialNumber == 25);
    QVERIFY(ef.m_runTimeDirection == EFX::Backward);
    QVERIFY(ef.m_startScene == reinterpret_cast<Scene*>(0xDEADBEEF));
    QVERIFY(ef.m_stopScene == reinterpret_cast<Scene*>(0x00BADA55));
    QVERIFY(ef.m_initialized == true);
    QVERIFY(ef.m_ready == true);
    QVERIFY(ef.m_iterator == 313.37);
    QVERIFY(ef.m_skipIterator == 4.2);
    QVERIFY(ef.m_skipThreshold == 6.9);
    QVERIFY(ef.m_panValue == 127.15);
    QVERIFY(ef.m_tiltValue == 240.99);
    QVERIFY(ef.m_lsbPanChannel == 1);
    QVERIFY(ef.m_msbPanChannel == 5);
    QVERIFY(ef.m_lsbTiltChannel == 2);
    QVERIFY(ef.m_msbTiltChannel == 6);
}

void EFXFixture_Test::publicProperties()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.setFixture(19);
    QVERIFY(ef.fixture() == 19);

    ef.setFixture(Fixture::invalidId());
    QVERIFY(ef.fixture() == Fixture::invalidId());

    ef.setDirection(EFX::Backward);
    QVERIFY(ef.direction() == EFX::Backward);
    QVERIFY(ef.m_runTimeDirection == EFX::Backward);

    ef.setDirection(EFX::Forward);
    QVERIFY(ef.direction() == EFX::Forward);
    QVERIFY(ef.m_runTimeDirection == EFX::Forward);
}

void EFXFixture_Test::loadSuccess()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("83");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 83);
    QVERIFY(ef.direction() == EFX::Backward);
}

void EFXFixture_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("EFXFixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("189");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == false);
    QVERIFY(ef.fixture() == Fixture::invalidId());
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::loadWrongDirection()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("97");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Phorrwarrd");
    dir.appendChild(dirText);
    root.appendChild(dir);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 97);
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::loadExtraTag()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("108");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Forward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement foo = doc.createElement("Foobar");
    QDomText fooText = doc.createTextNode("Just testing");
    foo.appendChild(fooText);
    root.appendChild(foo);

    EFX e(m_doc);
    EFXFixture ef(&e);
    QVERIFY(ef.loadXML(&root) == true);
    QVERIFY(ef.fixture() == 108);
    QVERIFY(ef.direction() == EFX::Forward);
}

void EFXFixture_Test::save()
{
    EFX e(m_doc);
    EFXFixture ef(&e);
    ef.setFixture(56);
    ef.setDirection(EFX::Backward);

    QDomDocument doc;
    QDomElement root = doc.createElement("EFX");

    QVERIFY(ef.saveXML(&doc, &root) == true);

    QDomElement tag = root.firstChild().toElement();
    QVERIFY(tag.tagName() == "Fixture");

    tag = tag.firstChild().toElement();
    QVERIFY(tag.tagName() == "ID");
    QVERIFY(tag.text() == "56");

    tag = tag.nextSibling().toElement();
    QVERIFY(tag.tagName() == "Direction");
    QVERIFY(tag.text() == "Backward");
}

void EFXFixture_Test::protectedProperties()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.setSerialNumber(15);
    QVERIFY(ef.serialNumber() == 15);

    ef.setStartScene(reinterpret_cast<Scene*>(0xDEADBEEF));
    QVERIFY(ef.startScene() == reinterpret_cast<Scene*>(0xDEADBEEF));

    ef.setStopScene(reinterpret_cast<Scene*>(0xDEADBEEF));
    QVERIFY(ef.stopScene() == reinterpret_cast<Scene*>(0xDEADBEEF));

    ef.setLsbPanChannel(56);
    QVERIFY(ef.m_lsbPanChannel == 56);

    ef.setMsbPanChannel(3);
    QVERIFY(ef.m_msbPanChannel == 3);

    ef.setLsbTiltChannel(19);
    QVERIFY(ef.m_lsbTiltChannel == 19);

    ef.setMsbTiltChannel(102);
    QVERIFY(ef.m_msbTiltChannel == 102);
}

void EFXFixture_Test::updateSkipThreshold()
{
    EFX e(m_doc);

    EFXFixture* ef1 = new EFXFixture(&e);
    ef1->setFixture(1);
    ef1->setSerialNumber(0);
    e.addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(&e);
    ef2->setFixture(2);
    ef2->setSerialNumber(1);
    e.addFixture(ef2);

    EFXFixture* ef3 = new EFXFixture(&e);
    ef3->setFixture(3);
    ef3->setSerialNumber(2);
    e.addFixture(ef3);

    EFXFixture* ef4 = new EFXFixture(&e);
    ef4->setFixture(4);
    ef4->setSerialNumber(3);
    e.addFixture(ef4);

    ef1->updateSkipThreshold();
    QVERIFY(ef1->m_skipThreshold == (M_PI * 2.0) * 0);

    ef2->updateSkipThreshold();
    QVERIFY(ef2->m_skipThreshold == (M_PI * 2.0) * 0.25);

    ef3->updateSkipThreshold();
    QVERIFY(ef3->m_skipThreshold == (M_PI * 2.0) * 0.5);

    ef4->updateSkipThreshold();
    QVERIFY(ef4->m_skipThreshold == (M_PI * 2.0) * 0.75);
}

void EFXFixture_Test::isValid()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    QVERIFY(ef.isValid() == false);

    ef.m_msbPanChannel = 5;
    QVERIFY(ef.isValid() == false);

    ef.m_msbTiltChannel = 9;
    QVERIFY(ef.isValid() == false);

    ef.m_fixture = 72;
    QVERIFY(ef.isValid() == true);
}

void EFXFixture_Test::reset()
{
    EFX e(m_doc);

    EFXFixture* ef1 = new EFXFixture(&e);
    ef1->setFixture(1);
    ef1->setSerialNumber(0);
    e.addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(&e);
    ef2->setFixture(2);
    ef2->setSerialNumber(1);
    e.addFixture(ef2);

    EFXFixture* ef3 = new EFXFixture(&e);
    ef3->setFixture(3);
    ef3->setSerialNumber(2);
    e.addFixture(ef3);

    EFXFixture* ef4 = new EFXFixture(&e);
    ef4->m_fixture = 15;
    ef4->m_direction = EFX::Forward;
    ef4->m_serialNumber = 3;
    ef4->m_runTimeDirection = EFX::Backward;
    ef4->m_startScene = reinterpret_cast<Scene*>(0xDEADBEEF);
    ef4->m_stopScene = reinterpret_cast<Scene*>(0x00BADA55);
    ef4->m_initialized = true;
    ef4->m_ready = true;
    ef4->m_iterator = 313.37;
    ef4->m_skipIterator = 4.2;
    ef4->m_skipThreshold = 6.9;
    ef4->m_panValue = 127.15;
    ef4->m_tiltValue = 240.99;
    ef4->m_lsbPanChannel = 1;
    ef4->m_msbPanChannel = 5;
    ef4->m_lsbTiltChannel = 2;
    ef4->m_msbTiltChannel = 6;
    e.addFixture(ef4);

    ef4->reset();
    QVERIFY(ef4->m_fixture == 15);
    QVERIFY(ef4->m_direction == EFX::Forward);
    QVERIFY(ef4->m_serialNumber == 3);
    QVERIFY(ef4->m_runTimeDirection == EFX::Forward);
    QVERIFY(ef4->m_startScene == reinterpret_cast<Scene*>(0xDEADBEEF));
    QVERIFY(ef4->m_stopScene == reinterpret_cast<Scene*>(0x00BADA55));
    QVERIFY(ef4->m_initialized == false);
    QVERIFY(ef4->m_ready == false);
    QVERIFY(ef4->m_iterator == 0);
    QVERIFY(ef4->m_skipIterator == 0);
    QVERIFY(ef4->m_skipThreshold == (M_PI * 2.0) * 0.75);
    QVERIFY(ef4->m_panValue == 0);
    QVERIFY(ef4->m_tiltValue == 0);
    QVERIFY(ef4->m_lsbPanChannel == 1);
    QVERIFY(ef4->m_msbPanChannel == 5);
    QVERIFY(ef4->m_lsbTiltChannel == 2);
    QVERIFY(ef4->m_msbTiltChannel == 6);
}

void EFXFixture_Test::startStop()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    SceneStub s1(m_doc);
    s1.setValue(0, 1);
    s1.setValue(1, 2);
    s1.setValue(2, 3);
    ef.setStartScene(&s1);

    SceneStub s2(m_doc);
    s2.setValue(0, 4);
    s2.setValue(1, 5);
    s2.setValue(2, 6);
    ef.setStopScene(&s2);

    UniverseArray array(512 * 4);

    ef.start(&array);
    QVERIFY(array.preGMValues()[0] == (char) 1);
    QVERIFY(array.preGMValues()[1] == (char) 2);
    QVERIFY(array.preGMValues()[2] == (char) 3);

    ef.stop(&array);
    QVERIFY(array.preGMValues()[0] == (char) 4);
    QVERIFY(array.preGMValues()[1] == (char) 5);
    QVERIFY(array.preGMValues()[2] == (char) 6);
}

void EFXFixture_Test::setPoint8bit()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.m_msbPanChannel = 0;
    ef.m_msbTiltChannel = 1;

    ef.m_panValue = 5.4;
    ef.m_tiltValue = 1.5;

    UniverseArray array(512 * 4);
    ef.setPoint(&array);
    QVERIFY(array.preGMValues()[0] == (char) 5);
    QVERIFY(array.preGMValues()[1] == (char) 1);
}

void EFXFixture_Test::setPoint16bit()
{
    EFX e(m_doc);
    EFXFixture ef(&e);

    ef.m_msbPanChannel = 0;
    ef.m_msbTiltChannel = 1;
    ef.m_lsbPanChannel = 2;
    ef.m_lsbTiltChannel = 3;

    ef.m_panValue = 5.4; /* MSB: 5, LSB: 0.4 (102) */
    ef.m_tiltValue = 1.5; /* MSB: 1, LSB: 0.5 (127) */

    UniverseArray array(512 * 4);
    ef.setPoint(&array);
    QVERIFY(array.preGMValues()[0] == (char) 5);
    QVERIFY(array.preGMValues()[1] == (char) 1);
    QVERIFY(array.preGMValues()[2] == (char) 102); /* 255 * 0.4 */
    QVERIFY(array.preGMValues()[3] == (char) 127); /* 255 * 0.5 */
}

void EFXFixture_Test::nextStepLoop()
{
    UniverseArray array(512 * 4);

    EFX e(m_doc);
    e.slotBusValueChanged(e.busID(), 50); /* 50 steps */

    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    /* Nothing should happen since isValid() == false */
    ef->nextStep(&array);
    for (int i = 0; i < 512 * 4; i++)
        QVERIFY(array.preGMValues()[i] == 0);

    /* Initialize the EFXFixture so that it can do math */
    ef->setSerialNumber(0);
    ef->setMsbPanChannel(0);
    ef->setMsbTiltChannel(1);
    ef->setLsbPanChannel(2);
    ef->setLsbTiltChannel(3);
    QVERIFY(ef->isValid() == true);
    QVERIFY(ef->m_initialized == false);
    QVERIFY(ef->m_iterator == 0);

    /* Run two cycles (2 * 50 = 100) and reset the checking iterator in
       the middle to expect correct iterator values. */
    qreal checkIter = 0;
    for (int i = 0; i < 100; i++)
    {
        ef->nextStep(&array);
        checkIter += e.m_stepSize;
        if (i == 50)
            checkIter = 0;
        QVERIFY(ef->m_iterator == checkIter);
        QVERIFY(ef->m_initialized == true);
        QVERIFY(ef->isReady() == false); // Loop is never ready
    }
}

void EFXFixture_Test::nextStepSingleShot()
{
    UniverseArray array(512 * 4);

    EFX e(m_doc);
    e.slotBusValueChanged(e.busID(), 50); /* 50 steps */
    e.setRunOrder(EFX::SingleShot);

    EFXFixture* ef = new EFXFixture(&e);
    ef->setFixture(0);
    e.addFixture(ef);

    /* Nothing should happen since isValid() == false */
    ef->nextStep(&array);
    for (int i = 0; i < 512 * 4; i++)
        QVERIFY(array.preGMValues()[i] == 0);

    /* Initialize the EFXFixture so that it can do math */
    ef->setSerialNumber(0);
    ef->setMsbPanChannel(0);
    ef->setMsbTiltChannel(1);
    ef->setLsbPanChannel(2);
    ef->setLsbTiltChannel(3);
    QVERIFY(ef->isValid() == true);
    QVERIFY(ef->m_initialized == false);
    QVERIFY(ef->m_iterator == 0);

    ef->reset();

    /* Run one cycle (50 steps) and reset the checking iterator in
       the middle to expect correct iterator values. */
    qreal checkIter = 0;
    for (int i = 0; i < 50; i++)
    {
        ef->nextStep(&array);
        checkIter += e.m_stepSize;
        QVERIFY(ef->m_initialized == true);
        QVERIFY(ef->m_iterator == checkIter);
        QVERIFY(ef->isReady() == false);
    }

    ef->nextStep(&array);

    /* Single-shot EFX should now be ready */
    QVERIFY(ef->isReady() == true);
}
