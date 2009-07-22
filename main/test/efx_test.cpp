/*
  Q Light Controller - Unit test
  efx_test.cpp

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
#include "efx_test.h"

#include "../function.h"
#include "../fixture.h"
#include "../scene.h"
#include "../efx.h"
#include "../doc.h"

void EFX_Test::initTestCase()
{
	Bus::init(this);
	m_cache.load("../../fixtures/");
}

void EFX_Test::initial()
{
	EFX e(this);
	QVERIFY(e.type() == Function::EFX);
	QVERIFY(e.name() == "New EFX");
	QVERIFY(e.id() == KNoID);

	QVERIFY(e.algorithm() == "Circle");
	QVERIFY(e.width() == 127);
	QVERIFY(e.height() == 127);
	QVERIFY(e.rotation() == 0);
	QVERIFY(e.xOffset() == 127);
	QVERIFY(e.yOffset() == 127);

	QVERIFY(e.xFrequency() == 2);
	QVERIFY(e.yFrequency() == 3);
	QVERIFY(e.isFrequencyEnabled() == false);

	QVERIFY(e.yPhase() == 0);
	QVERIFY(e.xPhase() == 90);
	QVERIFY(e.isPhaseEnabled() == false);

	QVERIFY(e.fixtures().size() == 0);
	QVERIFY(e.propagationMode() == EFX::Parallel);

	QVERIFY(e.startScene() == Function::invalidId());
	QVERIFY(e.startSceneEnabled() == false);
	QVERIFY(e.stopScene() == Function::invalidId());
	QVERIFY(e.stopSceneEnabled() == false);
}

void EFX_Test::algorithmNames()
{
	QStringList list = EFX::algorithmList();
	QVERIFY(list.size() == 5);
	QVERIFY(list.contains("Circle") == true);
	QVERIFY(list.contains("Eight") == true);
	QVERIFY(list.contains("Line") == true);
	QVERIFY(list.contains("Diamond") == true);
	QVERIFY(list.contains("Lissajous") == true);

	EFX e(this);

	/* All EFX's have Circle as the initial algorithm */
	QVERIFY(e.algorithm() == "Circle");

	e.setAlgorithm("Eight");
	QVERIFY(e.algorithm() == "Eight");

	e.setAlgorithm("Line");
	QVERIFY(e.algorithm() == "Line");

	e.setAlgorithm("Diamond");
	QVERIFY(e.algorithm() == "Diamond");

	e.setAlgorithm("Lissajous");
	QVERIFY(e.algorithm() == "Lissajous");

	/* Invalid algorithm name results in Circle as a fallback */
	e.setAlgorithm("Foo");
	QVERIFY(e.algorithm() == "Circle");
}

void EFX_Test::width()
{
	EFX e(this);

	e.setWidth(300);
	QVERIFY(e.width() == 127);

	e.setWidth(128);
	QVERIFY(e.width() == 127);

	e.setWidth(0);
	QVERIFY(e.width() == 0);

	e.setWidth(52);
	QVERIFY(e.width() == 52);

	e.setWidth(-4);
	QVERIFY(e.width() == 0);
}

void EFX_Test::height()
{
	EFX e(this);

	e.setHeight(300);
	QVERIFY(e.height() == 127);

	e.setHeight(128);
	QVERIFY(e.height() == 127);

	e.setHeight(0);
	QVERIFY(e.height() == 0);

	e.setHeight(12);
	QVERIFY(e.height() == 12);

	e.setHeight(-4);
	QVERIFY(e.height() == 0);
}

void EFX_Test::rotation()
{
	EFX e(this);

	e.setRotation(400);
	QVERIFY(e.rotation() == 359);

	e.setRotation(360);
	QVERIFY(e.rotation() == 359);

	e.setRotation(0);
	QVERIFY(e.rotation() == 0);

	e.setRotation(12);
	QVERIFY(e.rotation() == 12);

	e.setRotation(-4);
	QVERIFY(e.rotation() == 0);
}

void EFX_Test::xOffset()
{
	EFX e(this);

	e.setXOffset(300);
	QVERIFY(e.xOffset() == 127);

	e.setXOffset(128);
	QVERIFY(e.xOffset() == 127);

	e.setXOffset(0);
	QVERIFY(e.xOffset() == 0);

	e.setXOffset(12);
	QVERIFY(e.xOffset() == 12);

	e.setXOffset(-4);
	QVERIFY(e.xOffset() == 0);
}

void EFX_Test::yOffset()
{
	EFX e(this);

	e.setYOffset(300);
	QVERIFY(e.yOffset() == 127);

	e.setYOffset(128);
	QVERIFY(e.yOffset() == 127);

	e.setYOffset(0);
	QVERIFY(e.yOffset() == 0);

	e.setYOffset(12);
	QVERIFY(e.yOffset() == 12);

	e.setYOffset(-4);
	QVERIFY(e.yOffset() == 0);
}

void EFX_Test::xFrequency()
{
	EFX e(this);

	QVERIFY(e.isFrequencyEnabled() == false);

	e.setXFrequency(10);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(6);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(0);
	QVERIFY(e.xFrequency() == 0);

	e.setXFrequency(3);
	QVERIFY(e.xFrequency() == 3);

	e.setXFrequency(-4);
	QVERIFY(e.xFrequency() == 0);

	e.setAlgorithm("Lissajous");
	QVERIFY(e.isFrequencyEnabled() == true);

	e.setXFrequency(10);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(6);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(0);
	QVERIFY(e.xFrequency() == 0);

	e.setXFrequency(3);
	QVERIFY(e.xFrequency() == 3);

	e.setXFrequency(-4);
	QVERIFY(e.xFrequency() == 0);
}

void EFX_Test::yFrequency()
{
	EFX e(this);

	QVERIFY(e.isFrequencyEnabled() == false);

	e.setYFrequency(10);
	QVERIFY(e.yFrequency() == 5);

	e.setYFrequency(6);
	QVERIFY(e.yFrequency() == 5);

	e.setYFrequency(0);
	QVERIFY(e.yFrequency() == 0);

	e.setYFrequency(3);
	QVERIFY(e.yFrequency() == 3);

	e.setYFrequency(-4);
	QVERIFY(e.yFrequency() == 0);

	e.setAlgorithm("Lissajous");
	QVERIFY(e.isFrequencyEnabled() == true);

	e.setXFrequency(10);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(6);
	QVERIFY(e.xFrequency() == 5);

	e.setXFrequency(0);
	QVERIFY(e.xFrequency() == 0);

	e.setXFrequency(3);
	QVERIFY(e.xFrequency() == 3);

	e.setXFrequency(-4);
	QVERIFY(e.xFrequency() == 0);
}

void EFX_Test::xPhase()
{
	EFX e(this);

	QVERIFY(e.isPhaseEnabled() == false);

	e.setXPhase(400);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(360);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(0);
	QVERIFY(e.xPhase() == 0);

	e.setXPhase(359);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(46);
	QVERIFY(e.xPhase() == 46);

	e.setXPhase(-4);
	QVERIFY(e.xPhase() == 0);

	e.setAlgorithm("Lissajous");
	QVERIFY(e.isPhaseEnabled() == true);

	e.setXPhase(400);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(360);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(0);
	QVERIFY(e.xPhase() == 0);

	e.setXPhase(359);
	QVERIFY(e.xPhase() == 359);

	e.setXPhase(46);
	QVERIFY(e.xPhase() == 46);

	e.setXPhase(-4);
	QVERIFY(e.xPhase() == 0);
}

void EFX_Test::yPhase()
{
	EFX e(this);

	QVERIFY(e.isPhaseEnabled() == false);

	e.setYPhase(400);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(360);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(0);
	QVERIFY(e.yPhase() == 0);

	e.setYPhase(359);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(152);
	QVERIFY(e.yPhase() == 152);

	e.setYPhase(-4);
	QVERIFY(e.yPhase() == 0);

	e.setAlgorithm("Lissajous");
	QVERIFY(e.isPhaseEnabled() == true);

	e.setYPhase(400);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(360);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(0);
	QVERIFY(e.yPhase() == 0);

	e.setYPhase(359);
	QVERIFY(e.yPhase() == 359);

	e.setYPhase(152);
	QVERIFY(e.yPhase() == 152);

	e.setYPhase(-4);
	QVERIFY(e.yPhase() == 0);
}

void EFX_Test::fixtures()
{
	EFX* e = new EFX(this);
	QVERIFY(e->fixtures().size() == 0);

	/* Add first fixture */
	EFXFixture* ef1 = new EFXFixture(e);
	ef1->setFixture(12);
	QVERIFY(e->addFixture(ef1) == true);
	QVERIFY(e->fixtures().size() == 1);

	/* Add second fixture */
	EFXFixture* ef2 = new EFXFixture(e);
	ef2->setFixture(34);
	QVERIFY(e->addFixture(ef2) == true);
	QVERIFY(e->fixtures().size() == 2);

	/* Must not be able to add the same fixture twice */
	QVERIFY(e->addFixture(ef1) == false);
	QVERIFY(e->addFixture(ef2) == false);
	QVERIFY(e->fixtures().size() == 2);

	/* Try to remove a non-member fixture */
	EFXFixture* ef3 = new EFXFixture(e);
	ef3->setFixture(56);
	QVERIFY(e->removeFixture(ef3) == false);
	QVERIFY(e->fixtures().size() == 2);

	/* Add third fixture */
	e->addFixture(ef3);
	QVERIFY(e->fixtures().size() == 3);
	QVERIFY(e->fixtures().at(0) == ef1);
	QVERIFY(e->fixtures().at(1) == ef2);
	QVERIFY(e->fixtures().at(2) == ef3);

	/* Add fourth fixture */
	EFXFixture* ef4 = new EFXFixture(e);
	ef4->setFixture(78);
	e->addFixture(ef4);
	QVERIFY(e->fixtures().size() == 4);
	QVERIFY(e->fixtures().at(0) == ef1);
	QVERIFY(e->fixtures().at(1) == ef2);
	QVERIFY(e->fixtures().at(2) == ef3);
	QVERIFY(e->fixtures().at(3) == ef4);

	QVERIFY(e->removeFixture(ef4) == true);
	QVERIFY(e->fixtures().at(0) == ef1);
	QVERIFY(e->fixtures().at(1) == ef2);
	QVERIFY(e->fixtures().at(2) == ef3);
	delete ef4;

	/* Raising the first and lowering the last must not succeed */
	QVERIFY(e->raiseFixture(ef1) == false);
	QVERIFY(e->lowerFixture(ef3) == false);
	QVERIFY(e->fixtures().at(0) == ef1);
	QVERIFY(e->fixtures().at(1) == ef2);
	QVERIFY(e->fixtures().at(2) == ef3);

	QVERIFY(e->raiseFixture(ef2) == true);
	QVERIFY(e->fixtures().at(0) == ef2);
	QVERIFY(e->fixtures().at(1) == ef1);
	QVERIFY(e->fixtures().at(2) == ef3);
	QVERIFY(e->raiseFixture(ef2) == false);

	QVERIFY(e->lowerFixture(ef1) == true);
	QVERIFY(e->fixtures().at(0) == ef2);
	QVERIFY(e->fixtures().at(1) == ef3);
	QVERIFY(e->fixtures().at(2) == ef1);
	QVERIFY(e->lowerFixture(ef1) == false);

	/* Uninteresting fixture is removed */
	e->slotFixtureRemoved(99);
	QVERIFY(e->fixtures().size() == 3);
	QVERIFY(e->fixtures().at(0) == ef2);
	QVERIFY(e->fixtures().at(1) == ef3);
	QVERIFY(e->fixtures().at(2) == ef1);

	/* Member fixture is removed */
	e->slotFixtureRemoved(34);
	QVERIFY(e->fixtures().size() == 2);
	QVERIFY(e->fixtures().at(0) == ef3);
	QVERIFY(e->fixtures().at(1) == ef1);

	/* Non-Member fixture is removed */
	e->slotFixtureRemoved(34);
	QVERIFY(e->fixtures().size() == 2);
	QVERIFY(e->fixtures().at(0) == ef3);
	QVERIFY(e->fixtures().at(1) == ef1);

	/* Member fixture is removed */
	e->slotFixtureRemoved(12);
	QVERIFY(e->fixtures().size() == 1);
	QVERIFY(e->fixtures().at(0) == ef3);

	/* Member fixture is removed */
	e->slotFixtureRemoved(56);
	QVERIFY(e->fixtures().size() == 0);

	delete e;
}

void EFX_Test::propagationMode()
{
	EFX e(this);
	QVERIFY(e.propagationMode() == EFX::Parallel);

	e.setPropagationMode(EFX::Serial);
	QVERIFY(e.propagationMode() == EFX::Serial);

	QVERIFY(EFX::propagationModeToString(EFX::Serial) == "Serial");
	QVERIFY(EFX::propagationModeToString(EFX::Parallel) == "Parallel");
	QVERIFY(EFX::propagationModeToString(EFX::PropagationMode(7)) == "Parallel");

	QVERIFY(EFX::stringToPropagationMode("Serial") == EFX::Serial);
	QVERIFY(EFX::stringToPropagationMode("Parallel") == EFX::Parallel);
	QVERIFY(EFX::stringToPropagationMode("Foobar") == EFX::Parallel);
}

void EFX_Test::startStopScenes()
{
	EFX e(this);

	QVERIFY(e.startSceneEnabled() == false);
	QVERIFY(e.startScene() == Function::invalidId());

	QVERIFY(e.stopSceneEnabled() == false);
	QVERIFY(e.stopScene() == Function::invalidId());

	e.setStartScene(15);
	QVERIFY(e.startSceneEnabled() == false);
	QVERIFY(e.startScene() == 15);

	e.setStartSceneEnabled(true);
	QVERIFY(e.startSceneEnabled() == true);
	QVERIFY(e.startScene() == 15);

	e.setStopScene(73);
	QVERIFY(e.stopSceneEnabled() == false);
	QVERIFY(e.stopScene() == 73);

	e.setStopSceneEnabled(true);
	QVERIFY(e.stopSceneEnabled() == true);
	QVERIFY(e.stopScene() == 73);

	e.setStartScene(Function::invalidId());
	QVERIFY(e.startSceneEnabled() == false);
	QVERIFY(e.startScene() == Function::invalidId());

	e.setStopScene(Function::invalidId());
	QVERIFY(e.stopSceneEnabled() == false);
	QVERIFY(e.stopScene() == Function::invalidId());

	/* Test function removal slot */
	e.setStartScene(15);
	e.setStartSceneEnabled(true);
	e.setStopScene(20);
	e.setStopSceneEnabled(true);

	e.slotFunctionRemoved(10);
	QVERIFY(e.startScene() == 15);
	QVERIFY(e.stopScene() == 20);

	e.slotFunctionRemoved(15);
	QVERIFY(e.startScene() == Function::invalidId());
	QVERIFY(e.startSceneEnabled() == false);
	QVERIFY(e.stopScene() == 20);
	QVERIFY(e.stopSceneEnabled() == true);

	e.setStartScene(15);
	e.setStartSceneEnabled(true);

	e.slotFunctionRemoved(20);
	QVERIFY(e.startScene() == 15);
	QVERIFY(e.startSceneEnabled() == true);
	QVERIFY(e.stopScene() == Function::invalidId());
	QVERIFY(e.stopSceneEnabled() == false);
}

void EFX_Test::copyFrom()
{
	EFX e1(this);
	e1.setName("First");
	e1.setDirection(EFX::Backward);
	e1.setRunOrder(EFX::SingleShot);
	e1.setBus(15);
	e1.setAlgorithm("Lissajous");
	e1.setWidth(13);
	e1.setHeight(42);
	e1.setRotation(78);
	e1.setXOffset(34);
	e1.setYOffset(27);
	e1.setXFrequency(5);
	e1.setYFrequency(4);
	e1.setXPhase(163);
	e1.setYPhase(94);
	e1.setPropagationMode(EFX::Serial);
	e1.setStartScene(99);
	e1.setStopScene(88);

	/* Verify that EFX contents are copied */
	EFX e2(this);
	QVERIFY(e2.copyFrom(&e1) == true);
	QVERIFY(e2.name() == "First");
	QVERIFY(e2.direction() == EFX::Backward);
	QVERIFY(e2.runOrder() == EFX::SingleShot);
	QVERIFY(e2.busID() == 15);
	QVERIFY(e2.algorithm() == "Lissajous");
	QVERIFY(e2.width() == 13);
	QVERIFY(e2.height() == 42);
	QVERIFY(e2.rotation() == 78);
	QVERIFY(e2.xOffset() == 34);
	QVERIFY(e2.yOffset() == 27);
	QVERIFY(e2.xFrequency() == 5);
	QVERIFY(e2.yFrequency() == 4);
	QVERIFY(e2.xPhase() == 163);
	QVERIFY(e2.yPhase() == 94);
	QVERIFY(e2.propagationMode() == EFX::Serial);
	QVERIFY(e2.startScene() == 99);
	QVERIFY(e2.stopScene() == 88);

	/* Verify that an EFX gets a copy only from another EFX */
	Scene s(this);
	QVERIFY(e2.copyFrom(&s) == false);

	/* Make a third Collection */
	EFX e3(this);
	e3.setName("Third");

	/* Verify that copying to the same EFX a second time succeeds */
	QVERIFY(e2.copyFrom(&e3) == true);
	QVERIFY(e2.name() == "Third");
}

void EFX_Test::createCopy()
{
	Doc doc(this, m_cache);

	EFX* e1 = new EFX(this);
	e1->setName("First");

	doc.addFunction(e1);
	QVERIFY(e1->id() != KNoID);

	Function* f = e1->createCopy(&doc);
	QVERIFY(f != NULL);
	QVERIFY(f != e1);
	QVERIFY(f->id() != e1->id());

	EFX* copy = qobject_cast<EFX*> (f);
	QVERIFY(copy != NULL);
}

#if 0
void Collection_Test::functions()
{
	Collection c(this);
	c.setID(50);
	QVERIFY(c.functions().size() == 0);

	/* A collection should not be allowed to be its own member */
	QVERIFY(c.addFunction(50) == false);
	QVERIFY(c.functions().size() == 0);

	/* Add a function with id "12" to the Collection */
	QVERIFY(c.addFunction(12) == true);
	QVERIFY(c.functions().size() == 1);
	QVERIFY(c.functions().at(0) == 12);

	/* Add another function in the middle */
	QVERIFY(c.addFunction(34) == true);
	QVERIFY(c.functions().size() == 2);
	QVERIFY(c.functions().at(0) == 12);
	QVERIFY(c.functions().at(1) == 34);

	/* Must not be able to add the same function multiple times */
	QVERIFY(c.addFunction(12) == false);
	QVERIFY(c.functions().size() == 2);
	QVERIFY(c.functions().at(0) == 12);
	QVERIFY(c.functions().at(1) == 34);

	/* Removing a non-existent function should make no modifications */
	QVERIFY(c.removeFunction(999) == false);
	QVERIFY(c.functions().size() == 2);
	QVERIFY(c.functions().at(0) == 12);
	QVERIFY(c.functions().at(1) == 34);

	/* Removing the last step should succeed */
	QVERIFY(c.removeFunction(34) == true);
	QVERIFY(c.functions().size() == 1);
	QVERIFY(c.functions().at(0) == 12);

	/* Removing the only step should succeed */
	QVERIFY(c.removeFunction(12) == true);
	QVERIFY(c.functions().size() == 0);
}

void Collection_Test::functionRemoval()
{
	Collection c(this);
	c.setID(42);
	QVERIFY(c.functions().size() == 0);

	QVERIFY(c.addFunction(0) == true);
	QVERIFY(c.addFunction(1) == true);
	QVERIFY(c.addFunction(2) == true);
	QVERIFY(c.addFunction(3) == true);
	QVERIFY(c.functions().size() == 4);

	/* Simulate function removal signal with an uninteresting function id */
	c.slotFunctionRemoved(6);
	QVERIFY(c.functions().size() == 4);

	/* Simulate function removal signal with a function in the Collection */
	c.slotFunctionRemoved(1);
	QVERIFY(c.functions().size() == 3);
	QVERIFY(c.functions().at(0) == 0);
	QVERIFY(c.functions().at(1) == 2);
	QVERIFY(c.functions().at(2) == 3);

	/* Simulate function removal signal with an invalid function id */
	c.slotFunctionRemoved(Function::invalidId());
	QVERIFY(c.functions().size() == 3);
	QVERIFY(c.functions().at(0) == 0);
	QVERIFY(c.functions().at(1) == 2);
	QVERIFY(c.functions().at(2) == 3);

	/* Simulate function removal signal with a function in the Collection */
	c.slotFunctionRemoved(0);
	QVERIFY(c.functions().size() == 2);
	QVERIFY(c.functions().at(0) == 2);
	QVERIFY(c.functions().at(1) == 3);
}

void Collection_Test::loadSuccess()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "Collection");

	QDomElement s1 = doc.createElement("Step");
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Collection c(this);
	QVERIFY(c.loadXML(&root) == true);
	QVERIFY(c.functions().size() == 3);
	QVERIFY(c.functions().contains(50) == true);
	QVERIFY(c.functions().contains(12) == true);
	QVERIFY(c.functions().contains(87) == true);
}

void Collection_Test::loadWrongType()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "Chaser");

	QDomElement s1 = doc.createElement("Step");
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Collection c(this);
	QVERIFY(c.loadXML(&root) == false);
}

void Collection_Test::loadWrongRoot()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Collection");
	root.setAttribute("Type", "Collection");

	QDomElement s1 = doc.createElement("Step");
	QDomText s1Text = doc.createTextNode("50");
	s1.appendChild(s1Text);
	root.appendChild(s1);

	QDomElement s2 = doc.createElement("Step");
	QDomText s2Text = doc.createTextNode("12");
	s2.appendChild(s2Text);
	root.appendChild(s2);

	QDomElement s3 = doc.createElement("Step");
	QDomText s3Text = doc.createTextNode("87");
	s3.appendChild(s3Text);
	root.appendChild(s3);

	Collection c(this);
	QVERIFY(c.loadXML(&root) == false);
}

void Collection_Test::save()
{
	Collection c(this);
	c.addFunction(3);
	c.addFunction(1);
	c.addFunction(0);
	c.addFunction(2);

	QDomDocument doc;
	QDomElement root = doc.createElement("TestRoot");

	QVERIFY(c.saveXML(&doc, &root) == true);
	QVERIFY(root.firstChild().toElement().tagName() == "Function");
	QVERIFY(root.firstChild().toElement().attribute("Type") == "Collection");

	QDomNode node = root.firstChild().firstChild();
	int fids = 0;
	while (node.isNull() == false)
	{
		QDomElement tag = node.toElement();
		if (tag.tagName() == "Step")
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

	QVERIFY(fids == 4);
}

void Collection_Test::armSuccess()
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

	Collection* c = new Collection(doc);
	c->setName("Collection");
	c->addFunction(s1->id());
	c->addFunction(s2->id());

	QVERIFY(c->functions().size() == 2);
	c->arm();
	QVERIFY(c->functions().size() == 2);

	delete doc;
}

void Collection_Test::armMissingFunction()
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

	Collection* c = new Collection(doc);
	c->setName("Collection");
	c->addFunction(s1->id());
	c->addFunction(123); // Nonexistent function
	c->addFunction(s2->id());
	c->addFunction(55); // Nonexistent function

	QVERIFY(c->functions().size() == 4);
	c->arm();
	QVERIFY(c->functions().size() == 2); // Nonexistent functions are removed

	delete doc;
}

void Collection_Test::write()
{
	Doc* doc = new Doc(this, m_cache);

	Bus::instance()->setValue(Bus::defaultFade(), 0);
	Bus::instance()->setValue(Bus::defaultHold(), 0);

	Fixture* fxi = new Fixture(doc);
	fxi->setAddress(0);
	fxi->setUniverse(0);
	fxi->setChannels(4);
	doc->addFixture(fxi);

	Scene* s1 = new Scene(doc);
	s1->setName("Scene1");
	s1->setValue(fxi->id(), 0, 255);
	s1->setValue(fxi->id(), 1, 255);
	doc->addFunction(s1);

	Scene* s2 = new Scene(doc);
	s2->setName("Scene2");
	s2->setValue(fxi->id(), 2, 255);
	s2->setValue(fxi->id(), 3, 255);
	doc->addFunction(s2);

	Collection* c = new Collection(doc);
	c->setName("Collection");
	c->addFunction(s1->id());
	c->addFunction(s2->id());

	s1->arm();
	s2->arm();
	c->arm();

	MasterTimerStub* mts = new MasterTimerStub(this);

	/* Collection starts all of its members immediately when it is started
	   itself. */
	c->start(mts);
	QVERIFY(mts->m_list.size() == 3);
	QVERIFY(mts->m_list[0] == s1);
	QVERIFY(mts->m_list[1] == s2);
	QVERIFY(mts->m_list[2] == c);

	QByteArray uni;

	/* All write calls to the collection "succeed" as long as there are
	   members running. */
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 3);
	QVERIFY(mts->m_list[0] == s1);
	QVERIFY(mts->m_list[1] == s2);
	QVERIFY(mts->m_list[2] == c);

	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 3);
	QVERIFY(mts->m_list[0] == s1);
	QVERIFY(mts->m_list[1] == s2);
	QVERIFY(mts->m_list[2] == c);

	/* S2 is still running after this so the collection is also running */
	s1->stop(mts);
	QVERIFY(c->write(&uni) == true);
	QVERIFY(mts->m_list.size() == 2);
	QVERIFY(mts->m_list[0] == s2);
	QVERIFY(mts->m_list[1] == c);

	/* Now also the collection tells that it's ready by returning false */
	s2->stop(mts);
	QVERIFY(c->write(&uni) == false);

	c->stop(mts);
	QVERIFY(mts->m_list.size() == 0);

	c->disarm();
	s1->disarm();
	s2->disarm();

	delete mts;
	delete doc;
}
#endif
