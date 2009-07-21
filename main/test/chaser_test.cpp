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

#include "mastertimer_stub.h"
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
	QVERIFY(c.direction() == Chaser::Forward);
	QVERIFY(c.runOrder() == Chaser::Loop);
	QVERIFY(c.id() == KNoID);
}

void Chaser_Test::directionRunOrder()
{
	Chaser c(this);

	QVERIFY(c.direction() == Chaser::Forward);
	QVERIFY(c.runOrder() == Chaser::Loop);

	c.setDirection(Chaser::Backward);
	QVERIFY(c.direction() == Chaser::Backward);

	c.setRunOrder(Chaser::PingPong);
	QVERIFY(c.runOrder() == Chaser::PingPong);

	c.setDirection(Chaser::Forward);
	QVERIFY(c.direction() == Chaser::Forward);

	c.setRunOrder(Chaser::SingleShot);
	QVERIFY(c.runOrder() == Chaser::SingleShot);

	c.setDirection(Chaser::Backward);
	QVERIFY(c.direction() == Chaser::Backward);

	c.setRunOrder(Chaser::Loop);
	QVERIFY(c.runOrder() == Chaser::Loop);

	/* Check that invalid direction results in a sane fallback value */
	c.setDirection(Chaser::Direction(15));
	QVERIFY(c.direction() == Chaser::Forward);

	/* Check that invalid run order results in a sane fallback value */
	c.setRunOrder(Chaser::RunOrder(42));
	QVERIFY(c.runOrder() == Chaser::Loop);
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

void Chaser_Test::save()
{
	Chaser c(this);
	c.setDirection(Chaser::Backward);
	c.setRunOrder(Chaser::SingleShot);
	c.setBus(9);
	c.addStep(3);
	c.addStep(1);
	c.addStep(0);
	c.addStep(2);

	QDomDocument doc;
	QDomElement root = doc.createElement("TestRoot");

	QVERIFY(c.saveXML(&doc, &root) == true);
	QVERIFY(root.firstChild().toElement().tagName() == "Function");
	QVERIFY(root.firstChild().toElement().attribute("Type") == "Chaser");

	QVERIFY(root.firstChild().firstChild().toElement().tagName() == "Bus");
	QVERIFY(root.firstChild().firstChild().toElement().attribute("Role") == "Hold");
	QVERIFY(root.firstChild().firstChild().toElement().text() == "9");

	QDomNode node = root.firstChild().firstChild();
	bool bus = false, run = false, dir = false;
	int fids = 0;
	while (node.isNull() == false)
	{
		QDomElement tag = node.toElement();
		if (tag.tagName() == "Bus")
		{
			QVERIFY(tag.text().toUInt() == 9);
			QVERIFY(tag.attribute("Role") == "Hold");
			bus = true;
		}
		else if (tag.tagName() == "Direction")
		{
			QVERIFY(tag.text() == "Backward");
			dir = true;
		}
		else if (tag.tagName() == "RunOrder")
		{
			QVERIFY(tag.text() == "SingleShot");
			run = true;
		}
		else if (tag.tagName() == "Step")
		{
			t_function_id fid = tag.text().toUInt();
			QVERIFY(fid == 0 || fid == 1 || fid == 2 || fid == 3);
			fids++;
		}
		else
		{
			QFAIL("Unhandled XML tag.");
		}

		node = node.nextSibling();
	}

	QVERIFY(bus == true);
	QVERIFY(dir == true);
	QVERIFY(run == true);
	QVERIFY(fids == 4);
}

void Chaser_Test::copyFrom()
{
	Chaser c1(this);
	c1.setName("First");
	c1.setDirection(Chaser::Backward);
	c1.setRunOrder(Chaser::PingPong);
	c1.setBus(15);
	c1.addStep(2);
	c1.addStep(0);
	c1.addStep(1);
	c1.addStep(25);

	/* Verify that chaser contents are copied */
	Chaser c2(this);
	QVERIFY(c2.copyFrom(&c1) == true);
	QVERIFY(c2.name() == "First");
	QVERIFY(c2.busID() == 15);
	QVERIFY(c2.direction() == Chaser::Backward);
	QVERIFY(c2.runOrder() == Chaser::PingPong);
	QVERIFY(c2.steps().size() == 4);
	QVERIFY(c2.steps().at(0) == 2);
	QVERIFY(c2.steps().at(1) == 0);
	QVERIFY(c2.steps().at(2) == 1);
	QVERIFY(c2.steps().at(3) == 25);

	/* Verify that a Chaser gets a copy only from another Chaser */
	Scene s(this);
	QVERIFY(c2.copyFrom(&s) == false);

	/* Make a third Chaser */
	Chaser c3(this);
	c3.setName("Third");
	c3.setBus(8);
	c3.setDirection(Chaser::Forward);
	c3.setRunOrder(Chaser::Loop);
	c3.addStep(15);
	c3.addStep(94);
	c3.addStep(3);

	/* Verify that copying TO the same Chaser a second time succeeds and
	   that steps are not appended but replaced completely. */
	QVERIFY(c2.copyFrom(&c3) == true);
	QVERIFY(c2.name() == "Third");
	QVERIFY(c2.busID() == 8);
	QVERIFY(c2.direction() == Chaser::Forward);
	QVERIFY(c2.runOrder() == Chaser::Loop);
	QVERIFY(c2.steps().size() == 3);
	QVERIFY(c2.steps().at(0) == 15);
	QVERIFY(c2.steps().at(1) == 94);
	QVERIFY(c2.steps().at(2) == 3);
}

void Chaser_Test::createCopy()
{
	Doc doc(this, m_cache);

	Chaser* c1 = new Chaser(this);
	c1->setName("First");
	c1->setBus(15);
	c1->setDirection(Chaser::Backward);
	c1->setRunOrder(Chaser::SingleShot);
	c1->addStep(20);
	c1->addStep(30);
	c1->addStep(40);

	doc.addFunction(c1);
	QVERIFY(c1->id() != KNoID);

	Function* f = c1->createCopy(&doc);
	QVERIFY(f != NULL);
	QVERIFY(f != c1);
	QVERIFY(f->id() != c1->id());

	Chaser* copy = qobject_cast<Chaser*> (f);
	QVERIFY(copy != NULL);
	QVERIFY(copy->busID() == 15);
	QVERIFY(copy->direction() == Chaser::Backward);
	QVERIFY(copy->runOrder() == Chaser::SingleShot);
	QVERIFY(copy->steps().size() == 3);
	QVERIFY(copy->steps().at(0) == 20);
	QVERIFY(copy->steps().at(1) == 30);
	QVERIFY(copy->steps().at(2) == 40);
}

void Chaser_Test::armSuccess()
{
	Doc* doc = new Doc(this, m_cache);

	Fixture* fxi = new Fixture(doc);
	fxi->setName("Test Fixture");
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);
	QVERIFY(s1->id() != Function::invalidId());

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);
	QVERIFY(s2->id() != Function::invalidId());

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->addStep(s1->id());
	c->addStep(s2->id());

	QVERIFY(c->steps().size() == 2);
	c->arm();
	QVERIFY(c->steps().size() == 2);

	delete doc;
}

void Chaser_Test::armMissingFunction()
{
	Doc* doc = new Doc(this, m_cache);

	Fixture* fxi = new Fixture(doc);
	fxi->setName("Test Fixture");
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->addStep(s1->id());
	c->addStep(123); // Nonexistent function
	c->addStep(s2->id());
	c->addStep(55); // Nonexistent function

	QVERIFY(c->steps().size() == 4);
	c->arm();
	QVERIFY(c->steps().size() == 2); // Nonexistent functions are removed

	delete doc;
}

void Chaser_Test::writeBusZeroLoopForward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->addStep(s1->id());
	c->addStep(s2->id());

	s1->arm();
	s2->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* Since hold bus' value is 0 each call to write() starts the next
	   step in chaser. */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	c->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();

	delete mts;
	delete doc;
}

void Chaser_Test::writeBusZeroLoopBackward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->setDirection(Chaser::Backward);
	c->addStep(s1->id());
	c->addStep(s2->id());

	s1->arm();
	s2->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* Since hold bus' value is 0 each call to write() starts the next
	   step in chaser. Last step comes first because the chaser is now
	   running backwards. */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	c->stop(mts);
	s1->stop(mts);
	s2->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();

	delete mts;
	delete doc;
}

void Chaser_Test::writeBusZeroSingleShotForward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->setRunOrder(Chaser::SingleShot);
	c->addStep(s1->id());
	c->addStep(s2->id());

	s1->arm();
	s2->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* SingleShot will stop automatically after the last step */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == false);
	c->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();

	delete mts;
	delete doc;
}

void Chaser_Test::writeBusZeroSingleShotBackward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 0);
	s2->setValue(fxi->id(), 1, 0);
	doc->addFunction(s2);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->setRunOrder(Chaser::SingleShot);
	c->setDirection(Chaser::Backward);
	c->addStep(s1->id());
	c->addStep(s2->id());

	s1->arm();
	s2->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* SingleShot will stop automatically after the last step */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == false);
	c->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();

	delete mts;
	delete doc;
}

void Chaser_Test::writeBusZeroPingPongForward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 127);
	s2->setValue(fxi->id(), 1, 127);
	doc->addFunction(s2);

	Scene* s3 = new Scene(doc);
	s3->setName("Scene2");
	s3->setValue(fxi->id(), 0, 0);
	s3->setValue(fxi->id(), 1, 0);
	doc->addFunction(s3);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->setRunOrder(Chaser::PingPong);
	c->addStep(s1->id());
	c->addStep(s2->id());
	c->addStep(s3->id());

	s1->arm();
	s2->arm();
	s3->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* Since hold bus' value is 0 each call to write() starts the next
	   step in chaser. Last step comes first because the chaser is now
	   running backwards. */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s3);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s3);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	c->stop(mts);
	s1->stop(mts);
	s2->stop(mts);
	s3->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();
	s3->disarm();

	delete mts;
	delete doc;
}

void Chaser_Test::writeBusZeroPingPongBackward()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(2);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 0, 127);
	s2->setValue(fxi->id(), 1, 127);
	doc->addFunction(s2);

	Scene* s3 = new Scene(doc);
	s3->setName("Scene2");
	s3->setValue(fxi->id(), 0, 0);
	s3->setValue(fxi->id(), 1, 0);
	doc->addFunction(s3);

	Chaser* c = new Chaser(doc);
	c->setName("Chaser");
	c->setRunOrder(Chaser::PingPong);
	c->setDirection(Chaser::Backward);
	c->addStep(s1->id());
	c->addStep(s2->id());
	c->addStep(s3->id());

	s1->arm();
	s2->arm();
	s3->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);
	c->start(mts);

	QVERIFY(mts->m_list.size() == 1);
	QVERIFY(mts->m_list[0] == c);

	QByteArray uni;

	/* Since hold bus' value is 0 each call to write() starts the next
	   step in chaser. Last step comes first because the chaser is now
	   running backwards. */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s3);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s3);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s1);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s2);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == c);
	QVERIFY(mts->m_list[1] == s3);

	c->stop(mts);
	s1->stop(mts);
	s2->stop(mts);
	s3->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();
	s3->disarm();

	delete mts;
	delete doc;
}
