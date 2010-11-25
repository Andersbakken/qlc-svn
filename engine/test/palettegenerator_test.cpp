/*
  Q Light Controller - Unit test
  palettegenerator_test.cpp

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
#include <QList>

#include "palettegenerator_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#define protected public
#define private public
#include "palettegenerator.h"
#undef private
#undef protected

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void PaletteGenerator_Test::initTestCase()
{
    Bus::init(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_fixtureDefCache.load(dir) == true);
}

void PaletteGenerator_Test::initial()
{
    Doc doc(this, m_fixtureDefCache);
    QList <Fixture*> list;
    list << new Fixture(&doc);
    list << new Fixture(&doc);
    list << new Fixture(&doc);

    PaletteGenerator pg(&doc, list);
    QCOMPARE(pg.m_doc, &doc);
    QCOMPARE(pg.m_fixtures.size(), 3);
    QCOMPARE(pg.m_scenes.size(), 0);
}

void PaletteGenerator_Test::findChannels()
{
    Doc doc(this, m_fixtureDefCache);

    const QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Showtec", "MiniMax 250");
    Q_ASSERT(fixtureDef != NULL);
    const QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    Fixture* fxi = new Fixture(&doc);
    fxi->setFixtureDefinition(fixtureDef, fixtureMode);

    QList <quint32> chs;
    chs = PaletteGenerator::findChannels(fxi, QLCChannel::Colour);
    QCOMPARE(chs.size(), 1);
    chs = PaletteGenerator::findChannels(fxi, QLCChannel::Gobo);
    QCOMPARE(chs.size(), 1);
    chs = PaletteGenerator::findChannels(fxi, QLCChannel::Intensity);
    QCOMPARE(chs.size(), 1);

    fixtureDef = m_fixtureDefCache.fixtureDef("Martin", "MAC300");
    Q_ASSERT(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);
    chs = PaletteGenerator::findChannels(fxi, QLCChannel::Gobo);
    QCOMPARE(chs.size(), 1);
    chs = PaletteGenerator::findChannels(fxi, QLCChannel::Colour);
    // MAC300 has 4 colour channels but only one with a fixed colour wheel
    QCOMPARE(chs.size(), 1);
}

void PaletteGenerator_Test::createColours()
{
    Doc doc(this, m_fixtureDefCache);

    const QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(fixtureDef != NULL);
    const QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    QList <Fixture*> list;
    Fixture* fxi1 = new Fixture(&doc);
    fxi1->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi1;
    doc.addFixture(fxi1);

    Fixture* fxi2 = new Fixture(&doc);
    fxi2->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi2;
    doc.addFixture(fxi2);

    PaletteGenerator pg(&doc, list);
    pg.createColours();
    QCOMPARE(doc.functions(), 10); // 10 colours
    for (t_function_id i = 0; i < 10; i++)
    {
        Scene* s = qobject_cast<Scene*> (doc.function(i));
        QVERIFY(s != NULL);
        QCOMPARE(s->values().size(), 2); // One colour for two fixtures
        QCOMPARE(s->values().at(0).fxi, fxi1->id());
        QCOMPARE(s->values().at(0).channel, quint32(2)); // DJScan colour channel
        QCOMPARE(s->values().at(1).fxi, fxi2->id());
        QCOMPARE(s->values().at(1).channel, quint32(2)); // DJScan colour channel
    }
}
