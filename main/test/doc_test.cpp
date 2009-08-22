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

#include <QtTest>
#include <QtXml>

#include "doc_test.h"
#include "../fixture.h"

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

void Doc_Test::fixture()
{
}

void Doc_Test::cleanupTestCase()
{
}
