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
#include <QList>

#include "mastertimer_stub.h"
#include "efx_test.h"

#include "../function.h"
#include "../fixture.h"
#include "../scene.h"
#include "../doc.h"

/* Expose protected members to the unit test */
#define protected public
#include "../efx.h"
#undef protected

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
	QVERIFY(e.pointFunc == NULL);

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

	e.setWidth(0);
	QVERIFY(e.width() == 0);

	e.setWidth(128);
	QVERIFY(e.width() == 127);

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

	e.setHeight(0);
	QVERIFY(e.height() == 0);

	e.setHeight(128);
	QVERIFY(e.height() == 127);

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
	QVERIFY(e.xOffset() == 255);

	e.setXOffset(0);
	QVERIFY(e.xOffset() == 0);

	e.setXOffset(256);
	QVERIFY(e.xOffset() == 255);

	e.setXOffset(12);
	QVERIFY(e.xOffset() == 12);

	e.setXOffset(-4);
	QVERIFY(e.xOffset() == 0);
}

void EFX_Test::yOffset()
{
	EFX e(this);

	e.setYOffset(300);
	QVERIFY(e.yOffset() == 255);

	e.setYOffset(0);
	QVERIFY(e.yOffset() == 0);

	e.setYOffset(256);
	QVERIFY(e.yOffset() == 255);

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
	EFXFixture* ef1 = new EFXFixture(&e1);
	ef1->setFixture(12);
	e1.addFixture(ef1);
	EFXFixture* ef2 = new EFXFixture(&e1);
	ef2->setFixture(34);
	e1.addFixture(ef2);

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
	QVERIFY(e2.fixtures().size() == 2);
	QVERIFY(e2.fixtures().at(0)->fixture() == 12);
	QVERIFY(e2.fixtures().at(1)->fixture() == 34);
	QVERIFY(e2.fixtures().at(0) != ef1);
	QVERIFY(e2.fixtures().at(1) != ef2);
	
	/* Verify that an EFX gets a copy only from another EFX */
	Scene s(this);
	QVERIFY(e2.copyFrom(&s) == false);

	/* Make a third EFX */
	EFX e3(this);
	e3.setName("Third");
	e3.setDirection(EFX::Forward);
	e3.setRunOrder(EFX::Loop);
	e3.setBus(3);
	e3.setAlgorithm("Eight");
	e3.setWidth(31);
	e3.setHeight(24);
	e3.setRotation(87);
	e3.setXOffset(43);
	e3.setYOffset(72);
	e3.setXFrequency(3);
	e3.setYFrequency(2);
	e3.setXPhase(136);
	e3.setYPhase(49);
	e3.setPropagationMode(EFX::Parallel);
	e3.setStartScene(77);
	e3.setStopScene(66);
	EFXFixture* ef3 = new EFXFixture(&e3);
	ef3->setFixture(56);
	e3.addFixture(ef3);
	EFXFixture* ef4 = new EFXFixture(&e3);
	ef4->setFixture(78);
	e3.addFixture(ef4);

	/* Verify that copying to the same EFX a second time succeeds */
	QVERIFY(e2.copyFrom(&e3) == true);
	QVERIFY(e2.name() == "Third");
	QVERIFY(e2.direction() == EFX::Forward);
	QVERIFY(e2.runOrder() == EFX::Loop);
	QVERIFY(e2.busID() == 3);
	QVERIFY(e2.algorithm() == "Eight");
	QVERIFY(e2.width() == 31);
	QVERIFY(e2.height() == 24);
	QVERIFY(e2.rotation() == 87);
	QVERIFY(e2.xOffset() == 43);
	QVERIFY(e2.yOffset() == 72);
	QVERIFY(e2.xFrequency() == 3);
	QVERIFY(e2.yFrequency() == 2);
	QVERIFY(e2.xPhase() == 136);
	QVERIFY(e2.yPhase() == 49);
	QVERIFY(e2.propagationMode() == EFX::Parallel);
	QVERIFY(e2.startScene() == 77);
	QVERIFY(e2.stopScene() == 66);
	QVERIFY(e2.fixtures().size() == 2);
	QVERIFY(e2.fixtures().at(0)->fixture() == 56);
	QVERIFY(e2.fixtures().at(1)->fixture() == 78);
	QVERIFY(e2.fixtures().at(0) != ef1);
	QVERIFY(e2.fixtures().at(0) != ef3);
	QVERIFY(e2.fixtures().at(1) != ef2);
	QVERIFY(e2.fixtures().at(1) != ef4);
}

void EFX_Test::createCopy()
{
	Doc doc(this, m_cache);

	EFX* e1 = new EFX(this);
	e1->setName("First");
	e1->setDirection(EFX::Forward);
	e1->setRunOrder(EFX::PingPong);
	e1->setBus(15);
	e1->setAlgorithm("Line");
	e1->setWidth(13);
	e1->setHeight(42);
	e1->setRotation(78);
	e1->setXOffset(34);
	e1->setYOffset(27);
	e1->setXFrequency(5);
	e1->setYFrequency(4);
	e1->setXPhase(163);
	e1->setYPhase(94);
	e1->setPropagationMode(EFX::Serial);
	e1->setStartScene(99);
	e1->setStopScene(88);
	EFXFixture* ef1 = new EFXFixture(e1);
	ef1->setFixture(12);
	e1->addFixture(ef1);
	EFXFixture* ef2 = new EFXFixture(e1);
	ef2->setFixture(34);
	e1->addFixture(ef2);

	doc.addFunction(e1);
	QVERIFY(e1->id() != KNoID);

	Function* f = e1->createCopy(&doc);
	QVERIFY(f != NULL);
	QVERIFY(f != e1);
	QVERIFY(f->id() != e1->id());

	EFX* copy = qobject_cast<EFX*> (f);
	QVERIFY(copy != NULL);
	QVERIFY(copy->name() == "Copy of First");
	QVERIFY(copy->direction() == EFX::Forward);
	QVERIFY(copy->runOrder() == EFX::PingPong);
	QVERIFY(copy->busID() == 15);
	QVERIFY(copy->algorithm() == "Line");
	QVERIFY(copy->width() == 13);
	QVERIFY(copy->height() == 42);
	QVERIFY(copy->rotation() == 78);
	QVERIFY(copy->xOffset() == 34);
	QVERIFY(copy->yOffset() == 27);
	QVERIFY(copy->xFrequency() == 5);
	QVERIFY(copy->yFrequency() == 4);
	QVERIFY(copy->xPhase() == 163);
	QVERIFY(copy->yPhase() == 94);
	QVERIFY(copy->propagationMode() == EFX::Serial);
	QVERIFY(copy->startScene() == 99);
	QVERIFY(copy->stopScene() == 88);
	QVERIFY(copy->fixtures().size() == 2);
	QVERIFY(copy->fixtures().at(0)->fixture() == 12);
	QVERIFY(copy->fixtures().at(1)->fixture() == 34);
	QVERIFY(copy->fixtures().at(0) != ef1);
	QVERIFY(copy->fixtures().at(1) != ef2);
}

void EFX_Test::loadXAxis()
{
	QDomDocument doc;

	QDomElement ax = doc.createElement("Axis");
	ax.setAttribute("Name", "X");

	QDomElement off = doc.createElement("Offset");
	QDomText offText = doc.createTextNode("1");
	off.appendChild(offText);
	ax.appendChild(off);

	QDomElement freq = doc.createElement("Frequency");
	QDomText freqText = doc.createTextNode("2");
	freq.appendChild(freqText);
	ax.appendChild(freq);

	QDomElement pha = doc.createElement("Phase");
	QDomText phaText = doc.createTextNode("3");
	pha.appendChild(phaText);
	ax.appendChild(pha);

	EFX e(this);
	QVERIFY(e.loadXMLAxis(&ax) == true);
}

void EFX_Test::loadYAxis()
{
	QDomDocument doc;

	QDomElement ax = doc.createElement("Axis");
	ax.setAttribute("Name", "Y");

	QDomElement off = doc.createElement("Offset");
	QDomText offText = doc.createTextNode("1");
	off.appendChild(offText);
	ax.appendChild(off);

	QDomElement freq = doc.createElement("Frequency");
	QDomText freqText = doc.createTextNode("2");
	freq.appendChild(freqText);
	ax.appendChild(freq);

	QDomElement pha = doc.createElement("Phase");
	QDomText phaText = doc.createTextNode("3");
	pha.appendChild(phaText);
	ax.appendChild(pha);

	EFX e(this);
	QVERIFY(e.loadXMLAxis(&ax) == true);
}

void EFX_Test::loadYAxisWrongRoot()
{
	QDomDocument doc;

	QDomElement ax = doc.createElement("sixA");
	ax.setAttribute("Name", "Y");

	QDomElement off = doc.createElement("Offset");
	QDomText offText = doc.createTextNode("1");
	off.appendChild(offText);
	ax.appendChild(off);

	QDomElement freq = doc.createElement("Frequency");
	QDomText freqText = doc.createTextNode("2");
	freq.appendChild(freqText);
	ax.appendChild(freq);

	QDomElement pha = doc.createElement("Phase");
	QDomText phaText = doc.createTextNode("3");
	pha.appendChild(phaText);
	ax.appendChild(pha);

	EFX e(this);
	QVERIFY(e.loadXMLAxis(&ax) == false);
}

void EFX_Test::loadAxisNoXY()
{
	QDomDocument doc;

	QDomElement ax = doc.createElement("Axis");
	ax.setAttribute("Name", "Not X nor Y");

	QDomElement off = doc.createElement("Offset");
	QDomText offText = doc.createTextNode("1");
	off.appendChild(offText);
	ax.appendChild(off);

	QDomElement freq = doc.createElement("Frequency");
	QDomText freqText = doc.createTextNode("5");
	freq.appendChild(freqText);
	ax.appendChild(freq);

	QDomElement pha = doc.createElement("Phase");
	QDomText phaText = doc.createTextNode("333");
	pha.appendChild(phaText);
	ax.appendChild(pha);

	EFX e(this);
	QVERIFY(e.loadXMLAxis(&ax) == false);
	QVERIFY(e.xOffset() != 1);
	QVERIFY(e.xFrequency() != 5);
	QVERIFY(e.xPhase() != 333);
}

void EFX_Test::loadSuccess()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "EFX");
	root.setAttribute("Name", "Test EFX");

	QDomElement prop = doc.createElement("PropagationMode");
	QDomText propText = doc.createTextNode("Serial");
	prop.appendChild(propText);
	root.appendChild(prop);

	QDomElement bus = doc.createElement("Bus");
	bus.setAttribute("Role", "Fade");
	QDomText busText = doc.createTextNode("12");
	bus.appendChild(busText);
	root.appendChild(bus);

	QDomElement dir = doc.createElement("Direction");
	QDomText dirText = doc.createTextNode("Forward");
	dir.appendChild(dirText);
	root.appendChild(dir);

	QDomElement run = doc.createElement("RunOrder");
	QDomText runText = doc.createTextNode("Loop");
	run.appendChild(runText);
	root.appendChild(run);

	QDomElement algo = doc.createElement("Algorithm");
	QDomText algoText = doc.createTextNode("Diamond");
	algo.appendChild(algoText);
	root.appendChild(algo);

	QDomElement w = doc.createElement("Width");
	QDomText wText = doc.createTextNode("100");
	w.appendChild(wText);
	root.appendChild(w);

	QDomElement h = doc.createElement("Height");
	QDomText hText = doc.createTextNode("90");
	h.appendChild(hText);
	root.appendChild(h);

	QDomElement rot = doc.createElement("Rotation");
	QDomText rotText = doc.createTextNode("310");
	rot.appendChild(rotText);
	root.appendChild(rot);

	QDomElement stas = doc.createElement("StartScene");
	stas.setAttribute("Enabled", "True");
	QDomText stasText = doc.createTextNode("13");
	stas.appendChild(stasText);
	root.appendChild(stas);

	QDomElement stos = doc.createElement("StopScene");
	stos.setAttribute("Enabled", "True");
	QDomText stosText = doc.createTextNode("77");
	stos.appendChild(stosText);
	root.appendChild(stos);

	/* X Axis */
	QDomElement xax = doc.createElement("Axis");
	xax.setAttribute("Name", "X");
	root.appendChild(xax);

	QDomElement xoff = doc.createElement("Offset");
	QDomText xoffText = doc.createTextNode("10");
	xoff.appendChild(xoffText);
	xax.appendChild(xoff);

	QDomElement xfreq = doc.createElement("Frequency");
	QDomText xfreqText = doc.createTextNode("2");
	xfreq.appendChild(xfreqText);
	xax.appendChild(xfreq);

	QDomElement xpha = doc.createElement("Phase");
	QDomText xphaText = doc.createTextNode("270");
	xpha.appendChild(xphaText);
	xax.appendChild(xpha);

	/* Y Axis */
	QDomElement yax = doc.createElement("Axis");
	yax.setAttribute("Name", "Y");
	root.appendChild(yax);

	QDomElement yoff = doc.createElement("Offset");
	QDomText yoffText = doc.createTextNode("20");
	yoff.appendChild(yoffText);
	yax.appendChild(yoff);

	QDomElement yfreq = doc.createElement("Frequency");
	QDomText yfreqText = doc.createTextNode("3");
	yfreq.appendChild(yfreqText);
	yax.appendChild(yfreq);

	QDomElement ypha = doc.createElement("Phase");
	QDomText yphaText = doc.createTextNode("80");
	ypha.appendChild(yphaText);
	yax.appendChild(ypha);

	/* Fixture 1 */
	QDomElement ef1 = doc.createElement("Fixture");
	root.appendChild(ef1);

	QDomElement ef1ID = doc.createElement("ID");
	QDomText ef1IDText = doc.createTextNode("33");
	ef1ID.appendChild(ef1IDText);
	ef1.appendChild(ef1ID);

	QDomElement ef1dir = doc.createElement("Direction");
	QDomText ef1dirText = doc.createTextNode("Forward");
	ef1dir.appendChild(ef1dirText);
	ef1.appendChild(ef1dir);

	/* Fixture 2 */
	QDomElement ef2 = doc.createElement("Fixture");
	root.appendChild(ef2);

	QDomElement ef2ID = doc.createElement("ID");
	QDomText ef2IDText = doc.createTextNode("11");
	ef2ID.appendChild(ef2IDText);
	ef2.appendChild(ef2ID);

	QDomElement ef2dir = doc.createElement("Direction");
	QDomText ef2dirText = doc.createTextNode("Backward");
	ef2dir.appendChild(ef2dirText);
	ef2.appendChild(ef2dir);

	/* Fixture 3 */
	QDomElement ef3 = doc.createElement("Fixture");
	root.appendChild(ef3);

	QDomElement ef3ID = doc.createElement("ID");
	QDomText ef3IDText = doc.createTextNode("45");
	ef3ID.appendChild(ef3IDText);
	ef3.appendChild(ef3ID);

	QDomElement ef3dir = doc.createElement("Direction");
	QDomText ef3dirText = doc.createTextNode("Backward");
	ef3dir.appendChild(ef3dirText);
	ef3.appendChild(ef3dir);

	EFX e(this);
	QVERIFY(e.loadXML(&root) == true);
	QVERIFY(e.busID() == 12);
	QVERIFY(e.direction() == EFX::Forward);
	QVERIFY(e.runOrder() == EFX::Loop);

	QVERIFY(e.algorithm() == "Diamond");
	QVERIFY(e.width() == 100);
	QVERIFY(e.height() == 90);
	QVERIFY(e.rotation() == 310);

	QVERIFY(e.xOffset() == 10);
	QVERIFY(e.xFrequency() == 2);
	QVERIFY(e.xPhase() == 270);
	QVERIFY(e.yOffset() == 20);
	QVERIFY(e.yFrequency() == 3);
	QVERIFY(e.yPhase() == 80);

	QVERIFY(e.startScene() == 13);
	QVERIFY(e.stopScene() == 77);

	QVERIFY(e.propagationMode() == EFX::Serial);
	QVERIFY(e.fixtures().size() == 3);
	QVERIFY(e.fixtures().at(0)->fixture() == 33);
	QVERIFY(e.fixtures().at(0)->direction() == EFX::Forward);
	QVERIFY(e.fixtures().at(1)->fixture() == 11);
	QVERIFY(e.fixtures().at(1)->direction() == EFX::Backward);
	QVERIFY(e.fixtures().at(2)->fixture() == 45);
	QVERIFY(e.fixtures().at(2)->direction() == EFX::Backward);
}

void EFX_Test::loadWrongType()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "Chaser");

	EFX e(this);
	QVERIFY(e.loadXML(&root) == false);
}

void EFX_Test::loadWrongRoot()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("EFX");
	root.setAttribute("Type", "EFX");

	EFX e(this);
	QVERIFY(e.loadXML(&root) == false);
}

void EFX_Test::loadDuplicateFixture()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Function");
	root.setAttribute("Type", "EFX");

	/* Fixture 1 */
	QDomElement ef1 = doc.createElement("Fixture");
	root.appendChild(ef1);

	QDomElement ef1ID = doc.createElement("ID");
	QDomText ef1IDText = doc.createTextNode("33");
	ef1ID.appendChild(ef1IDText);
	ef1.appendChild(ef1ID);

	QDomElement ef1dir = doc.createElement("Direction");
	QDomText ef1dirText = doc.createTextNode("Forward");
	ef1dir.appendChild(ef1dirText);
	ef1.appendChild(ef1dir);

	/* Fixture 2 */
	QDomElement ef2 = doc.createElement("Fixture");
	root.appendChild(ef2);

	QDomElement ef2ID = doc.createElement("ID");
	QDomText ef2IDText = doc.createTextNode("33");
	ef2ID.appendChild(ef2IDText);
	ef2.appendChild(ef2ID);

	QDomElement ef2dir = doc.createElement("Direction");
	QDomText ef2dirText = doc.createTextNode("Backward");
	ef2dir.appendChild(ef2dirText);
	ef2.appendChild(ef2dir);

	EFX e(this);
	QVERIFY(e.loadXML(&root) == true);
	QVERIFY(e.fixtures().size() == 1);
	QVERIFY(e.fixtures().at(0)->direction() == EFX::Forward);
}

void EFX_Test::save()
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
	e1.setStartSceneEnabled(true);
	e1.setStopScene(88);
	//e1.setStartSceneEnabled(false);

	EFXFixture* ef1 = new EFXFixture(&e1);
	ef1->setFixture(12);
	e1.addFixture(ef1);
	EFXFixture* ef2 = new EFXFixture(&e1);
	ef2->setFixture(34);
	ef2->setDirection(EFX::Backward);
	e1.addFixture(ef2);
	EFXFixture* ef3 = new EFXFixture(&e1);
	ef3->setFixture(56);
	e1.addFixture(ef3);

	QDomDocument doc;
	QDomElement root = doc.createElement("TestRoot");

	QVERIFY(e1.saveXML(&doc, &root) == true);
	QVERIFY(root.firstChild().toElement().tagName() == "Function");
	QVERIFY(root.firstChild().toElement().attribute("Type") == "EFX");
	QVERIFY(root.firstChild().toElement().attribute("Name") == "First");

	QDomNode node = root.firstChild().firstChild();
	bool dir = false, run = false, bus = false, algo = false, w = false,
		h = false, rot = false, xoff = false, yoff = false,
		xfreq = false, yfreq = false, xpha = false, ypha = false,
		prop = false, stas = false, stos = false;
	QList <QString> fixtures;
	while (node.isNull() == false)
	{
		QDomElement tag = node.toElement();
		if (tag.tagName() == "Direction")
		{
			QVERIFY(tag.text() == "Backward");
			dir = true;
		}
		else if (tag.tagName() == "RunOrder")
		{
			QVERIFY(tag.text() == "SingleShot");
			run = true;
		}
		else if (tag.tagName() == "Bus")
		{
			QVERIFY(tag.text() == "15");
			bus = true;
		}
		else if (tag.tagName() == "Algorithm")
		{
			QVERIFY(tag.text() == "Lissajous");
			algo = true;
		}
		else if (tag.tagName() == "Width")
		{
			QVERIFY(tag.text() == "13");
			w = true;
		}
		else if (tag.tagName() == "Height")
		{
			QVERIFY(tag.text() == "42");
			h = true;
		}
		else if (tag.tagName() == "Rotation")
		{
			QVERIFY(tag.text() == "78");
			rot = true;
		}
		else if (tag.tagName() == "PropagationMode")
		{
			QVERIFY(tag.text() == "Serial");
			prop = true;
		}
		else if (tag.tagName() == "StartScene")
		{
			QVERIFY(tag.text() == "99");
			QVERIFY(tag.attribute("Enabled") == "True");
			stos = true;
		}
		else if (tag.tagName() == "StopScene")
		{
			QVERIFY(tag.text() == "88");
			QVERIFY(tag.attribute("Enabled") == "False");
			stos = true;
		}
		else if (tag.tagName() == "Axis")
		{
			bool axis = true;
			if (tag.attribute("Name") == "X")
				axis = true;
			else if (tag.attribute("Name") == "Y")
				axis = false;
			else
				QFAIL("Invalid axis!");

			QDomNode subnode = tag.firstChild();
			QDomElement subtag = subnode.toElement();
			if (subtag.tagName() == "Offset")
			{
				if (axis == true)
				{
					QVERIFY(subtag.text() == "34");
					xoff = true;
				}
				else
				{
					QVERIFY(subtag.text() == "27");
					yoff = true;
				}
			}
			else if (subtag.tagName() == "Frequency")
			{
				if (axis == true)
				{
					QVERIFY(subtag.text() == "5");
					xfreq = true;
				}
				else
				{
					QVERIFY(subtag.text() == "4");
					yfreq = true;
				}
			}
			else if (subtag.tagName() == "Phase")
			{
				if (axis == true)
				{
					QVERIFY(subtag.text() == "163");
					xpha = true;
				}
				else
				{
					QVERIFY(subtag.text() == "94");
					ypha = true;
				}
			}
			else
			{
				QFAIL("Unexpected axis tag!");
			}

			subnode = subnode.nextSibling();
		}
		else if (tag.tagName() == "Fixture")
		{
			bool expectBackward = false;

			QDomNode subnode = tag.firstChild();
			QDomElement subtag = subnode.toElement();

			if (subtag.tagName() == "ID")
			{
				if (fixtures.contains(subtag.text()) == true)
					QFAIL("Same fixture multiple times!");
				else
					fixtures.append(subtag.text());

				if (subtag.text() == "34")
					expectBackward = true;
				else
					expectBackward = false;
			}
			else if (subtag.tagName() == "Direction")
			{
				if (expectBackward == false &&
				    subtag.text() == "Backward")
				{
					QFAIL("Not expecting reversal!");
				}
			}
			else
			{
				QFAIL("Unexpected fixture tag!");
			}

			subnode = subnode.nextSibling();
		}
		else
		{
			QFAIL("Unexpected EFX tag!");
		}

		node = node.nextSibling();
	}

	QVERIFY(fixtures.size() == 3);
	QVERIFY(dir = true);
	QVERIFY(run = true);
	QVERIFY(bus = true);
	QVERIFY(algo = true);
	QVERIFY(w = true);
	QVERIFY(h = true);
	QVERIFY(rot = true);
	QVERIFY(xoff = true);
	QVERIFY(yoff = true);
	QVERIFY(xfreq = true);
	QVERIFY(yfreq = true);
	QVERIFY(xpha = true);
	QVERIFY(ypha = true);
	QVERIFY(prop = true);
	QVERIFY(stas = true);
	QVERIFY(stos = true);
}

#if 0
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
