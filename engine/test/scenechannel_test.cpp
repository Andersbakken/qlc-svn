/*
  Q Light Controller - Unit test
  scenechannel_test.cpp

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

#include "scenechannel_test.h"
#include "scene.h"

void SceneChannel_Test::initial()
{
    SceneChannel sch;
    QVERIFY(sch.address == 0);
    QVERIFY(sch.start == 0);
    QVERIFY(sch.current == 0);
    QVERIFY(sch.target == 0);
}

void SceneChannel_Test::copy()
{
    SceneChannel sch;
    sch.address = 123;
    sch.start = 15;
    sch.current = 48;
    sch.target = 90;

    SceneChannel sch2 = sch;
    QVERIFY(sch2.address == 123);
    QVERIFY(sch2.start == 15);
    QVERIFY(sch2.current == 48);
    QVERIFY(sch2.target == 90);

    SceneChannel sch3(sch);
    QVERIFY(sch3.address == 123);
    QVERIFY(sch3.start == 15);
    QVERIFY(sch3.current == 48);
    QVERIFY(sch3.target == 90);
}
