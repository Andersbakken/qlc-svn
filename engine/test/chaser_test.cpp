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

#include "universearray.h"
#include "function.h"
#include "fixture.h"
#define protected public
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#undef protected

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void Chaser_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void Chaser_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void Chaser_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void Chaser_Test::initial()
{
    Chaser c(m_doc);
    QVERIFY(c.type() == Function::Chaser);
    QVERIFY(c.name() == "New Chaser");
    QVERIFY(c.steps().size() == 0);
    QVERIFY(c.direction() == Chaser::Forward);
    QVERIFY(c.runOrder() == Chaser::Loop);
    QVERIFY(c.id() == Function::invalidId());
}

void Chaser_Test::directionRunOrder()
{
    Chaser c(m_doc);

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
    Chaser c(m_doc);
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
    Chaser c(m_doc);
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

    Chaser c(m_doc);
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

    Chaser c(m_doc);
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

    Chaser c(m_doc);
    QVERIFY(c.loadXML(&root) == false);
}

void Chaser_Test::save()
{
    Chaser c(m_doc);
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
    Chaser c1(m_doc);
    c1.setName("First");
    c1.setDirection(Chaser::Backward);
    c1.setRunOrder(Chaser::PingPong);
    c1.setBus(15);
    c1.addStep(2);
    c1.addStep(0);
    c1.addStep(1);
    c1.addStep(25);

    /* Verify that chaser contents are copied */
    Chaser c2(m_doc);
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
    Scene s(m_doc);
    QVERIFY(c2.copyFrom(&s) == false);

    /* Make a third Chaser */
    Chaser c3(m_doc);
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

    Chaser* c1 = new Chaser(m_doc);
    c1->setName("First");
    c1->setBus(15);
    c1->setDirection(Chaser::Backward);
    c1->setRunOrder(Chaser::SingleShot);
    c1->addStep(20);
    c1->addStep(30);
    c1->addStep(40);

    doc.addFunction(c1);
    QVERIFY(c1->id() != Function::invalidId());

    // Verify that the function is not created when Doc is full
    doc.m_functionAllocation = KFunctionArraySize;
    Function* f = c1->createCopy(&doc);
    QVERIFY(f == NULL);

    doc.m_functionAllocation = 1;
    f = c1->createCopy(&doc);
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
