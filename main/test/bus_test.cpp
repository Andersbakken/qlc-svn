/*
  Q Light Controller - Unit test
  bus_test.cpp

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

#include "bus_test.h"
#include "../bus.h"

void Bus_Test::initTestCase()
{
	QVERIFY(Bus::instance() == NULL);
	Bus::init(this);
	QVERIFY(Bus::instance() != NULL);
}

void Bus_Test::defaults()
{
	QVERIFY(Bus::count() == 32);
	QVERIFY(Bus::defaultFade() == 0);
	QVERIFY(Bus::defaultHold() == 1);
	QVERIFY(Bus::instance()->name(0) == QString("Fade"));
	QVERIFY(Bus::instance()->name(1) == QString("Hold"));
}

void Bus_Test::value()
{
	QSignalSpy spy(Bus::instance(), SIGNAL(valueChanged(quint32,quint32)));

	/* Setting bus value should produce a signal */
	Bus::instance()->setValue(0, 15);
	QVERIFY(Bus::instance()->value(0) == 15);
	QVERIFY(spy.count() == 1);
	QVERIFY(spy.at(0).count() == 2);
	QVERIFY(spy.at(0).at(0).toUInt() == 0);
	QVERIFY(spy.at(0).at(1).toUInt() == 15);

	/* Another bus should change only one bus value */
	Bus::instance()->setValue(5, 30);
	QVERIFY(Bus::instance()->value(0) == 15);
	QVERIFY(Bus::instance()->value(5) == 30);
	QVERIFY(spy.count() == 2);
	QVERIFY(spy.at(1).at(0).toUInt() == 5);
	QVERIFY(spy.at(1).at(1).toUInt() == 30);

	/* Invalid bus shouldn't produce signals */
	Bus::instance()->setValue(Bus::count(), 30);
	QVERIFY(Bus::instance()->value(0) == 15);
	QVERIFY(Bus::instance()->value(5) == 30);
	QVERIFY(spy.count() == 2);

	/* Invalid bus shouldn't produce signals */
	Bus::instance()->setValue(0, UINT_MAX);
	QVERIFY(Bus::instance()->value(0) == UINT_MAX);
	QVERIFY(Bus::instance()->value(5) == 30);
	QVERIFY(spy.count() == 3);
	QVERIFY(spy.at(2).at(0).toUInt() == 0);
	QVERIFY(spy.at(2).at(1).toUInt() == UINT_MAX);
}

void Bus_Test::name()
{
	QSignalSpy spy(Bus::instance(), SIGNAL(nameChanged(quint32,QString)));

	/* Setting bus name should produce a signal */
	QVERIFY(Bus::instance()->name(0) == QString("Fade"));
	Bus::instance()->setName(0, "Foo");
	QVERIFY(Bus::instance()->name(0) == QString("Foo"));
	QVERIFY(spy.count() == 1);
	QVERIFY(spy.at(0).at(0).toUInt() == 0);
	QVERIFY(spy.at(0).at(1).toString() == QString("Foo"));

	/* Invalid bus should not produce a signal */
	Bus::instance()->setName(5000, "Bar");
	QVERIFY(spy.count() == 1);
}

void Bus_Test::tap()
{
	QSignalSpy spy(Bus::instance(), SIGNAL(tapped(quint32)));

	/* Tapping an existing bus should produce a signal */
	Bus::instance()->tap(17);
	QVERIFY(spy.count() == 1);
	QVERIFY(spy.at(0).at(0).toUInt() == 17);

	/* Tapping a non-existing bus should produce a signal */
	Bus::instance()->tap(6342);
	QVERIFY(spy.count() == 1);
	QVERIFY(spy.at(0).at(0).toUInt() == 17);
}

void Bus_Test::cleanupTestCase()
{
	QVERIFY(Bus::instance() != NULL);
	delete Bus::instance();
	QVERIFY(Bus::instance() == NULL);
}
