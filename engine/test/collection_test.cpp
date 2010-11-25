/*
  Q Light Controller - Unit test
  collection_test.cpp

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
#include "collection_test.h"

#include "universearray.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void Collection_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void Collection_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void Collection_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void Collection_Test::initial()
{
    Collection c(m_doc);
    QVERIFY(c.type() == Function::Collection);
    QVERIFY(c.name() == "New Collection");
    QVERIFY(c.functions().size() == 0);
    QVERIFY(c.id() == Function::invalidId());
}

void Collection_Test::functions()
{
    Collection c(m_doc);
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
    Collection c(m_doc);
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

    Collection c(m_doc);
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

    Collection c(m_doc);
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

    Collection c(m_doc);
    QVERIFY(c.loadXML(&root) == false);
}

void Collection_Test::loadWrongMemberTag()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Collection");

    QDomElement s1 = doc.createElement("Foo");
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Bar");
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    Collection c(m_doc);
    QVERIFY(c.loadXML(&root) == true);
    QCOMPARE(c.functions().size(), 1);
}

void Collection_Test::save()
{
    Collection c(m_doc);
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

void Collection_Test::copyFrom()
{
    Collection c1(m_doc);
    c1.setName("First");
    c1.addFunction(2);
    c1.addFunction(0);
    c1.addFunction(1);
    c1.addFunction(25);

    /* Verify that Collection contents are copied */
    Collection c2(m_doc);
    QVERIFY(c2.copyFrom(&c1) == true);
    QVERIFY(c2.name() == "First");
    QVERIFY(c2.functions().size() == 4);
    QVERIFY(c2.functions().at(0) == 2);
    QVERIFY(c2.functions().at(1) == 0);
    QVERIFY(c2.functions().at(2) == 1);
    QVERIFY(c2.functions().at(3) == 25);

    /* Verify that a Collection gets a copy only from another Collection */
    Scene s(m_doc);
    QVERIFY(c2.copyFrom(&s) == false);

    /* Make a third Collection */
    Collection c3(m_doc);
    c3.setName("Third");
    c3.addFunction(15);
    c3.addFunction(94);
    c3.addFunction(3);

    /* Verify that copying TO the same Collection a second time succeeds and
       that steps are not appended but replaced completely. */
    QVERIFY(c2.copyFrom(&c3) == true);
    QVERIFY(c2.name() == "Third");
    QVERIFY(c2.functions().size() == 3);
    QVERIFY(c2.functions().at(0) == 15);
    QVERIFY(c2.functions().at(1) == 94);
    QVERIFY(c2.functions().at(2) == 3);
}

void Collection_Test::createCopy()
{
    Doc doc(this, m_cache);

    Collection* c1 = new Collection(m_doc);
    c1->setName("First");
    c1->addFunction(20);
    c1->addFunction(30);
    c1->addFunction(40);

    doc.addFunction(c1);
    QVERIFY(c1->id() != Function::invalidId());

    Function* f = c1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != c1);
    QVERIFY(f->id() != c1->id());

    Collection* copy = qobject_cast<Collection*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->functions().size() == 3);
    QVERIFY(copy->functions().at(0) == 20);
    QVERIFY(copy->functions().at(1) == 30);
    QVERIFY(copy->functions().at(2) == 40);
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
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, UCHAR_MAX);
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
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, UCHAR_MAX);
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
    s1->setValue(fxi->id(), 0, UCHAR_MAX);
    s1->setValue(fxi->id(), 1, UCHAR_MAX);
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("Scene2");
    s2->setValue(fxi->id(), 2, UCHAR_MAX);
    s2->setValue(fxi->id(), 3, UCHAR_MAX);
    doc->addFunction(s2);

    Collection* c = new Collection(doc);
    c->setName("Collection");
    c->addFunction(s1->id());
    c->addFunction(s2->id());

    s1->arm();
    s2->arm();
    c->arm();

    UniverseArray uni(1);
    MasterTimerStub* mts = new MasterTimerStub(this, NULL, uni);

    /* Collection starts all of its members immediately when it is started
       itself. */
    QVERIFY(c->stopped() == true);
    mts->startFunction(c, false);
    QVERIFY(c->stopped() == false);

    c->write(mts, &uni);
    QVERIFY(c->stopped() == false);
    QVERIFY(mts->m_functionList.size() == 3);
    QVERIFY(mts->m_functionList[0] == c);
    QVERIFY(mts->m_functionList[1] == s1);
    QVERIFY(mts->m_functionList[2] == s2);

    /* All write calls to the collection "succeed" as long as there are
       members running. */
    c->write(mts, &uni);
    QVERIFY(c->stopped() == false);
    QVERIFY(mts->m_functionList.size() == 3);
    QVERIFY(mts->m_functionList[0] == c);
    QVERIFY(mts->m_functionList[1] == s1);
    QVERIFY(mts->m_functionList[2] == s2);

    c->write(mts, &uni);
    QVERIFY(c->stopped() == false);
    QVERIFY(mts->m_functionList.size() == 3);
    QVERIFY(mts->m_functionList[0] == c);
    QVERIFY(mts->m_functionList[1] == s1);
    QVERIFY(mts->m_functionList[2] == s2);

    /* S2 is still running after this so the collection is also running */
    mts->stopFunction(s1);
    QVERIFY(s1->stopped() == true);

    c->write(mts, &uni);
    QVERIFY(c->stopped() == false);
    QVERIFY(mts->m_functionList.size() == 2);
    QVERIFY(mts->m_functionList[0] == c);
    QVERIFY(mts->m_functionList[1] == s2);

    /* Now the collection must also tell it's ready to be stopped */
    mts->stopFunction(s2);
    QVERIFY(s2->stopped() == true);
    QVERIFY(c->stopped() == true);
    mts->stopFunction(c);

    c->disarm();
    s1->disarm();
    s2->disarm();

    delete mts;
    delete doc;
}
