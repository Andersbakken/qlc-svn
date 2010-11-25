/*
  Q Light Controller - Unit test
  doc_test.cpp

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

#include <QPointer>
#include <QtTest>
#include <QtXml>

#include "collection.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "bus.h"
#include "qlcphysical.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

#include "doc_test.h"
#define protected public
#include "doc.h"
#undef protected

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void Doc_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_fixtureDefCache.load(dir) == true);
}

void Doc_Test::defaults()
{
    Doc doc(this, m_fixtureDefCache);
    QVERIFY(&doc.m_fixtureDefCache == &m_fixtureDefCache);
    QVERIFY(doc.m_modified == false);
    QVERIFY(doc.m_fixtureAllocation == 0);
    QVERIFY(doc.m_fixtureArray != NULL);
    QVERIFY(doc.m_functionAllocation == 0);
    QVERIFY(doc.m_functionArray != NULL);
}

void Doc_Test::addFixture()
{
    Doc doc(this, m_fixtureDefCache);
    QVERIFY(doc.isModified() == false);

    /* Add a completely new fixture */
    Fixture* f1 = new Fixture(&doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    QVERIFY(doc.addFixture(f1) == true);
    QVERIFY(doc.m_fixtureAllocation == 1);
    QVERIFY(f1->id() == 0);
    QVERIFY(doc.isModified() == true);

    doc.resetModified();

    /* Add another fixture but attempt to put assign it an already-assigned
       fixture ID. */
    Fixture* f2 = new Fixture(&doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    QVERIFY(doc.addFixture(f2, f1->id()) == false);
    QVERIFY(doc.m_fixtureAllocation == 1);
    QVERIFY(doc.isModified() == false);

    /* But, the fixture can be added if we give it an unassigned ID. */
    QVERIFY(doc.addFixture(f2, f1->id() + 1) == true);
    QVERIFY(doc.m_fixtureAllocation == 2);
    QVERIFY(f1->id() == 0);
    QVERIFY(f2->id() == 1);
    QVERIFY(doc.isModified() == true);

    doc.resetModified();

    /* Add again a completely new fixture, with automatic ID assignment */
    Fixture* f3 = new Fixture(&doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    QVERIFY(doc.addFixture(f3) == true);
    QVERIFY(doc.m_fixtureAllocation == 3);
    QVERIFY(f1->id() == 0);
    QVERIFY(f2->id() == 1);
    QVERIFY(f3->id() == 2);
    QVERIFY(doc.isModified() == true);
}

void Doc_Test::deleteFixture()
{
    Doc doc(this, m_fixtureDefCache);

    QVERIFY(doc.m_fixtureAllocation == 0);
    QVERIFY(doc.deleteFixture(0) == false);
    QVERIFY(doc.m_fixtureAllocation == 0);
    QVERIFY(doc.deleteFixture(1) == false);
    QVERIFY(doc.m_fixtureAllocation == 0);
    QVERIFY(doc.deleteFixture(Fixture::invalidId()) == false);
    QVERIFY(doc.m_fixtureAllocation == 0);

    Fixture* f1 = new Fixture(&doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    doc.addFixture(f1);

    Fixture* f2 = new Fixture(&doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    doc.addFixture(f2);

    Fixture* f3 = new Fixture(&doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    doc.addFixture(f3);

    QVERIFY(doc.isModified() == true);
    doc.resetModified();

    QVERIFY(doc.deleteFixture(42) == false);
    QVERIFY(doc.m_fixtureAllocation == 3);
    QVERIFY(doc.isModified() == false);

    QVERIFY(doc.deleteFixture(Fixture::invalidId()) == false);
    QVERIFY(doc.m_fixtureAllocation == 3);
    QVERIFY(doc.isModified() == false);

    t_fixture_id id = f2->id();
    QVERIFY(doc.deleteFixture(id) == true);
    QVERIFY(doc.m_fixtureAllocation == 2);
    QVERIFY(doc.isModified() == true);

    QVERIFY(doc.deleteFixture(id) == false);
    QVERIFY(doc.m_fixtureAllocation == 2);
    QVERIFY(doc.isModified() == true);

    doc.resetModified();

    Fixture* f4 = new Fixture(&doc);
    f4->setName("Four");
    f4->setChannels(5);
    f4->setAddress(0);
    f4->setUniverse(0);
    doc.addFixture(f4);
    QVERIFY(f1->id() == 0);
    QVERIFY(f4->id() == 1); // Takes the place of the removed f2
    QVERIFY(f3->id() == 2);
    QVERIFY(doc.isModified() == true);
}

void Doc_Test::fixtureLimits()
{
    Doc doc(this, m_fixtureDefCache);

    for (t_fixture_id id = 0; id < KFixtureArraySize; id++)
    {
        Fixture* fxi = new Fixture(&doc);
        fxi->setName(QString("Test %1").arg(id));
        QVERIFY(doc.addFixture(fxi) == true);
        QVERIFY(doc.m_fixtureAllocation == id + 1);
    }

    doc.resetModified();

    Fixture* over = new Fixture(&doc);
    over->setName("Over Limits");
    QVERIFY(doc.addFixture(over) == false);
    QVERIFY(doc.isModified() == false);
    QVERIFY(doc.m_fixtureAllocation == KFixtureArraySize);
    delete over;
}

void Doc_Test::fixture()
{
    Doc doc(this, m_fixtureDefCache);

    Fixture* f1 = new Fixture(&doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    doc.addFixture(f1);

    Fixture* f2 = new Fixture(&doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    doc.addFixture(f2);

    Fixture* f3 = new Fixture(&doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    doc.addFixture(f3);

    QVERIFY(doc.fixture(f1->id()) == f1);
    QVERIFY(doc.fixture(f2->id()) == f2);
    QVERIFY(doc.fixture(f3->id()) == f3);
    QVERIFY(doc.fixture(f3->id() + 1) == NULL);
    QVERIFY(doc.fixture(42) == NULL);
    QVERIFY(doc.fixture(KFixtureArraySize) == NULL);
}

void Doc_Test::findAddress()
{
    Doc doc(this, m_fixtureDefCache);

    /* All addresses are available (except for fixtures taking more than
       one complete universe). */
    QVERIFY(doc.findAddress(15) == 0);
    QVERIFY(doc.findAddress(0) == QLCChannel::invalid());
    QVERIFY(doc.findAddress(512) == 0);
    QVERIFY(doc.findAddress(513) == QLCChannel::invalid());

    Fixture* f1 = new Fixture(&doc);
    f1->setChannels(15);
    f1->setAddress(10);
    doc.addFixture(f1);

    /* There's a fixture taking 15 channels (10-24) */
    QVERIFY(doc.findAddress(10) == 0);
    QVERIFY(doc.findAddress(11) == 25);

    Fixture* f2 = new Fixture(&doc);
    f2->setChannels(15);
    f2->setAddress(10);
    doc.addFixture(f2);

    /* Now there are two fixtures at the same address, with all channels
       overlapping. */
    QVERIFY(doc.findAddress(10) == 0);
    QVERIFY(doc.findAddress(11) == 25);

    /* Now only some channels overlap (f2: 0-14, f1: 10-24) */
    f2->setAddress(0);
    QVERIFY(doc.findAddress(1) == 25);
    QVERIFY(doc.findAddress(10) == 25);
    QVERIFY(doc.findAddress(11) == 25);

    Fixture* f3 = new Fixture(&doc);
    f3->setChannels(5);
    f3->setAddress(30);
    doc.addFixture(f3);

    doc.resetModified();

    /* Next free slot for max 5 channels is between 25 and 30 */
    QVERIFY(doc.findAddress(1) == 25);
    QVERIFY(doc.findAddress(5) == 25);
    QVERIFY(doc.findAddress(6) == 35);
    QVERIFY(doc.findAddress(11) == 35);

    /* Next free slot is found only from the next universe */
    QVERIFY(doc.findAddress(500) == 512);

    /* findAddress() must not affect modified state */
    QVERIFY(doc.isModified() == false);
}

void Doc_Test::totalPowerConsumption()
{
    Doc doc(this, m_fixtureDefCache);
    int fuzzy = 0;

    /* Load Showtec - MiniMax 250 with 250W power consumption */
    const QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Showtec", "MiniMax 250");
    Q_ASSERT(fixtureDef != NULL);
    const QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    /* Add a new fixture */
    Fixture* f1 = new Fixture(&doc);
    f1->setName("250W (total 250W)");
    f1->setChannels(6);
    f1->setAddress(0);
    f1->setUniverse(0);
    f1->setFixtureDefinition(fixtureDef, fixtureMode);
    QVERIFY(f1->fixtureDef() == fixtureDef);
    QVERIFY(f1->fixtureMode() == fixtureMode);
    QVERIFY(f1->fixtureMode()->physical().powerConsumption() == 250);
    QVERIFY(doc.addFixture(f1) == true);
    QVERIFY(doc.totalPowerConsumption(fuzzy) == 250);
    QVERIFY(fuzzy == 0);

    /* Add the same fixture once more */
    Fixture* f2 = new Fixture(&doc);
    f2->setName("250W (total 500W)");
    f2->setChannels(6);
    f2->setAddress(10);
    f2->setUniverse(0);
    f2->setFixtureDefinition(fixtureDef, fixtureMode);
    QVERIFY(f2->fixtureDef() == fixtureDef);
    QVERIFY(f2->fixtureMode() == fixtureMode);
    QVERIFY(f2->fixtureMode()->physical().powerConsumption() == 250);
    QVERIFY(doc.addFixture(f2) == true);
    QVERIFY(doc.totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 0);

    /* Test generic dimmer and fuzzy */
    Fixture* f3 = new Fixture(&doc);
    f3->setName("Generic Dimmer");
    f3->setChannels(6);
    f3->setAddress(20);
    f3->setUniverse(0);
    QVERIFY(doc.addFixture(f3) == true);
    QVERIFY(doc.totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 1);
    // reset fuzzy count
    fuzzy = 0;

    /* Test fuzzy count */
    Fixture* f4 = new Fixture(&doc);
    f4->setName("Generic Dimmer 2");
    f4->setChannels(6);
    f4->setAddress(30);
    f4->setUniverse(0);
    QVERIFY(doc.addFixture(f4) == true);
    QVERIFY(doc.totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 2);
}

void Doc_Test::addFunction()
{
    Doc doc(this, m_fixtureDefCache);
    QVERIFY(doc.m_functionAllocation == 0);

    Scene* s = new Scene(&doc);
    QVERIFY(s->id() == Function::invalidId());
    QVERIFY(doc.addFunction(s) == true);
    QVERIFY(s->id() == 0);
    QVERIFY(doc.m_functionAllocation == 1);
    QVERIFY(doc.isModified() == true);
    QCOMPARE(int(doc.functionsFree()), KFunctionArraySize - 1);

    doc.resetModified();

    Chaser* c = new Chaser(&doc);
    QVERIFY(c->id() == Function::invalidId());
    QVERIFY(doc.addFunction(c) == true);
    QVERIFY(c->id() == 1);
    QVERIFY(doc.m_functionAllocation == 2);
    QVERIFY(doc.isModified() == true);
    QCOMPARE(int(doc.functionsFree()), KFunctionArraySize - 2);

    doc.resetModified();

    Collection* o = new Collection(&doc);
    QVERIFY(o->id() == Function::invalidId());
    QVERIFY(doc.addFunction(o, 0) == false);
    QVERIFY(doc.isModified() == false);
    QVERIFY(o->id() == Function::invalidId());
    QVERIFY(doc.m_functionAllocation == 2);
    QVERIFY(doc.addFunction(o, 2) == true);
    QVERIFY(o->id() == 2);
    QVERIFY(doc.m_functionAllocation == 3);
    QVERIFY(doc.isModified() == true);
    QCOMPARE(int(doc.functionsFree()), KFunctionArraySize - 3);

    doc.resetModified();

    EFX* e = new EFX(&doc);
    QVERIFY(e->id() == Function::invalidId());
    QVERIFY(doc.addFunction(e, KFunctionArraySize) == false);
    QVERIFY(e->id() == Function::invalidId());
    QVERIFY(doc.addFunction(e) == true);
    QVERIFY(e->id() == 3);
    QVERIFY(doc.m_functionAllocation == 4);
    QVERIFY(doc.isModified() == true);
    QCOMPARE(int(doc.functionsFree()), KFunctionArraySize - 4);
}

void Doc_Test::deleteFunction()
{
    Doc doc(this, m_fixtureDefCache);

    Scene* s1 = new Scene(&doc);
    doc.addFunction(s1);

    Scene* s2 = new Scene(&doc);
    doc.addFunction(s2);

    Scene* s3 = new Scene(&doc);
    doc.addFunction(s3);

    doc.resetModified();

    QPointer <Scene> ptr(s2);
    QVERIFY(ptr != NULL);
    t_function_id id = s2->id();
    QVERIFY(doc.deleteFunction(id) == true);
    QVERIFY(doc.isModified() == true);

    doc.resetModified();

    QVERIFY(doc.deleteFunction(id) == false);
    QVERIFY(doc.deleteFunction(42) == false);
    QVERIFY(doc.isModified() == false);
    QVERIFY(ptr == NULL); // doc.deleteFunction() should also delete
    QVERIFY(doc.m_fixtureArray[id] == NULL);

    id = s1->id();
    QVERIFY(doc.deleteFunction(id) == true);
    QVERIFY(doc.m_fixtureArray[id] == NULL);
    QVERIFY(doc.isModified() == true);

    id = s3->id();
    QVERIFY(doc.deleteFunction(id) == true);
    QVERIFY(doc.m_fixtureArray[id] == NULL);
    QVERIFY(doc.isModified() == true);

    QVERIFY(doc.functions() == 0);
}

void Doc_Test::function()
{
    Doc doc(this, m_fixtureDefCache);

    Scene* s1 = new Scene(&doc);
    doc.addFunction(s1);

    Scene* s2 = new Scene(&doc);
    doc.addFunction(s2);

    Scene* s3 = new Scene(&doc);
    doc.addFunction(s3);

    QVERIFY(doc.function(s1->id()) == s1);
    QVERIFY(doc.function(s2->id()) == s2);
    QVERIFY(doc.function(s3->id()) == s3);

    t_function_id id = s2->id();
    doc.deleteFunction(id);
    QVERIFY(doc.function(id) == NULL);
}

void Doc_Test::functionLimits()
{
    Doc doc(this, m_fixtureDefCache);

    for (t_function_id id = 0; id < KFunctionArraySize; id++)
    {
        Scene* s = new Scene(&doc);
        s->setName(QString("Test %1").arg(id));
        QVERIFY(doc.addFunction(s) == true);
        QVERIFY(doc.m_functionAllocation == id + 1);
    }

    doc.resetModified();

    Scene* over = new Scene(&doc);
    over->setName("Over Limits");
    QVERIFY(doc.addFunction(over) == false);
    QVERIFY(doc.m_functionAllocation == KFunctionArraySize);
    delete over;
    QVERIFY(doc.isModified() == false);
}

void Doc_Test::load()
{
    Doc doc(this, m_fixtureDefCache);

    QDomDocument document;
    QDomElement root = document.createElement("Engine");

    root.appendChild(createFixtureNode(document, 0));
    root.appendChild(createFixtureNode(document, 72));
    root.appendChild(createFixtureNode(document, 15));

    root.appendChild(createCollectionNode(document, 5));
    root.appendChild(createCollectionNode(document, 9));
    root.appendChild(createCollectionNode(document, 1));
    root.appendChild(createCollectionNode(document, 7));

    root.appendChild(createBusNode(document, 0, 1));
    root.appendChild(createBusNode(document, 7, 2));
    root.appendChild(createBusNode(document, 12, 3));
    root.appendChild(createBusNode(document, 29, 4));
    root.appendChild(createBusNode(document, 31, 500));

    root.appendChild(document.createElement("ExtraTag"));

    QVERIFY(doc.fixtures() == 0);
    QVERIFY(doc.functions() == 0);
    QVERIFY(doc.loadXML(&root) == true);
    QVERIFY(doc.fixtures() == 3);
    QVERIFY(doc.functions() == 4);
    QVERIFY(Bus::instance()->value(0) == 1);
    QVERIFY(Bus::instance()->value(7) == 2);
    QVERIFY(Bus::instance()->value(12) == 3);
    QVERIFY(Bus::instance()->value(29) == 4);
    QVERIFY(Bus::instance()->value(31) == 500);
}

void Doc_Test::loadWrongRoot()
{
    Doc doc(this, m_fixtureDefCache);

    QDomDocument document;
    QDomElement root = document.createElement("Enjine");

    root.appendChild(createFixtureNode(document, 0));
    root.appendChild(createFixtureNode(document, 72));
    root.appendChild(createFixtureNode(document, 15));

    root.appendChild(createCollectionNode(document, 5));
    root.appendChild(createCollectionNode(document, 9));
    root.appendChild(createCollectionNode(document, 1));
    root.appendChild(createCollectionNode(document, 7));

    root.appendChild(createBusNode(document, 0, 1));
    root.appendChild(createBusNode(document, 7, 2));
    root.appendChild(createBusNode(document, 12, 3));
    root.appendChild(createBusNode(document, 29, 4));
    root.appendChild(createBusNode(document, 31, 500));

    root.appendChild(document.createElement("ExtraTag"));

    QVERIFY(doc.loadXML(&root) == false);
}

void Doc_Test::save()
{
    Doc doc(this, m_fixtureDefCache);

    Scene* s = new Scene(&doc);
    doc.addFunction(s);

    Fixture* f1 = new Fixture(&doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    doc.addFixture(f1);

    Chaser* c = new Chaser(&doc);
    doc.addFunction(c);

    Fixture* f2 = new Fixture(&doc);
    f2->setName("Two");
    f2->setChannels(10);
    f2->setAddress(20);
    f2->setUniverse(1);
    doc.addFixture(f2);

    Collection* o = new Collection(&doc);
    doc.addFunction(o);

    Fixture* f3 = new Fixture(&doc);
    f3->setName("Three");
    f3->setChannels(15);
    f3->setAddress(40);
    f3->setUniverse(2);
    doc.addFixture(f3);

    EFX* e = new EFX(&doc);
    doc.addFunction(e);

    QVERIFY(doc.isModified() == true);

    QDomDocument document;
    QDomElement root = document.createElement("TestRoot");

    QVERIFY(doc.saveXML(&document, &root) == true);

    unsigned int fixtures = 0, functions = 0, buses = 0;
    QDomNode node = root.firstChild();
    QVERIFY(node.toElement().tagName() == "Engine");

    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Fixture")
            fixtures++;
        else if (tag.tagName() == "Function")
            functions++;
        else if (tag.tagName() == "Bus")
            buses++;
        else
            QFAIL(QString("Unexpected tag: %1")
                  .arg(tag.tagName()).toAscii());

        node = node.nextSibling();
    }

    QVERIFY(fixtures == 3);
    QVERIFY(functions == 4);
    QVERIFY(buses == Bus::count());

    /* Saving doesn't implicitly reset modified status */
    QVERIFY(doc.isModified() == true);
}

QDomElement Doc_Test::createFixtureNode(QDomDocument& doc, t_fixture_id id)
{
    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode(QString("Fixture %1").arg(id));
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("3");
    uni.appendChild(uniText);
    root.appendChild(uni);

    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("Foobar");
    model.appendChild(modelText);
    root.appendChild(model);

    QDomElement mode = doc.createElement("Mode");
    QDomText modeText = doc.createTextNode("Foobar");
    mode.appendChild(modeText);
    root.appendChild(mode);

    QDomElement type = doc.createElement("Manufacturer");
    QDomText typeText = doc.createTextNode("Foobar");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement fxi_id = doc.createElement("ID");
    QDomText fxi_idText = doc.createTextNode(QString("%1").arg(id));
    fxi_id.appendChild(fxi_idText);
    root.appendChild(fxi_id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    return root;
}

QDomElement Doc_Test::createCollectionNode(QDomDocument& doc, t_function_id id)
{
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Collection");
    root.setAttribute("ID", QString("%1").arg(id));

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

    return root;
}

QDomElement Doc_Test::createBusNode(QDomDocument& doc, quint32 id, quint32 val)
{
    QDomElement root = doc.createElement("Bus");
    doc.appendChild(root);
    root.setAttribute("ID", id);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode(QString("Bus %1").arg(id));
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement value = doc.createElement("Value");
    QDomText valueText = doc.createTextNode(QString("%1").arg(val));
    value.appendChild(valueText);
    root.appendChild(value);

    return root;
}

