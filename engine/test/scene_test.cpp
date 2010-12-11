/*
  Q Light Controller - Unit test
  scene_test.cpp

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

#include "mastertimer_stub.h"
#include "scene_test.h"

#include "universearray.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void Scene_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void Scene_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void Scene_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void Scene_Test::initial()
{
    Scene s(m_doc);
    QVERIFY(s.type() == Function::Scene);
    QVERIFY(s.name() == "New Scene");
    QVERIFY(s.values().size() == 0);
    QVERIFY(s.id() == Function::invalidId());
}

void Scene_Test::values()
{
    Scene s(m_doc);
    QVERIFY(s.values().size() == 0);

    /* Value 3 to fixture 1's channel number 2 */
    s.setValue(1, 2, 3);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Value 6 to fixture 4's channel number 5 */
    SceneValue scv(4, 5, 6);
    s.setValue(scv);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    /* Replace previous value 3 with 15 for fixture 1's channel number 2 */
    s.setValue(1, 2, 15);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 15);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    QVERIFY(s.value(1, 2) == 15);
    QVERIFY(s.value(3, 2) == 0); // No such channel
    QVERIFY(s.value(4, 5) == 6);

    /* No channel 5 for fixture 1 in the scene, unset shouldn't happen */
    s.unsetValue(1, 5);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 15);
    QVERIFY(s.values().at(1).fxi == 4);
    QVERIFY(s.values().at(1).channel == 5);
    QVERIFY(s.values().at(1).value == 6);

    /* Remove fixture 1's channel 2 from the scene */
    s.unsetValue(1, 2);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 4);
    QVERIFY(s.values().at(0).channel == 5);
    QVERIFY(s.values().at(0).value == 6);

    /* No fixture 1 anymore */
    s.unsetValue(1, 2);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 4);
    QVERIFY(s.values().at(0).channel == 5);
    QVERIFY(s.values().at(0).value == 6);

    /* Remove fixture 4's channel 5 from the scene */
    s.unsetValue(4, 5);
    QVERIFY(s.values().size() == 0);

    s.setValue(1, 1, 255);
    s.setValue(2, 2, 255);
    s.setValue(4, 3, 255);
    s.setValue(1, 4, 255);
    QVERIFY(s.values().size() == 4);

    s.clear();
    QVERIFY(s.values().size() == 0);
}

void Scene_Test::fixtureRemoval()
{
    Scene s(m_doc);
    QVERIFY(s.values().size() == 0);

    s.setValue(1, 2, 3);
    s.setValue(4, 5, 6);
    QVERIFY(s.values().size() == 2);

    /* Simulate fixture removal signal with an uninteresting fixture id */
    s.slotFixtureRemoved(6);
    QVERIFY(s.values().size() == 2);

    /* Simulate fixture removal signal with a fixture in the scene */
    s.slotFixtureRemoved(4);
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Simulate fixture removal signal with an invalid fixture id */
    s.slotFixtureRemoved(Fixture::invalidId());
    QVERIFY(s.values().size() == 1);
    QVERIFY(s.values().at(0).fxi == 1);
    QVERIFY(s.values().at(0).channel == 2);
    QVERIFY(s.values().at(0).value == 3);

    /* Simulate fixture removal signal with a fixture in the scene */
    s.slotFixtureRemoved(1);
    QVERIFY(s.values().size() == 0);
}

void Scene_Test::loadSuccess()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Scene");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("5");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement v1 = doc.createElement("Value");
    v1.setAttribute("Fixture", 5);
    v1.setAttribute("Channel", 60);
    QDomText v1Text = doc.createTextNode("100");
    v1.appendChild(v1Text);
    root.appendChild(v1);

    QDomElement v2 = doc.createElement("Value");
    v2.setAttribute("Fixture", 133);
    v2.setAttribute("Channel", 4);
    QDomText v2Text = doc.createTextNode("59");
    v2.appendChild(v2Text);
    root.appendChild(v2);

    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Fixture", 133);
    foo.setAttribute("Channel", 4);
    QDomText fooText = doc.createTextNode("59");
    foo.appendChild(fooText);
    root.appendChild(foo);

    Scene s(m_doc);
    QVERIFY(s.loadXML(&root) == true);
    QVERIFY(s.busID() == 5);
    QVERIFY(s.values().size() == 2);
    QVERIFY(s.value(5, 60) == 100);
    QVERIFY(s.value(133, 4) == 59);
}

void Scene_Test::loadWrongType()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("5");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement v1 = doc.createElement("Value");
    v1.setAttribute("Fixture", 5);
    v1.setAttribute("Channel", 60);
    QDomText v1Text = doc.createTextNode("100");
    v1.appendChild(v1Text);
    root.appendChild(v1);

    QDomElement v2 = doc.createElement("Value");
    v2.setAttribute("Fixture", 133);
    v2.setAttribute("Channel", 4);
    QDomText v2Text = doc.createTextNode("59");
    v2.appendChild(v2Text);
    root.appendChild(v2);

    Scene s(m_doc);
    QVERIFY(s.loadXML(&root) == false);
}

void Scene_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Scene");
    root.setAttribute("Type", "Scene");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("5");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement v1 = doc.createElement("Value");
    v1.setAttribute("Fixture", 5);
    v1.setAttribute("Channel", 60);
    QDomText v1Text = doc.createTextNode("100");
    v1.appendChild(v1Text);
    root.appendChild(v1);

    QDomElement v2 = doc.createElement("Value");
    v2.setAttribute("Fixture", 133);
    v2.setAttribute("Channel", 4);
    QDomText v2Text = doc.createTextNode("59");
    v2.appendChild(v2Text);
    root.appendChild(v2);

    Scene s(m_doc);
    QVERIFY(s.loadXML(&root) == false);
}

void Scene_Test::save()
{
    Scene s(m_doc);
    s.setBus(5);
    s.setValue(1, 2, 3);
    s.setValue(4, 5, 6);

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");

    QVERIFY(s.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "Function");
    QVERIFY(root.firstChild().toElement().attribute("Type") == "Scene");

    QVERIFY(root.firstChild().firstChild().toElement().tagName() == "Bus");
    QVERIFY(root.firstChild().firstChild().toElement().attribute("Role") == "Fade");
    QVERIFY(root.firstChild().firstChild().toElement().text() == "5");

    QVERIFY(root.firstChild().firstChild().nextSibling().toElement().tagName() == "Value");
    QVERIFY(root.firstChild().firstChild().nextSibling().toElement().attribute("Fixture") == "1");
    QVERIFY(root.firstChild().firstChild().nextSibling().toElement().attribute("Channel") == "2");
    QVERIFY(root.firstChild().firstChild().nextSibling().toElement().text() == "3");

    QVERIFY(root.firstChild().firstChild().nextSibling().nextSibling().toElement().tagName() == "Value");
    QVERIFY(root.firstChild().firstChild().nextSibling().nextSibling().toElement().attribute("Fixture") == "4");
    QVERIFY(root.firstChild().firstChild().nextSibling().nextSibling().toElement().attribute("Channel") == "5");
    QVERIFY(root.firstChild().firstChild().nextSibling().nextSibling().toElement().text() == "6");
}

void Scene_Test::copyFrom()
{
    Scene s1(m_doc);
    s1.setName("First");
    s1.setBus(15);
    s1.setValue(1, 2, 3);
    s1.setValue(4, 5, 6);
    s1.setValue(7, 8, 9);

    /* Verify that scene contents are copied */
    Scene s2(m_doc);
    QVERIFY(s2.copyFrom(&s1) == true);
    QVERIFY(s2.name() == "First");
    QVERIFY(s2.busID() == 15);
    QVERIFY(s2.value(1, 2) == 3);
    QVERIFY(s2.value(4, 5) == 6);
    QVERIFY(s2.value(7, 8) == 9);

    /* Verify that a Scene gets a copy only from another Scene */
    Chaser c(m_doc);
    QVERIFY(s2.copyFrom(&c) == false);

    /* Make a third Scene */
    Scene s3(m_doc);
    s3.setName("Third");
    s3.setBus(8);
    s3.setValue(3, 1, 2);
    s3.setValue(6, 4, 5);
    s3.setValue(9, 7, 8);

    /* Verify that copying TO the same Scene a second time succeeds */
    QVERIFY(s2.copyFrom(&s3) == true);
    QVERIFY(s2.name() == "Third");
    QVERIFY(s2.busID() == 8);
    QVERIFY(s2.value(3, 1) == 2);
    QVERIFY(s2.value(6, 4) == 5);
    QVERIFY(s2.value(9, 7) == 8);
}

void Scene_Test::createCopy()
{
    Doc doc(this, m_cache);

    Scene* s1 = new Scene(m_doc);
    s1->setName("First");
    s1->setBus(15);
    s1->setValue(1, 2, 3);
    s1->setValue(4, 5, 6);
    s1->setValue(7, 8, 9);

    doc.addFunction(s1);
    QVERIFY(s1->id() != Function::invalidId());

    Function* f = s1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != s1);
    QVERIFY(f->id() != s1->id());

    Scene* copy = qobject_cast<Scene*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->busID() == 15);
    QVERIFY(copy->values().size() == 3);
    QVERIFY(copy->value(1, 2) == 3);
    QVERIFY(copy->value(4, 5) == 6);
    QVERIFY(copy->value(7, 8) == 9);
}

void Scene_Test::arm()
{
    Doc* doc = new Doc(this, m_cache);

    Fixture* fxi = new Fixture(doc);
    fxi->setName("Test Fixture");
    fxi->setAddress(15);
    fxi->setUniverse(3);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 123);
    s1->setValue(fxi->id(), 7, 45);
    s1->setValue(fxi->id(), 3, 67);
    doc->addFunction(s1);

    QVERIFY(s1->armedChannels().size() == 0);
    s1->arm();
    QVERIFY(s1->armedChannels().size() == 3);

    FadeChannel ch;
    ch = s1->armedChannels().at(0);
    QVERIFY(ch.address() == fxi->universeAddress());
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 123);

    ch = s1->armedChannels().at(1);
    QVERIFY(ch.address() == fxi->universeAddress() + 7);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 45);

    ch = s1->armedChannels().at(2);
    QVERIFY(ch.address() == fxi->universeAddress() + 3);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 67);

    s1->disarm();
    QVERIFY(s1->armedChannels().size() == 0);
    QVERIFY(s1->values().size() == 3);

    delete doc;
}

void Scene_Test::armMissingFixture()
{
    Doc* doc = new Doc(this, m_cache);

    Fixture* fxi = new Fixture(doc);
    fxi->setName("Test Fixture");
    fxi->setAddress(15);
    fxi->setUniverse(3);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 7, 45);
    s1->setValue(fxi->id() + 5, 9, 123); // Missing fixture
    s1->setValue(fxi->id(), 3, 67);
    doc->addFunction(s1);

    QVERIFY(s1->armedChannels().size() == 0);
    QVERIFY(s1->values().size() == 3);
    s1->arm();
    QVERIFY(s1->armedChannels().size() == 2);
    QVERIFY(s1->values().size() == 2); // The channel is removed

    FadeChannel ch;
    ch = s1->armedChannels().at(0);
    QVERIFY(ch.address() == fxi->universeAddress() + 7);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 45);

    ch = s1->armedChannels().at(1);
    QVERIFY(ch.address() == fxi->universeAddress() + 3);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 67);

    s1->disarm();
    QVERIFY(s1->armedChannels().size() == 0);
    QVERIFY(s1->values().size() == 2);

    delete doc;
}

void Scene_Test::armTooManyChannels()
{
    Doc* doc = new Doc(this, m_cache);

    Fixture* fxi = new Fixture(doc);
    fxi->setName("Test Fixture");
    fxi->setAddress(15);
    fxi->setUniverse(3);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 10, 123); // Channels 0 - 9 are valid
    s1->setValue(fxi->id(), 7, 45);
    s1->setValue(fxi->id(), 3, 67);
    doc->addFunction(s1);

    QVERIFY(s1->armedChannels().size() == 0);
    QVERIFY(s1->values().size() == 3);
    s1->arm();
    QVERIFY(s1->armedChannels().size() == 2);
    QVERIFY(s1->values().size() == 2); // The channel is removed

    FadeChannel ch;
    ch = s1->armedChannels().at(0);
    QVERIFY(ch.address() == fxi->universeAddress() + 7);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 45);

    ch = s1->armedChannels().at(1);
    QVERIFY(ch.address() == fxi->universeAddress() + 3);
    QVERIFY(ch.start() == 0);
    QVERIFY(ch.current() == 0);
    QVERIFY(ch.target() == 67);

    s1->disarm();
    QVERIFY(s1->armedChannels().size() == 0);
    QVERIFY(s1->values().size() == 2);

    delete doc;
}

void Scene_Test::flashUnflash()
{
    Doc* doc = new Doc(this, m_cache);

    Fixture* fxi = new Fixture(doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 123);
    s1->setValue(fxi->id(), 1, 45);
    s1->setValue(fxi->id(), 2, 67);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);
    QVERIFY(mts->m_dmxSourceList.size() == 0);

    s1->flash(mts);
    QVERIFY(mts->m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == true);

    uni.zeroIntensityChannels();

    s1->writeDMX(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == char(123));
    QVERIFY(uni.preGMValues()[1] == char(45));
    QVERIFY(uni.preGMValues()[2] == char(67));

    s1->flash(mts);
    QVERIFY(mts->m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == true);

    uni.zeroIntensityChannels();

    s1->writeDMX(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == char(123));
    QVERIFY(uni.preGMValues()[1] == char(45));
    QVERIFY(uni.preGMValues()[2] == char(67));

    s1->unFlash(mts);
    QVERIFY(mts->m_dmxSourceList.size() == 1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(s1->flashing() == false);

    uni.zeroIntensityChannels();

    s1->writeDMX(mts, &uni);
    QVERIFY(mts->m_dmxSourceList.size() == 0);
    QVERIFY(uni.preGMValues()[0] == char(0));
    QVERIFY(uni.preGMValues()[1] == char(0));
    QVERIFY(uni.preGMValues()[2] == char(0));

    s1->disarm();

    delete doc;
}

/** Test scene running with bus value 0 (takes one cycle) */
void Scene_Test::writeHTPBusZero()
{
    Doc* doc = new Doc(this, m_cache);

    Bus::instance()->setValue(Bus::defaultFade(), 0);

    Fixture* fxi = new Fixture(doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, 127);
    s1->setValue(fxi->id(), 2, 0);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    mts->startFunction(s1, false);
    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) 127);
    QVERIFY(uni.preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    mts->stopFunction(s1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    s1->disarm();

    delete mts;
    delete doc;
}

/** Test scene running with bus value 1 (takes two cycles) */
void Scene_Test::writeHTPBusOne()
{
    Doc* doc = new Doc(this, m_cache);

    Bus::instance()->setValue(Bus::defaultFade(), 1);

    Fixture* fxi = new Fixture(doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(10);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, 127);
    s1->setValue(fxi->id(), 2, 0);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    QVERIFY(s1->stopped() == true);
    mts->startFunction(s1, false);
    QVERIFY(s1->stopped() == false);

    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) 127);
    QVERIFY(uni.preGMValues()[1] == (char) 63);
    QVERIFY(uni.preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) 127);
    QVERIFY(uni.preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    mts->stopFunction(s1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    s1->disarm();

    delete mts;
    delete doc;
}

void Scene_Test::writeLTPHTPBusZero()
{
    Doc* doc = new Doc(this, m_cache);

    Bus::instance()->setValue(Bus::defaultFade(), 1);

    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, 127);
    s1->setValue(fxi->id(), 5, 0);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    QVERIFY(s1->stopped() == true);
    mts->startFunction(s1, false);
    QVERIFY(s1->stopped() == false);

    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) 127);
    QVERIFY(uni.preGMValues()[1] == (char) 63);
    QVERIFY(uni.preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) 127);
    QVERIFY(uni.preGMValues()[2] == (char) 0);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    mts->stopFunction(s1);
    QVERIFY(s1->stopped() == true);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) 127);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    s1->disarm();

    delete mts;
    delete doc;
}

void Scene_Test::writeLTPBusOne()
{
    Doc* doc = new Doc(this, m_cache);

    Bus::instance()->setValue(Bus::defaultFade(), 1);

    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, UCHAR_MAX);
    s1->setValue(fxi->id(), 2, UCHAR_MAX);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    QVERIFY(s1->stopped() == true);
    mts->startFunction(s1, false);
    QVERIFY(s1->stopped() == false);

    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) 127);
    QVERIFY(uni.preGMValues()[1] == (char) 127);
    QVERIFY(uni.preGMValues()[2] == (char) 127);
    QVERIFY(s1->stopped() == false);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[2] == (char) UCHAR_MAX);
    QVERIFY(s1->stopped() == true);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[2] == (char) UCHAR_MAX);
    QVERIFY(s1->stopped() == true);

    uni.zeroIntensityChannels();

    s1->disarm();

    delete mts;
    delete doc;
}

void Scene_Test::writeLTPReady()
{
    Doc* doc = new Doc(this, m_cache);

    Bus::instance()->setValue(Bus::defaultFade(), 1);

    const QLCFixtureDef* def = m_cache.fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    Fixture* fxi = new Fixture(doc);
    fxi->setFixtureDefinition(def, mode);
    QCOMPARE(fxi->channels(), quint32(6));
    fxi->setAddress(0);
    fxi->setUniverse(0);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, UCHAR_MAX);
    s1->setValue(fxi->id(), 2, UCHAR_MAX);
    doc->addFunction(s1);

    s1->arm();

    UniverseArray uni(4 * 512);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    QVERIFY(s1->stopped() == true);
    mts->startFunction(s1, false);
    QVERIFY(s1->stopped() == false);

    QVERIFY(uni.preGMValues()[0] == (char) 0);
    QVERIFY(uni.preGMValues()[1] == (char) 0);
    QVERIFY(uni.preGMValues()[2] == (char) 0);

    uni.write(0, 255, QLCChannel::Pan);
    uni.write(1, 255, QLCChannel::Tilt);
    uni.write(2, 255, QLCChannel::Colour);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[2] == (char) UCHAR_MAX);
    QVERIFY(s1->stopped() == true);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[2] == (char) UCHAR_MAX);
    QVERIFY(s1->stopped() == true);

    uni.zeroIntensityChannels();

    s1->write(mts, &uni);
    QVERIFY(uni.preGMValues()[0] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[1] == (char) UCHAR_MAX);
    QVERIFY(uni.preGMValues()[2] == (char) UCHAR_MAX);
    QVERIFY(s1->stopped() == true);

    uni.zeroIntensityChannels();

    s1->disarm();

    delete mts;
    delete doc;
}
