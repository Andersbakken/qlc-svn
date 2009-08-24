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

#include "../collection.h"
#include "../fixture.h"
#include "../chaser.h"
#include "../scene.h"
#include "../efx.h"

#include "doc_test.h"
#define protected public
#include "../doc.h"
#undef protected

void Doc_Test::initTestCase()
{
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

	/* Add a completely new fixture */
	Fixture* f1 = new Fixture(&doc);
	f1->setName("One");
	f1->setChannels(5);
	f1->setAddress(0);
	f1->setUniverse(0);
	QVERIFY(doc.addFixture(f1) == true);
	QVERIFY(doc.m_fixtureAllocation == 1);
	QVERIFY(f1->id() == 0);

	/* Add another fixture but attempt to put assign it an already-assigned
	   fixture ID. */
	Fixture* f2 = new Fixture(&doc);
	f2->setName("Two");
	f2->setChannels(5);
	f2->setAddress(0);
	f2->setUniverse(0);
	QVERIFY(doc.addFixture(f2, f1->id()) == false);
	QVERIFY(doc.m_fixtureAllocation == 1);

	/* But, the fixture can be added if we give it an unassigned ID. */
	QVERIFY(doc.addFixture(f2, f1->id() + 1) == true);
	QVERIFY(doc.m_fixtureAllocation == 2);
	QVERIFY(f1->id() == 0);
	QVERIFY(f2->id() == 1);

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

	QVERIFY(doc.deleteFixture(42) == false);
	QVERIFY(doc.m_fixtureAllocation == 3);

	QVERIFY(doc.deleteFixture(Fixture::invalidId()) == false);
	QVERIFY(doc.m_fixtureAllocation == 3);

	t_fixture_id id = f2->id();
	QVERIFY(doc.deleteFixture(id) == true);
	QVERIFY(doc.m_fixtureAllocation == 2);

	QVERIFY(doc.deleteFixture(id) == false);
	QVERIFY(doc.m_fixtureAllocation == 2);

	Fixture* f4 = new Fixture(&doc);
	f4->setName("Four");
	f4->setChannels(5);
	f4->setAddress(0);
	f4->setUniverse(0);
	doc.addFixture(f4);
	QVERIFY(f1->id() == 0);
	QVERIFY(f4->id() == 1); // Takes the place of the removed f2
	QVERIFY(f3->id() == 2);
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

	Fixture* over = new Fixture(&doc);
	over->setName("Over Limits");
	QVERIFY(doc.addFixture(over) == false);
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
	QVERIFY(doc.findAddress(0) == KChannelInvalid);
	QVERIFY(doc.findAddress(512) == 0);
	QVERIFY(doc.findAddress(513) == KChannelInvalid);

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

	/* Next free slot for max 5 channels is between 25 and 30 */
	QVERIFY(doc.findAddress(1) == 25);
	QVERIFY(doc.findAddress(5) == 25);
	QVERIFY(doc.findAddress(6) == 35);
	QVERIFY(doc.findAddress(11) == 35);

	/* Next free slot is found only from the next universe */
	QVERIFY(doc.findAddress(500) == 512);
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

	Chaser* c = new Chaser(&doc);
	QVERIFY(c->id() == Function::invalidId());
	QVERIFY(doc.addFunction(c) == true);
	QVERIFY(c->id() == 1);
	QVERIFY(doc.m_functionAllocation == 2);

	Collection* o = new Collection(&doc);
	QVERIFY(o->id() == Function::invalidId());
	QVERIFY(doc.addFunction(o, 0) == false);
	QVERIFY(o->id() == Function::invalidId());
	QVERIFY(doc.m_functionAllocation == 2);
	QVERIFY(doc.addFunction(o, 2) == true);
	QVERIFY(o->id() == 2);
	QVERIFY(doc.m_functionAllocation == 3);

	EFX* e = new EFX(&doc);
	QVERIFY(e->id() == Function::invalidId());
	QVERIFY(doc.addFunction(e, KFunctionArraySize) == false);
	QVERIFY(e->id() == Function::invalidId());
	QVERIFY(doc.addFunction(e) == true);
	QVERIFY(e->id() == 3);
	QVERIFY(doc.m_functionAllocation == 4);
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

	QPointer <Scene> ptr(s2);
	QVERIFY(ptr != NULL);
	t_function_id id = s2->id();
	QVERIFY(doc.deleteFunction(id) == true);
	QVERIFY(doc.deleteFunction(id) == false);
	QVERIFY(doc.deleteFunction(42) == false);
	QVERIFY(ptr == NULL); // doc.deleteFunction() should also delete
	QVERIFY(doc.m_fixtureArray[id] == NULL);
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

	Scene* over = new Scene(&doc);
	over->setName("Over Limits");
	QVERIFY(doc.addFunction(over) == false);
	QVERIFY(doc.m_functionAllocation == KFunctionArraySize);
	delete over;
}

void Doc_Test::cleanupTestCase()
{
}
