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

#include "scene_test.h"
#include "../function.h"
#include "../fixture.h"
#include "../chaser.h"
#include "../scene.h"
#include "../doc.h"

void Scene_Test::initTestCase()
{
	Bus::init(this);
	m_cache.load("../../fixtures/");
}

void Scene_Test::initial()
{
	Scene s(this);
	QVERIFY(s.type() == Function::Scene);
	QVERIFY(s.name() == "New Scene");
	QVERIFY(s.values().size() == 0);
	QVERIFY(s.id() == KNoID);
}

void Scene_Test::values()
{
	Scene s(this);
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
}

void Scene_Test::fixtureRemoval()
{
	Scene s(this);
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

	Scene s(this);
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

	Scene s(this);
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

	Scene s(this);
	QVERIFY(s.loadXML(&root) == false);
}

void Scene_Test::save()
{
	Scene s(this);
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
	Scene s1(this);
	s1.setName("First");
	s1.setBus(15);
	s1.setValue(1, 2, 3);
	s1.setValue(4, 5, 6);
	s1.setValue(7, 8, 9);

	/* Verify that scene contents are copied */
	Scene s2(this);
	QVERIFY(s2.copyFrom(&s1) == true);
	QVERIFY(s2.name() == "First");
	QVERIFY(s2.busID() == 15);
	QVERIFY(s2.value(1, 2) == 3);
	QVERIFY(s2.value(4, 5) == 6);
	QVERIFY(s2.value(7, 8) == 9);

	/* Verify that a Scene gets a copy only from another Scene */
	Chaser c(this);
	QVERIFY(s2.copyFrom(&c) == false);

	/* Make a third Scene */
	Scene s3(this);
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

	Scene* s1 = new Scene(this);
	s1->setName("First");
	s1->setBus(15);
	s1->setValue(1, 2, 3);
	s1->setValue(4, 5, 6);
	s1->setValue(7, 8, 9);

	doc.addFunction(s1);
	QVERIFY(s1->id() != KNoID);

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

	SceneChannel ch;
	ch = s1->armedChannels().at(0);
	QVERIFY(ch.address == fxi->universeAddress());
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 123);

	ch = s1->armedChannels().at(1);
	QVERIFY(ch.address == fxi->universeAddress() + 7);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 45);

	ch = s1->armedChannels().at(2);
	QVERIFY(ch.address == fxi->universeAddress() + 3);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 67);

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

	SceneChannel ch;
	ch = s1->armedChannels().at(0);
	QVERIFY(ch.address == fxi->universeAddress() + 7);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 45);

	ch = s1->armedChannels().at(1);
	QVERIFY(ch.address == fxi->universeAddress() + 3);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 67);

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

	SceneChannel ch;
	ch = s1->armedChannels().at(0);
	QVERIFY(ch.address == fxi->universeAddress() + 7);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 45);

	ch = s1->armedChannels().at(1);
	QVERIFY(ch.address == fxi->universeAddress() + 3);
	QVERIFY(ch.start == 0);
	QVERIFY(ch.current == 0);
	QVERIFY(ch.target == 67);

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

	QByteArray uni(4 * 512, 0);
	s1->flash(&uni);
	QVERIFY(int(uni[0]) == 123);
	QVERIFY(int(uni[1]) == 45);
	QVERIFY(int(uni[2]) == 67);

	s1->flash(&uni);
	QVERIFY(int(uni[0]) == 123);
	QVERIFY(int(uni[1]) == 45);
	QVERIFY(int(uni[2]) == 67);

	s1->unFlash(&uni);
	QVERIFY(int(uni[0]) == 0);
	QVERIFY(int(uni[1]) == 0);
	QVERIFY(int(uni[2]) == 0);

	s1->disarm();

	delete doc;
}

void Scene_Test::writeBusZero()
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
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 127);
	s1->setValue(fxi->id(), 2, 0);
	doc->addFunction(s1);

	s1->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	s1->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == s1);
	
	QByteArray uni(4 * 512, 0);
	QVERIFY(uni[0] == (char) 0);
	QVERIFY(uni[1] == (char) 0);
	QVERIFY(uni[2] == (char) 0);

	QVERIFY(s1->write(&uni) == false);
	QVERIFY(uni[0] == (char) 255);
	QVERIFY(uni[1] == (char) 127);
	QVERIFY(uni[2] == (char) 0);

	s1->stop(mts);
	QVERIFY(mts->m_list.size() == 0);
	s1->disarm();

	delete mts;
	delete doc;
}

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

MasterTimerStub::MasterTimerStub(QObject* parent) : MasterTimer(parent, NULL)
{
}

MasterTimerStub::~MasterTimerStub()
{
}

void MasterTimerStub::startFunction(Function* function)
{
	m_list.append(function);
}

void MasterTimerStub::stopFunction(Function* function)
{
	m_list.removeAll(function);
}
