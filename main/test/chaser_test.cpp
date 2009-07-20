/*
  Q Light Controller - Unit test
  chaser_test.cpp

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

#include "chaser_test.h"
#include "../function.h"
#include "../fixture.h"
#include "../chaser.h"
#include "../scene.h"
#include "../doc.h"

void Chaser_Test::initTestCase()
{
	Bus::init(this);
	m_cache.load("../../fixtures/");
}

void Chaser_Test::initial()
{
	Chaser c(this);
	QVERIFY(c.type() == Function::Chaser);
	QVERIFY(c.name() == "New Chaser");
	QVERIFY(c.steps().size() == 0);
	QVERIFY(c.id() == KNoID);
}

void Chaser_Test::steps()
{
	Chaser c(this);
	c.setID(50);
	QVERIFY(c.steps().size() == 0);

	/* A chaser should not be allowed to be its own member */
	QVERIFY(c.addStep(50) == false);
	QVERIFY(c.steps().size() == 0);

	/* Add a function with id "12" to the chaser */
	c.addStep(12);
	QVERIFY(c.steps().size() == 1);
	QVERIFY(c.steps().at(0) == 12);

	/* Add another function in the middle */
	c.addStep(34);
	QVERIFY(c.steps().size() == 2);
	QVERIFY(c.steps().at(0) == 12);
	QVERIFY(c.steps().at(1) == 34);

	/* Must be able to add the same function multiple times */
	c.addStep(12);
	QVERIFY(c.steps().size() == 3);
	QVERIFY(c.steps().at(0) == 12);
	QVERIFY(c.steps().at(1) == 34);
	QVERIFY(c.steps().at(2) == 12);

	/* Removing a non-existent index should make no modifications */
	QVERIFY(c.removeStep(3) == false);
	QVERIFY(c.steps().size() == 3);
	QVERIFY(c.steps().at(0) == 12);
	QVERIFY(c.steps().at(1) == 34);
	QVERIFY(c.steps().at(2) == 12);

	/* Removing the last step should succeed */
	QVERIFY(c.removeStep(2) == true);
	QVERIFY(c.steps().size() == 2);
	QVERIFY(c.steps().at(0) == 12);
	QVERIFY(c.steps().at(1) == 34);

	/* Removing the first step should succeed */
	QVERIFY(c.removeStep(0) == true);
	QVERIFY(c.steps().size() == 1);
	QVERIFY(c.steps().at(0) == 34);

	/* Removing the only step should succeed */
	QVERIFY(c.removeStep(0) == true);
	QVERIFY(c.steps().size() == 0);

	/* Add some new steps to test raising & lowering */
	c.addStep(0);
	c.addStep(1);
	c.addStep(2);
	c.addStep(3);

	QVERIFY(c.raiseStep(0) == false);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 1);
	QVERIFY(c.steps().at(2) == 2);
	QVERIFY(c.steps().at(3) == 3);

	QVERIFY(c.raiseStep(1) == true);
	QVERIFY(c.steps().at(0) == 1);
	QVERIFY(c.steps().at(1) == 0);
	QVERIFY(c.steps().at(2) == 2);
	QVERIFY(c.steps().at(3) == 3);

	QVERIFY(c.raiseStep(1) == true);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 1);
	QVERIFY(c.steps().at(2) == 2);
	QVERIFY(c.steps().at(3) == 3);

	QVERIFY(c.lowerStep(3) == false);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 1);
	QVERIFY(c.steps().at(2) == 2);
	QVERIFY(c.steps().at(3) == 3);

	QVERIFY(c.lowerStep(1) == true);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 2);
	QVERIFY(c.steps().at(2) == 1);
	QVERIFY(c.steps().at(3) == 3);

	QVERIFY(c.lowerStep(0) == true);
	QVERIFY(c.steps().at(0) == 2);
	QVERIFY(c.steps().at(1) == 0);
	QVERIFY(c.steps().at(2) == 1);
	QVERIFY(c.steps().at(3) == 3);
}

void Chaser_Test::functionRemoval()
{
	Chaser c(this);
	c.setID(42);
	QVERIFY(c.steps().size() == 0);

	QVERIFY(c.addStep(0) == true);
	QVERIFY(c.addStep(1) == true);
	QVERIFY(c.addStep(2) == true);
	QVERIFY(c.addStep(3) == true);
	QVERIFY(c.steps().size() == 4);

	/* Simulate function removal signal with an uninteresting function id */
	c.slotFunctionRemoved(6);
	QVERIFY(c.steps().size() == 4);

	/* Simulate function removal signal with a function in the chaser */
	c.slotFunctionRemoved(1);
	QVERIFY(c.steps().size() == 3);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 2);
	QVERIFY(c.steps().at(2) == 3);

	/* Simulate function removal signal with an invalid function id */
	c.slotFunctionRemoved(Function::invalidId());
	QVERIFY(c.steps().size() == 3);
	QVERIFY(c.steps().at(0) == 0);
	QVERIFY(c.steps().at(1) == 2);
	QVERIFY(c.steps().at(2) == 3);

	/* Simulate function removal signal with a function in the chaser */
	c.slotFunctionRemoved(0);
	QVERIFY(c.steps().size() == 2);
	QVERIFY(c.steps().at(0) == 2);
	QVERIFY(c.steps().at(1) == 3);
}

void Chaser_Test::loadSuccess()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "Chaser");

	QDomElement bus = doc.createElement("Bus");
	bus.setAttribute("Role", "Hold");
	QDomText busText = doc.createTextNode("16");
	bus.appendChild(busText);
	root.appendChild(bus);

	QDomElement dir = doc.createElement("Direction");
	QDomText dirText = doc.createTextNode("Backward");
	dir.appendChild(dirText);
	root.appendChild(dir);

	QDomElement run = doc.createElement("RunOrder");
	QDomText runText = doc.createTextNode("SingleShot");
	run.appendChild(runText);
	root.appendChild(run);

	QDomElement s1 = doc.createElement("Step");
	s1.setAttribute("Number", 1);
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	s2.setAttribute("Number", 2);
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	s3.setAttribute("Number", 0);
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Chaser c(this);
	QVERIFY(c.loadXML(&root) == true);
	QVERIFY(c.busID() == 16);
	QVERIFY(c.direction() == Chaser::Backward);
	QVERIFY(c.runOrder() == Chaser::SingleShot);
	QVERIFY(c.steps().size() == 3);
	QVERIFY(c.steps().at(0) == 87);
	QVERIFY(c.steps().at(1) == 50);
	QVERIFY(c.steps().at(2) == 12);
}

void Chaser_Test::loadWrongType()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "Scene");

	QDomElement bus = doc.createElement("Bus");
	bus.setAttribute("Role", "Hold");
	QDomText busText = doc.createTextNode("16");
	bus.appendChild(busText);
	root.appendChild(bus);

	QDomElement dir = doc.createElement("Direction");
	QDomText dirText = doc.createTextNode("Backward");
	dir.appendChild(dirText);
	root.appendChild(dir);

	QDomElement run = doc.createElement("RunOrder");
	QDomText runText = doc.createTextNode("SingleShot");
	run.appendChild(runText);
	root.appendChild(run);

	QDomElement s1 = doc.createElement("Step");
	s1.setAttribute("Number", 1);
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	s2.setAttribute("Number", 2);
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	s3.setAttribute("Number", 0);
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Chaser c(this);
	QVERIFY(c.loadXML(&root) == false);
}

void Chaser_Test::loadWrongRoot()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Chaser");
	root.setAttribute("Type", "Chaser");

	QDomElement bus = doc.createElement("Bus");
	bus.setAttribute("Role", "Hold");
	QDomText busText = doc.createTextNode("16");
	bus.appendChild(busText);
	root.appendChild(bus);

	QDomElement dir = doc.createElement("Direction");
	QDomText dirText = doc.createTextNode("Backward");
	dir.appendChild(dirText);
	root.appendChild(dir);

	QDomElement run = doc.createElement("RunOrder");
	QDomText runText = doc.createTextNode("SingleShot");
	run.appendChild(runText);
	root.appendChild(run);

	QDomElement s1 = doc.createElement("Step");
	s1.setAttribute("Number", 1);
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	s2.setAttribute("Number", 2);
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	s3.setAttribute("Number", 0);
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Chaser c(this);
	QVERIFY(c.loadXML(&root) == false);
}

#if 0
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

/** Test scene running with bus value 0 (takes one cycle) */
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

/** Test scene running with bus value 1 (takes two cycles) */
void Scene_Test::writeBusOne()
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

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 127);
	QVERIFY(uni[1] == (char) 63);
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

/** Test scene running with bus value 2 (takes three cycles) */
void Scene_Test::writeBusTwo()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 2);

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

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 85);
	QVERIFY(uni[1] == (char) 42);
	QVERIFY(uni[2] == (char) 0);

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 170);
	QVERIFY(uni[1] == (char) 84);
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

/** Test scene running with initial bus value 5 (takes 6 cycles) that is
    changed in the middle to 0 */
void Scene_Test::writeBusFiveChangeToZeroInTheMiddle()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 5);

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

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 42);
	QVERIFY(uni[1] == (char) 21);
	QVERIFY(uni[2] == (char) 0);

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 85);
	QVERIFY(uni[1] == (char) 42);
	QVERIFY(uni[2] == (char) 0);

	Bus::instance()->setValue(Bus::defaultFade(), 0);

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

/** Test scene running with initial values something else than zero */
void Scene_Test::writeNonZeroStartingValues()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 2);

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
	uni[0] = (char) 100;
	uni[1] = (char) 255;
	uni[2] = (char) 3;

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 151);
	QVERIFY(uni[1] == (char) 213);
	QVERIFY(uni[2] == (char) 2);

	QVERIFY(s1->write(&uni) == true);
	QVERIFY(uni[0] == (char) 203);
	QVERIFY(uni[1] == (char) 170);
	QVERIFY(uni[2] == (char) 1);

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
#endif
