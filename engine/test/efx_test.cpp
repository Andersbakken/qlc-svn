/*
  Q Light Controller - Unit test
  efx_test.cpp

  Copyright (c) Heikki Junnila

  This program is free software;you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY;without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program;if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtTest>
#include <QtXml>
#include <QList>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

#include "mastertimer_stub.h"
#include "outputmap_stub.h"
#include "efx_test.h"

#include "function.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

/* Expose protected members to the unit test */
#define protected public
#include "efx.h"
#undef protected

void EFX_Test::initTestCase()
{
    Bus::init(this);
    QVERIFY(m_cache.load("../../fixtures/") == true);
}

void EFX_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void EFX_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void EFX_Test::initial()
{
    EFX e(m_doc);
    QVERIFY(e.type() == Function::EFX);
    QVERIFY(e.name() == "New EFX");
    QVERIFY(e.id() == Function::invalidId());
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

    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX e(m_doc);

    e.setXOffset(300);
    QVERIFY(e.xOffset() == UCHAR_MAX);

    e.setXOffset(0);
    QVERIFY(e.xOffset() == 0);

    e.setXOffset(256);
    QVERIFY(e.xOffset() == UCHAR_MAX);

    e.setXOffset(12);
    QVERIFY(e.xOffset() == 12);

    e.setXOffset(-4);
    QVERIFY(e.xOffset() == 0);
}

void EFX_Test::yOffset()
{
    EFX e(m_doc);

    e.setYOffset(300);
    QVERIFY(e.yOffset() == UCHAR_MAX);

    e.setYOffset(0);
    QVERIFY(e.yOffset() == 0);

    e.setYOffset(256);
    QVERIFY(e.yOffset() == UCHAR_MAX);

    e.setYOffset(12);
    QVERIFY(e.yOffset() == 12);

    e.setYOffset(-4);
    QVERIFY(e.yOffset() == 0);
}

void EFX_Test::xFrequency()
{
    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX e(m_doc);

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
    EFX* e = new EFX(m_doc);
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
    EFX e(m_doc);
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
    EFX e(m_doc);

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

void EFX_Test::previewCircle()
{
    EFX e(m_doc);

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    QVERIFY(poly[0] == QPoint(127,254));
    QVERIFY(poly[1] == QPoint(120,253));
    QVERIFY(poly[2] == QPoint(114,253));
    QVERIFY(poly[3] == QPoint(108,252));
    QVERIFY(poly[4] == QPoint(102,251));
    QVERIFY(poly[5] == QPoint(96,250));
    QVERIFY(poly[6] == QPoint(90,248));
    QVERIFY(poly[7] == QPoint(84,246));
    QVERIFY(poly[8] == QPoint(78,244));
    QVERIFY(poly[9] == QPoint(72,241));
    QVERIFY(poly[10] == QPoint(67,239));
    QVERIFY(poly[11] == QPoint(61,235));
    QVERIFY(poly[12] == QPoint(56,232));
    QVERIFY(poly[13] == QPoint(51,229));
    QVERIFY(poly[14] == QPoint(46,225));
    QVERIFY(poly[15] == QPoint(41,221));
    QVERIFY(poly[16] == QPoint(37,216));
    QVERIFY(poly[17] == QPoint(32,212));
    QVERIFY(poly[18] == QPoint(28,207));
    QVERIFY(poly[19] == QPoint(24,202));
    QVERIFY(poly[20] == QPoint(21,197));
    QVERIFY(poly[21] == QPoint(18,192));
    QVERIFY(poly[22] == QPoint(14,186));
    QVERIFY(poly[23] == QPoint(12,181));
    QVERIFY(poly[24] == QPoint(9,175));
    QVERIFY(poly[25] == QPoint(7,169));
    QVERIFY(poly[26] == QPoint(5,163));
    QVERIFY(poly[27] == QPoint(3,157));
    QVERIFY(poly[28] == QPoint(2,151));
    QVERIFY(poly[29] == QPoint(1,145));
    QVERIFY(poly[30] == QPoint(0,139));
    QVERIFY(poly[31] == QPoint(0,133));
    QVERIFY(poly[32] == QPoint(0,126));
    QVERIFY(poly[33] == QPoint(0,120));
    QVERIFY(poly[34] == QPoint(0,114));
    QVERIFY(poly[35] == QPoint(1,108));
    QVERIFY(poly[36] == QPoint(2,102));
    QVERIFY(poly[37] == QPoint(3,96));
    QVERIFY(poly[38] == QPoint(5,90));
    QVERIFY(poly[39] == QPoint(7,84));
    QVERIFY(poly[40] == QPoint(9,78));
    QVERIFY(poly[41] == QPoint(12,72));
    QVERIFY(poly[42] == QPoint(14,67));
    QVERIFY(poly[43] == QPoint(18,61));
    QVERIFY(poly[44] == QPoint(21,56));
    QVERIFY(poly[45] == QPoint(24,51));
    QVERIFY(poly[46] == QPoint(28,46));
    QVERIFY(poly[47] == QPoint(32,41));
    QVERIFY(poly[48] == QPoint(37,37));
    QVERIFY(poly[49] == QPoint(41,32));
    QVERIFY(poly[50] == QPoint(46,28));
    QVERIFY(poly[51] == QPoint(51,24));
    QVERIFY(poly[52] == QPoint(56,21));
    QVERIFY(poly[53] == QPoint(61,18));
    QVERIFY(poly[54] == QPoint(67,14));
    QVERIFY(poly[55] == QPoint(72,12));
    QVERIFY(poly[56] == QPoint(78,9));
    QVERIFY(poly[57] == QPoint(84,7));
    QVERIFY(poly[58] == QPoint(90,5));
    QVERIFY(poly[59] == QPoint(96,3));
    QVERIFY(poly[60] == QPoint(102,2));
    QVERIFY(poly[61] == QPoint(108,1));
    QVERIFY(poly[62] == QPoint(114,0));
    QVERIFY(poly[63] == QPoint(120,0));
    QVERIFY(poly[64] == QPoint(126,0));
    QVERIFY(poly[65] == QPoint(133,0));
    QVERIFY(poly[66] == QPoint(139,0));
    QVERIFY(poly[67] == QPoint(145,1));
    QVERIFY(poly[68] == QPoint(151,2));
    QVERIFY(poly[69] == QPoint(157,3));
    QVERIFY(poly[70] == QPoint(163,5));
    QVERIFY(poly[71] == QPoint(169,7));
    QVERIFY(poly[72] == QPoint(175,9));
    QVERIFY(poly[73] == QPoint(181,12));
    QVERIFY(poly[74] == QPoint(186,14));
    QVERIFY(poly[75] == QPoint(192,18));
    QVERIFY(poly[76] == QPoint(197,21));
    QVERIFY(poly[77] == QPoint(202,24));
    QVERIFY(poly[78] == QPoint(207,28));
    QVERIFY(poly[79] == QPoint(212,32));
    QVERIFY(poly[80] == QPoint(216,37));
    QVERIFY(poly[81] == QPoint(221,41));
    QVERIFY(poly[82] == QPoint(225,46));
    QVERIFY(poly[83] == QPoint(229,51));
    QVERIFY(poly[84] == QPoint(232,56));
    QVERIFY(poly[85] == QPoint(235,61));
    QVERIFY(poly[86] == QPoint(239,67));
    QVERIFY(poly[87] == QPoint(241,72));
    QVERIFY(poly[88] == QPoint(244,78));
    QVERIFY(poly[89] == QPoint(246,84));
    QVERIFY(poly[90] == QPoint(248,90));
    QVERIFY(poly[91] == QPoint(250,96));
    QVERIFY(poly[92] == QPoint(251,102));
    QVERIFY(poly[93] == QPoint(252,108));
    QVERIFY(poly[94] == QPoint(253,114));
    QVERIFY(poly[95] == QPoint(253,120));
    QVERIFY(poly[96] == QPoint(254,126));
    QVERIFY(poly[97] == QPoint(253,133));
    QVERIFY(poly[98] == QPoint(253,139));
    QVERIFY(poly[99] == QPoint(252,145));
    QVERIFY(poly[100] == QPoint(251,151));
    QVERIFY(poly[101] == QPoint(250,157));
    QVERIFY(poly[102] == QPoint(248,163));
    QVERIFY(poly[103] == QPoint(246,169));
    QVERIFY(poly[104] == QPoint(244,175));
    QVERIFY(poly[105] == QPoint(241,181));
    QVERIFY(poly[106] == QPoint(239,186));
    QVERIFY(poly[107] == QPoint(235,192));
    QVERIFY(poly[108] == QPoint(232,197));
    QVERIFY(poly[109] == QPoint(229,202));
    QVERIFY(poly[110] == QPoint(225,207));
    QVERIFY(poly[111] == QPoint(221,212));
    QVERIFY(poly[112] == QPoint(216,216));
    QVERIFY(poly[113] == QPoint(212,221));
    QVERIFY(poly[114] == QPoint(207,225));
    QVERIFY(poly[115] == QPoint(202,229));
    QVERIFY(poly[116] == QPoint(197,232));
    QVERIFY(poly[117] == QPoint(192,235));
    QVERIFY(poly[118] == QPoint(186,239));
    QVERIFY(poly[119] == QPoint(181,241));
    QVERIFY(poly[120] == QPoint(175,244));
    QVERIFY(poly[121] == QPoint(169,246));
    QVERIFY(poly[122] == QPoint(163,248));
    QVERIFY(poly[123] == QPoint(157,250));
    QVERIFY(poly[124] == QPoint(151,251));
    QVERIFY(poly[125] == QPoint(145,252));
    QVERIFY(poly[126] == QPoint(139,253));
    QVERIFY(poly[127] == QPoint(133,253));
}

void EFX_Test::previewEight()
{
    EFX e(m_doc);
    e.setAlgorithm("Eight");

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    QVERIFY(poly[0] == QPoint(127,254));
    QVERIFY(poly[1] == QPoint(114,253));
    QVERIFY(poly[2] == QPoint(102,253));
    QVERIFY(poly[3] == QPoint(90,252));
    QVERIFY(poly[4] == QPoint(78,251));
    QVERIFY(poly[5] == QPoint(67,250));
    QVERIFY(poly[6] == QPoint(56,248));
    QVERIFY(poly[7] == QPoint(46,246));
    QVERIFY(poly[8] == QPoint(37,244));
    QVERIFY(poly[9] == QPoint(28,241));
    QVERIFY(poly[10] == QPoint(21,239));
    QVERIFY(poly[11] == QPoint(14,235));
    QVERIFY(poly[12] == QPoint(9,232));
    QVERIFY(poly[13] == QPoint(5,229));
    QVERIFY(poly[14] == QPoint(2,225));
    QVERIFY(poly[15] == QPoint(0,221));
    QVERIFY(poly[16] == QPoint(0,216));
    QVERIFY(poly[17] == QPoint(0,212));
    QVERIFY(poly[18] == QPoint(2,207));
    QVERIFY(poly[19] == QPoint(5,202));
    QVERIFY(poly[20] == QPoint(9,197));
    QVERIFY(poly[21] == QPoint(14,192));
    QVERIFY(poly[22] == QPoint(21,186));
    QVERIFY(poly[23] == QPoint(28,181));
    QVERIFY(poly[24] == QPoint(37,175));
    QVERIFY(poly[25] == QPoint(46,169));
    QVERIFY(poly[26] == QPoint(56,163));
    QVERIFY(poly[27] == QPoint(67,157));
    QVERIFY(poly[28] == QPoint(78,151));
    QVERIFY(poly[29] == QPoint(90,145));
    QVERIFY(poly[30] == QPoint(102,139));
    QVERIFY(poly[31] == QPoint(114,133));
    QVERIFY(poly[32] == QPoint(127,126));
    QVERIFY(poly[33] == QPoint(139,120));
    QVERIFY(poly[34] == QPoint(151,114));
    QVERIFY(poly[35] == QPoint(163,108));
    QVERIFY(poly[36] == QPoint(175,102));
    QVERIFY(poly[37] == QPoint(186,96));
    QVERIFY(poly[38] == QPoint(197,90));
    QVERIFY(poly[39] == QPoint(207,84));
    QVERIFY(poly[40] == QPoint(216,78));
    QVERIFY(poly[41] == QPoint(225,72));
    QVERIFY(poly[42] == QPoint(232,67));
    QVERIFY(poly[43] == QPoint(239,61));
    QVERIFY(poly[44] == QPoint(244,56));
    QVERIFY(poly[45] == QPoint(248,51));
    QVERIFY(poly[46] == QPoint(251,46));
    QVERIFY(poly[47] == QPoint(253,41));
    QVERIFY(poly[48] == QPoint(254,37));
    QVERIFY(poly[49] == QPoint(253,32));
    QVERIFY(poly[50] == QPoint(251,28));
    QVERIFY(poly[51] == QPoint(248,24));
    QVERIFY(poly[52] == QPoint(244,21));
    QVERIFY(poly[53] == QPoint(239,18));
    QVERIFY(poly[54] == QPoint(232,14));
    QVERIFY(poly[55] == QPoint(225,12));
    QVERIFY(poly[56] == QPoint(216,9));
    QVERIFY(poly[57] == QPoint(207,7));
    QVERIFY(poly[58] == QPoint(197,5));
    QVERIFY(poly[59] == QPoint(186,3));
    QVERIFY(poly[60] == QPoint(175,2));
    QVERIFY(poly[61] == QPoint(163,1));
    QVERIFY(poly[62] == QPoint(151,0));
    QVERIFY(poly[63] == QPoint(139,0));
    QVERIFY(poly[64] == QPoint(127,0));
    QVERIFY(poly[65] == QPoint(114,0));
    QVERIFY(poly[66] == QPoint(102,0));
    QVERIFY(poly[67] == QPoint(90,1));
    QVERIFY(poly[68] == QPoint(78,2));
    QVERIFY(poly[69] == QPoint(67,3));
    QVERIFY(poly[70] == QPoint(56,5));
    QVERIFY(poly[71] == QPoint(46,7));
    QVERIFY(poly[72] == QPoint(37,9));
    QVERIFY(poly[73] == QPoint(28,12));
    QVERIFY(poly[74] == QPoint(21,14));
    QVERIFY(poly[75] == QPoint(14,18));
    QVERIFY(poly[76] == QPoint(9,21));
    QVERIFY(poly[77] == QPoint(5,24));
    QVERIFY(poly[78] == QPoint(2,28));
    QVERIFY(poly[79] == QPoint(0,32));
    QVERIFY(poly[80] == QPoint(0,37));
    QVERIFY(poly[81] == QPoint(0,41));
    QVERIFY(poly[82] == QPoint(2,46));
    QVERIFY(poly[83] == QPoint(5,51));
    QVERIFY(poly[84] == QPoint(9,56));
    QVERIFY(poly[85] == QPoint(14,61));
    QVERIFY(poly[86] == QPoint(21,67));
    QVERIFY(poly[87] == QPoint(28,72));
    QVERIFY(poly[88] == QPoint(37,78));
    QVERIFY(poly[89] == QPoint(46,84));
    QVERIFY(poly[90] == QPoint(56,90));
    QVERIFY(poly[91] == QPoint(67,96));
    QVERIFY(poly[92] == QPoint(78,102));
    QVERIFY(poly[93] == QPoint(90,108));
    QVERIFY(poly[94] == QPoint(102,114));
    QVERIFY(poly[95] == QPoint(114,120));
    QVERIFY(poly[96] == QPoint(126,126));
    QVERIFY(poly[97] == QPoint(139,133));
    QVERIFY(poly[98] == QPoint(151,139));
    QVERIFY(poly[99] == QPoint(163,145));
    QVERIFY(poly[100] == QPoint(175,151));
    QVERIFY(poly[101] == QPoint(186,157));
    QVERIFY(poly[102] == QPoint(197,163));
    QVERIFY(poly[103] == QPoint(207,169));
    QVERIFY(poly[104] == QPoint(216,175));
    QVERIFY(poly[105] == QPoint(225,181));
    QVERIFY(poly[106] == QPoint(232,186));
    QVERIFY(poly[107] == QPoint(239,192));
    QVERIFY(poly[108] == QPoint(244,197));
    QVERIFY(poly[109] == QPoint(248,202));
    QVERIFY(poly[110] == QPoint(251,207));
    QVERIFY(poly[111] == QPoint(253,212));
    QVERIFY(poly[112] == QPoint(254,216));
    QVERIFY(poly[113] == QPoint(253,221));
    QVERIFY(poly[114] == QPoint(251,225));
    QVERIFY(poly[115] == QPoint(248,229));
    QVERIFY(poly[116] == QPoint(244,232));
    QVERIFY(poly[117] == QPoint(239,235));
    QVERIFY(poly[118] == QPoint(232,239));
    QVERIFY(poly[119] == QPoint(225,241));
    QVERIFY(poly[120] == QPoint(216,244));
    QVERIFY(poly[121] == QPoint(207,246));
    QVERIFY(poly[122] == QPoint(197,248));
    QVERIFY(poly[123] == QPoint(186,250));
    QVERIFY(poly[124] == QPoint(175,251));
    QVERIFY(poly[125] == QPoint(163,252));
    QVERIFY(poly[126] == QPoint(151,253));
    QVERIFY(poly[127] == QPoint(139,253));
}

void EFX_Test::previewLine()
{
    EFX e(m_doc);
    e.setAlgorithm("Line");

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    QVERIFY(poly[0] == QPoint(254,254));
    QVERIFY(poly[1] == QPoint(253,253));
    QVERIFY(poly[2] == QPoint(253,253));
    QVERIFY(poly[3] == QPoint(252,252));
    QVERIFY(poly[4] == QPoint(251,251));
    QVERIFY(poly[5] == QPoint(250,250));
    QVERIFY(poly[6] == QPoint(248,248));
    QVERIFY(poly[7] == QPoint(246,246));
    QVERIFY(poly[8] == QPoint(244,244));
    QVERIFY(poly[9] == QPoint(241,241));
    QVERIFY(poly[10] == QPoint(239,239));
    QVERIFY(poly[11] == QPoint(235,235));
    QVERIFY(poly[12] == QPoint(232,232));
    QVERIFY(poly[13] == QPoint(229,229));
    QVERIFY(poly[14] == QPoint(225,225));
    QVERIFY(poly[15] == QPoint(221,221));
    QVERIFY(poly[16] == QPoint(216,216));
    QVERIFY(poly[17] == QPoint(212,212));
    QVERIFY(poly[18] == QPoint(207,207));
    QVERIFY(poly[19] == QPoint(202,202));
    QVERIFY(poly[20] == QPoint(197,197));
    QVERIFY(poly[21] == QPoint(192,192));
    QVERIFY(poly[22] == QPoint(186,186));
    QVERIFY(poly[23] == QPoint(181,181));
    QVERIFY(poly[24] == QPoint(175,175));
    QVERIFY(poly[25] == QPoint(169,169));
    QVERIFY(poly[26] == QPoint(163,163));
    QVERIFY(poly[27] == QPoint(157,157));
    QVERIFY(poly[28] == QPoint(151,151));
    QVERIFY(poly[29] == QPoint(145,145));
    QVERIFY(poly[30] == QPoint(139,139));
    QVERIFY(poly[31] == QPoint(133,133));
    QVERIFY(poly[32] == QPoint(126,126));
    QVERIFY(poly[33] == QPoint(120,120));
    QVERIFY(poly[34] == QPoint(114,114));
    QVERIFY(poly[35] == QPoint(108,108));
    QVERIFY(poly[36] == QPoint(102,102));
    QVERIFY(poly[37] == QPoint(96,96));
    QVERIFY(poly[38] == QPoint(90,90));
    QVERIFY(poly[39] == QPoint(84,84));
    QVERIFY(poly[40] == QPoint(78,78));
    QVERIFY(poly[41] == QPoint(72,72));
    QVERIFY(poly[42] == QPoint(67,67));
    QVERIFY(poly[43] == QPoint(61,61));
    QVERIFY(poly[44] == QPoint(56,56));
    QVERIFY(poly[45] == QPoint(51,51));
    QVERIFY(poly[46] == QPoint(46,46));
    QVERIFY(poly[47] == QPoint(41,41));
    QVERIFY(poly[48] == QPoint(37,37));
    QVERIFY(poly[49] == QPoint(32,32));
    QVERIFY(poly[50] == QPoint(28,28));
    QVERIFY(poly[51] == QPoint(24,24));
    QVERIFY(poly[52] == QPoint(21,21));
    QVERIFY(poly[53] == QPoint(18,18));
    QVERIFY(poly[54] == QPoint(14,14));
    QVERIFY(poly[55] == QPoint(12,12));
    QVERIFY(poly[56] == QPoint(9,9));
    QVERIFY(poly[57] == QPoint(7,7));
    QVERIFY(poly[58] == QPoint(5,5));
    QVERIFY(poly[59] == QPoint(3,3));
    QVERIFY(poly[60] == QPoint(2,2));
    QVERIFY(poly[61] == QPoint(1,1));
    QVERIFY(poly[62] == QPoint(0,0));
    QVERIFY(poly[63] == QPoint(0,0));
    QVERIFY(poly[64] == QPoint(0,0));
    QVERIFY(poly[65] == QPoint(0,0));
    QVERIFY(poly[66] == QPoint(0,0));
    QVERIFY(poly[67] == QPoint(1,1));
    QVERIFY(poly[68] == QPoint(2,2));
    QVERIFY(poly[69] == QPoint(3,3));
    QVERIFY(poly[70] == QPoint(5,5));
    QVERIFY(poly[71] == QPoint(7,7));
    QVERIFY(poly[72] == QPoint(9,9));
    QVERIFY(poly[73] == QPoint(12,12));
    QVERIFY(poly[74] == QPoint(14,14));
    QVERIFY(poly[75] == QPoint(18,18));
    QVERIFY(poly[76] == QPoint(21,21));
    QVERIFY(poly[77] == QPoint(24,24));
    QVERIFY(poly[78] == QPoint(28,28));
    QVERIFY(poly[79] == QPoint(32,32));
    QVERIFY(poly[80] == QPoint(37,37));
    QVERIFY(poly[81] == QPoint(41,41));
    QVERIFY(poly[82] == QPoint(46,46));
    QVERIFY(poly[83] == QPoint(51,51));
    QVERIFY(poly[84] == QPoint(56,56));
    QVERIFY(poly[85] == QPoint(61,61));
    QVERIFY(poly[86] == QPoint(67,67));
    QVERIFY(poly[87] == QPoint(72,72));
    QVERIFY(poly[88] == QPoint(78,78));
    QVERIFY(poly[89] == QPoint(84,84));
    QVERIFY(poly[90] == QPoint(90,90));
    QVERIFY(poly[91] == QPoint(96,96));
    QVERIFY(poly[92] == QPoint(102,102));
    QVERIFY(poly[93] == QPoint(108,108));
    QVERIFY(poly[94] == QPoint(114,114));
    QVERIFY(poly[95] == QPoint(120,120));
    QVERIFY(poly[96] == QPoint(126,126));
    QVERIFY(poly[97] == QPoint(133,133));
    QVERIFY(poly[98] == QPoint(139,139));
    QVERIFY(poly[99] == QPoint(145,145));
    QVERIFY(poly[100] == QPoint(151,151));
    QVERIFY(poly[101] == QPoint(157,157));
    QVERIFY(poly[102] == QPoint(163,163));
    QVERIFY(poly[103] == QPoint(169,169));
    QVERIFY(poly[104] == QPoint(175,175));
    QVERIFY(poly[105] == QPoint(181,181));
    QVERIFY(poly[106] == QPoint(186,186));
    QVERIFY(poly[107] == QPoint(192,192));
    QVERIFY(poly[108] == QPoint(197,197));
    QVERIFY(poly[109] == QPoint(202,202));
    QVERIFY(poly[110] == QPoint(207,207));
    QVERIFY(poly[111] == QPoint(212,212));
    QVERIFY(poly[112] == QPoint(216,216));
    QVERIFY(poly[113] == QPoint(221,221));
    QVERIFY(poly[114] == QPoint(225,225));
    QVERIFY(poly[115] == QPoint(229,229));
    QVERIFY(poly[116] == QPoint(232,232));
    QVERIFY(poly[117] == QPoint(235,235));
    QVERIFY(poly[118] == QPoint(239,239));
    QVERIFY(poly[119] == QPoint(241,241));
    QVERIFY(poly[120] == QPoint(244,244));
    QVERIFY(poly[121] == QPoint(246,246));
    QVERIFY(poly[122] == QPoint(248,248));
    QVERIFY(poly[123] == QPoint(250,250));
    QVERIFY(poly[124] == QPoint(251,251));
    QVERIFY(poly[125] == QPoint(252,252));
    QVERIFY(poly[126] == QPoint(253,253));
    QVERIFY(poly[127] == QPoint(253,253));
}

//void EFX_Test::previewTriangle()
//{
//}

void EFX_Test::previewDiamond()
{
    EFX e(m_doc);
    e.setAlgorithm("Diamond");

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    QVERIFY(poly[0] == QPoint(127,254));
    QVERIFY(poly[1] == QPoint(127,253));
    QVERIFY(poly[2] == QPoint(127,252));
    QVERIFY(poly[3] == QPoint(127,249));
    QVERIFY(poly[4] == QPoint(127,246));
    QVERIFY(poly[5] == QPoint(128,242));
    QVERIFY(poly[6] == QPoint(130,238));
    QVERIFY(poly[7] == QPoint(131,233));
    QVERIFY(poly[8] == QPoint(134,227));
    QVERIFY(poly[9] == QPoint(136,220));
    QVERIFY(poly[10] == QPoint(140,214));
    QVERIFY(poly[11] == QPoint(144,207));
    QVERIFY(poly[12] == QPoint(148,200));
    QVERIFY(poly[13] == QPoint(153,192));
    QVERIFY(poly[14] == QPoint(159,185));
    QVERIFY(poly[15] == QPoint(165,178));
    QVERIFY(poly[16] == QPoint(171,171));
    QVERIFY(poly[17] == QPoint(178,165));
    QVERIFY(poly[18] == QPoint(185,159));
    QVERIFY(poly[19] == QPoint(192,153));
    QVERIFY(poly[20] == QPoint(200,148));
    QVERIFY(poly[21] == QPoint(207,144));
    QVERIFY(poly[22] == QPoint(214,140));
    QVERIFY(poly[23] == QPoint(220,136));
    QVERIFY(poly[24] == QPoint(227,134));
    QVERIFY(poly[25] == QPoint(233,131));
    QVERIFY(poly[26] == QPoint(238,130));
    QVERIFY(poly[27] == QPoint(242,128));
    QVERIFY(poly[28] == QPoint(246,127));
    QVERIFY(poly[29] == QPoint(249,127));
    QVERIFY(poly[30] == QPoint(252,127));
    QVERIFY(poly[31] == QPoint(253,127));
    QVERIFY(poly[32] == QPoint(254,127));
    QVERIFY(poly[33] == QPoint(253,126));
    QVERIFY(poly[34] == QPoint(252,126));
    QVERIFY(poly[35] == QPoint(249,126));
    QVERIFY(poly[36] == QPoint(246,126));
    QVERIFY(poly[37] == QPoint(242,125));
    QVERIFY(poly[38] == QPoint(238,123));
    QVERIFY(poly[39] == QPoint(233,122));
    QVERIFY(poly[40] == QPoint(227,119));
    QVERIFY(poly[41] == QPoint(220,117));
    QVERIFY(poly[42] == QPoint(214,113));
    QVERIFY(poly[43] == QPoint(207,109));
    QVERIFY(poly[44] == QPoint(200,105));
    QVERIFY(poly[45] == QPoint(192,100));
    QVERIFY(poly[46] == QPoint(185,94));
    QVERIFY(poly[47] == QPoint(178,88));
    QVERIFY(poly[48] == QPoint(171,82));
    QVERIFY(poly[49] == QPoint(165,75));
    QVERIFY(poly[50] == QPoint(159,68));
    QVERIFY(poly[51] == QPoint(153,61));
    QVERIFY(poly[52] == QPoint(148,53));
    QVERIFY(poly[53] == QPoint(144,46));
    QVERIFY(poly[54] == QPoint(140,39));
    QVERIFY(poly[55] == QPoint(136,33));
    QVERIFY(poly[56] == QPoint(134,26));
    QVERIFY(poly[57] == QPoint(131,20));
    QVERIFY(poly[58] == QPoint(130,15));
    QVERIFY(poly[59] == QPoint(128,11));
    QVERIFY(poly[60] == QPoint(127,7));
    QVERIFY(poly[61] == QPoint(127,4));
    QVERIFY(poly[62] == QPoint(127,1));
    QVERIFY(poly[63] == QPoint(127,0));
    QVERIFY(poly[64] == QPoint(127,0));
    QVERIFY(poly[65] == QPoint(126,0));
    QVERIFY(poly[66] == QPoint(126,1));
    QVERIFY(poly[67] == QPoint(126,4));
    QVERIFY(poly[68] == QPoint(126,7));
    QVERIFY(poly[69] == QPoint(125,11));
    QVERIFY(poly[70] == QPoint(123,15));
    QVERIFY(poly[71] == QPoint(122,20));
    QVERIFY(poly[72] == QPoint(119,26));
    QVERIFY(poly[73] == QPoint(117,33));
    QVERIFY(poly[74] == QPoint(113,39));
    QVERIFY(poly[75] == QPoint(109,46));
    QVERIFY(poly[76] == QPoint(105,53));
    QVERIFY(poly[77] == QPoint(100,61));
    QVERIFY(poly[78] == QPoint(94,68));
    QVERIFY(poly[79] == QPoint(88,75));
    QVERIFY(poly[80] == QPoint(82,82));
    QVERIFY(poly[81] == QPoint(75,88));
    QVERIFY(poly[82] == QPoint(68,94));
    QVERIFY(poly[83] == QPoint(61,100));
    QVERIFY(poly[84] == QPoint(53,105));
    QVERIFY(poly[85] == QPoint(46,109));
    QVERIFY(poly[86] == QPoint(39,113));
    QVERIFY(poly[87] == QPoint(33,117));
    QVERIFY(poly[88] == QPoint(26,119));
    QVERIFY(poly[89] == QPoint(20,122));
    QVERIFY(poly[90] == QPoint(15,123));
    QVERIFY(poly[91] == QPoint(11,125));
    QVERIFY(poly[92] == QPoint(7,126));
    QVERIFY(poly[93] == QPoint(4,126));
    QVERIFY(poly[94] == QPoint(1,126));
    QVERIFY(poly[95] == QPoint(0,126));
    QVERIFY(poly[96] == QPoint(0,127));
    QVERIFY(poly[97] == QPoint(0,127));
    QVERIFY(poly[98] == QPoint(1,127));
    QVERIFY(poly[99] == QPoint(4,127));
    QVERIFY(poly[100] == QPoint(7,127));
    QVERIFY(poly[101] == QPoint(11,128));
    QVERIFY(poly[102] == QPoint(15,130));
    QVERIFY(poly[103] == QPoint(20,131));
    QVERIFY(poly[104] == QPoint(26,134));
    QVERIFY(poly[105] == QPoint(33,136));
    QVERIFY(poly[106] == QPoint(39,140));
    QVERIFY(poly[107] == QPoint(46,144));
    QVERIFY(poly[108] == QPoint(53,148));
    QVERIFY(poly[109] == QPoint(61,153));
    QVERIFY(poly[110] == QPoint(68,159));
    QVERIFY(poly[111] == QPoint(75,165));
    QVERIFY(poly[112] == QPoint(82,171));
    QVERIFY(poly[113] == QPoint(88,178));
    QVERIFY(poly[114] == QPoint(94,185));
    QVERIFY(poly[115] == QPoint(100,192));
    QVERIFY(poly[116] == QPoint(105,200));
    QVERIFY(poly[117] == QPoint(109,207));
    QVERIFY(poly[118] == QPoint(113,214));
    QVERIFY(poly[119] == QPoint(117,220));
    QVERIFY(poly[120] == QPoint(119,227));
    QVERIFY(poly[121] == QPoint(122,233));
    QVERIFY(poly[122] == QPoint(123,238));
    QVERIFY(poly[123] == QPoint(125,242));
    QVERIFY(poly[124] == QPoint(126,246));
    QVERIFY(poly[125] == QPoint(126,249));
    QVERIFY(poly[126] == QPoint(126,252));
    QVERIFY(poly[127] == QPoint(126,253));
}

void EFX_Test::previewLissajous()
{
    EFX e(m_doc);
    e.setAlgorithm("Lissajous");

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    QVERIFY(poly[0] == QPoint(127,254));
    QVERIFY(poly[1] == QPoint(139,252));
    QVERIFY(poly[2] == QPoint(151,248));
    QVERIFY(poly[3] == QPoint(163,241));
    QVERIFY(poly[4] == QPoint(175,232));
    QVERIFY(poly[5] == QPoint(186,221));
    QVERIFY(poly[6] == QPoint(197,207));
    QVERIFY(poly[7] == QPoint(207,192));
    QVERIFY(poly[8] == QPoint(216,175));
    QVERIFY(poly[9] == QPoint(225,157));
    QVERIFY(poly[10] == QPoint(232,139));
    QVERIFY(poly[11] == QPoint(239,120));
    QVERIFY(poly[12] == QPoint(244,102));
    QVERIFY(poly[13] == QPoint(248,84));
    QVERIFY(poly[14] == QPoint(251,67));
    QVERIFY(poly[15] == QPoint(253,51));
    QVERIFY(poly[16] == QPoint(254,37));
    QVERIFY(poly[17] == QPoint(253,24));
    QVERIFY(poly[18] == QPoint(251,14));
    QVERIFY(poly[19] == QPoint(248,7));
    QVERIFY(poly[20] == QPoint(244,2));
    QVERIFY(poly[21] == QPoint(239,0));
    QVERIFY(poly[22] == QPoint(232,0));
    QVERIFY(poly[23] == QPoint(225,3));
    QVERIFY(poly[24] == QPoint(216,9));
    QVERIFY(poly[25] == QPoint(207,18));
    QVERIFY(poly[26] == QPoint(197,28));
    QVERIFY(poly[27] == QPoint(186,41));
    QVERIFY(poly[28] == QPoint(175,56));
    QVERIFY(poly[29] == QPoint(163,72));
    QVERIFY(poly[30] == QPoint(151,90));
    QVERIFY(poly[31] == QPoint(139,108));
    QVERIFY(poly[32] == QPoint(126,127));
    QVERIFY(poly[33] == QPoint(114,145));
    QVERIFY(poly[34] == QPoint(102,163));
    QVERIFY(poly[35] == QPoint(90,181));
    QVERIFY(poly[36] == QPoint(78,197));
    QVERIFY(poly[37] == QPoint(67,212));
    QVERIFY(poly[38] == QPoint(56,225));
    QVERIFY(poly[39] == QPoint(46,235));
    QVERIFY(poly[40] == QPoint(37,244));
    QVERIFY(poly[41] == QPoint(28,250));
    QVERIFY(poly[42] == QPoint(21,253));
    QVERIFY(poly[43] == QPoint(14,253));
    QVERIFY(poly[44] == QPoint(9,251));
    QVERIFY(poly[45] == QPoint(5,246));
    QVERIFY(poly[46] == QPoint(2,239));
    QVERIFY(poly[47] == QPoint(0,229));
    QVERIFY(poly[48] == QPoint(0,216));
    QVERIFY(poly[49] == QPoint(0,202));
    QVERIFY(poly[50] == QPoint(2,186));
    QVERIFY(poly[51] == QPoint(5,169));
    QVERIFY(poly[52] == QPoint(9,151));
    QVERIFY(poly[53] == QPoint(14,133));
    QVERIFY(poly[54] == QPoint(21,114));
    QVERIFY(poly[55] == QPoint(28,96));
    QVERIFY(poly[56] == QPoint(37,78));
    QVERIFY(poly[57] == QPoint(46,61));
    QVERIFY(poly[58] == QPoint(56,46));
    QVERIFY(poly[59] == QPoint(67,32));
    QVERIFY(poly[60] == QPoint(78,21));
    QVERIFY(poly[61] == QPoint(90,12));
    QVERIFY(poly[62] == QPoint(102,5));
    QVERIFY(poly[63] == QPoint(114,1));
    QVERIFY(poly[64] == QPoint(126,0));
    QVERIFY(poly[65] == QPoint(139,1));
    QVERIFY(poly[66] == QPoint(151,5));
    QVERIFY(poly[67] == QPoint(163,12));
    QVERIFY(poly[68] == QPoint(175,21));
    QVERIFY(poly[69] == QPoint(186,32));
    QVERIFY(poly[70] == QPoint(197,46));
    QVERIFY(poly[71] == QPoint(207,61));
    QVERIFY(poly[72] == QPoint(216,78));
    QVERIFY(poly[73] == QPoint(225,96));
    QVERIFY(poly[74] == QPoint(232,114));
    QVERIFY(poly[75] == QPoint(239,133));
    QVERIFY(poly[76] == QPoint(244,151));
    QVERIFY(poly[77] == QPoint(248,169));
    QVERIFY(poly[78] == QPoint(251,186));
    QVERIFY(poly[79] == QPoint(253,202));
    QVERIFY(poly[80] == QPoint(254,216));
    QVERIFY(poly[81] == QPoint(253,229));
    QVERIFY(poly[82] == QPoint(251,239));
    QVERIFY(poly[83] == QPoint(248,246));
    QVERIFY(poly[84] == QPoint(244,251));
    QVERIFY(poly[85] == QPoint(239,253));
    QVERIFY(poly[86] == QPoint(232,253));
    QVERIFY(poly[87] == QPoint(225,250));
    QVERIFY(poly[88] == QPoint(216,244));
    QVERIFY(poly[89] == QPoint(207,235));
    QVERIFY(poly[90] == QPoint(197,225));
    QVERIFY(poly[91] == QPoint(186,212));
    QVERIFY(poly[92] == QPoint(175,197));
    QVERIFY(poly[93] == QPoint(163,181));
    QVERIFY(poly[94] == QPoint(151,163));
    QVERIFY(poly[95] == QPoint(139,145));
    QVERIFY(poly[96] == QPoint(127,127));
    QVERIFY(poly[97] == QPoint(114,108));
    QVERIFY(poly[98] == QPoint(102,90));
    QVERIFY(poly[99] == QPoint(90,72));
    QVERIFY(poly[100] == QPoint(78,56));
    QVERIFY(poly[101] == QPoint(67,41));
    QVERIFY(poly[102] == QPoint(56,28));
    QVERIFY(poly[103] == QPoint(46,18));
    QVERIFY(poly[104] == QPoint(37,9));
    QVERIFY(poly[105] == QPoint(28,3));
    QVERIFY(poly[106] == QPoint(21,0));
    QVERIFY(poly[107] == QPoint(14,0));
    QVERIFY(poly[108] == QPoint(9,2));
    QVERIFY(poly[109] == QPoint(5,7));
    QVERIFY(poly[110] == QPoint(2,14));
    QVERIFY(poly[111] == QPoint(0,24));
    QVERIFY(poly[112] == QPoint(0,37));
    QVERIFY(poly[113] == QPoint(0,51));
    QVERIFY(poly[114] == QPoint(2,67));
    QVERIFY(poly[115] == QPoint(5,84));
    QVERIFY(poly[116] == QPoint(9,102));
    QVERIFY(poly[117] == QPoint(14,120));
    QVERIFY(poly[118] == QPoint(21,139));
    QVERIFY(poly[119] == QPoint(28,157));
    QVERIFY(poly[120] == QPoint(37,175));
    QVERIFY(poly[121] == QPoint(46,192));
    QVERIFY(poly[122] == QPoint(56,207));
    QVERIFY(poly[123] == QPoint(67,221));
    QVERIFY(poly[124] == QPoint(78,232));
    QVERIFY(poly[125] == QPoint(90,241));
    QVERIFY(poly[126] == QPoint(102,248));
    QVERIFY(poly[127] == QPoint(114,252));
}

void EFX_Test::widthHeightOffset()
{
    EFX e(m_doc);
    int i = 0;
    int max = 0;

    QVector <QPoint> poly;
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    /* Check that width affects the pattern */
    e.setWidth(50);
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    /* Width of 50 means actually 50px left of center (127-50) and
       50px right of center (127+50). +1 because the bound coordinates
       are OUTSIDE the actual points. */
    QVERIFY(QPolygon(poly).boundingRect().width() == 50 + 50 + 1);
    QVERIFY(QPolygon(poly).boundingRect().height() == UCHAR_MAX);

    /* Check that height affects the pattern */
    e.setHeight(87);
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);

    /* Height of 87 means actually 87px down of center (127-87) and
       87px up of center (127+87). And +1 because the bound coordinates
       are OUTSIDE the actual points. */
    QVERIFY(QPolygon(poly).boundingRect().height() == 87 + 87 + 1);
    QVERIFY(QPolygon(poly).boundingRect().width() == 100 + 1);

    /* X Offset is at center */
    max = 0;
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].x() > max)
            max = poly[i].x();
    QVERIFY(max == 177);

    /* X offset + 20 */
    max = 0;
    e.setXOffset(127 + 20);
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].x() > max)
            max = poly[i].x();
    QVERIFY(max == 197);

    /* Y Offset is at center */
    max = 0;
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].y() > max)
            max = poly[i].y();
    QVERIFY(max == 214);

    /* Y offset - 25 */
    max = 0;
    e.setYOffset(127 - 25);
    poly.clear();
    QVERIFY(e.preview(poly) == true);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].y() > max)
            max = poly[i].y();
    QVERIFY(max == 189);
}

void EFX_Test::rotateAndScale()
{
    /* The X & Y params used here represent the first point in a circle
       algorithm as calculated by EFX::circlePoint(), based on iterator
       value 0, before calling rotateAndScale(). */
    qreal x, y;

    /* Rotation */
    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 127, 0);
    QVERIFY(floor(x + 0.5) == 111);
    QVERIFY(floor(y + 0.5) == 253);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 253);
    QVERIFY(floor(y + 0.5) == 143);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 127, 180);
    QVERIFY(floor(x + 0.5) == 143);
    QVERIFY(floor(y + 0.5) == 1);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 127, 270);
    QVERIFY(floor(x + 0.5) == 1);
    QVERIFY(floor(y + 0.5) == 111);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 127, 45);
    QVERIFY(floor(x + 0.5) == 205);
    QVERIFY(floor(y + 0.5) == 227);

    /* Offset */
    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 128, 128, 0);
    QVERIFY(floor(x + 0.5) == 112);
    QVERIFY(floor(y + 0.5) == 254);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 0, 0, 0);
    QVERIFY(floor(x + 0.5) == -16);
    QVERIFY(floor(y + 0.5) == 126);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 0, 127, 0);
    QVERIFY(floor(x + 0.5) == -16);
    QVERIFY(floor(y + 0.5) == 253);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 127, 0, 0);
    QVERIFY(floor(x + 0.5) == 111);
    QVERIFY(floor(y + 0.5) == 126);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 127, 1, UCHAR_MAX, 0);
    QVERIFY(floor(x + 0.5) == -15);
    QVERIFY(floor(y + 0.5) == 381);

    /* Width & height (rotate also by 90 degrees to get more tangible
       results, because x&y point to the bottom-center, where width & height
       params have very little effect without offset or rotation). */
    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 64, 64, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 190);
    QVERIFY(floor(y + 0.5) == 135);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 10, 127, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 137);
    QVERIFY(floor(y + 0.5) == 143);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 64, 127, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 190);
    QVERIFY(floor(y + 0.5) == 143);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 64, 0, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 190);
    QVERIFY(floor(y + 0.5) == 127);

    x = -0.125333;
    y = 0.992115;
    EFX::rotateAndScale(&x, &y, 127, 0, 127, 127, 90);
    QVERIFY(floor(x + 0.5) == 253);
    QVERIFY(floor(y + 0.5) == 127);
}

void EFX_Test::copyFrom()
{
    EFX e1(m_doc);
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
    EFX e2(m_doc);
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
    Scene s(m_doc);
    QVERIFY(e2.copyFrom(&s) == false);

    /* Make a third EFX */
    EFX e3(m_doc);
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

    EFX* e1 = new EFX(m_doc);
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
    QVERIFY(e1->id() != Function::invalidId());

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

    EFX e(m_doc);
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

    EFX e(m_doc);
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

    EFX e(m_doc);
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

    EFX e(m_doc);
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

    EFX e(m_doc);
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

    EFX e(m_doc);
    QVERIFY(e.loadXML(&root) == false);
}

void EFX_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("EFX");
    root.setAttribute("Type", "EFX");

    EFX e(m_doc);
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

    EFX e(m_doc);
    QVERIFY(e.loadXML(&root) == true);
    QVERIFY(e.fixtures().size() == 1);
    QVERIFY(e.fixtures().at(0)->direction() == EFX::Forward);
}

void EFX_Test::save()
{
    EFX e1(m_doc);
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
    QVERIFY(dir == true);
    QVERIFY(run == true);
    QVERIFY(bus == true);
    QVERIFY(algo == true);
    QVERIFY(w == true);
    QVERIFY(h == true);
    QVERIFY(rot == true);
    QVERIFY(xoff == true);
    QVERIFY(yoff == true);
    QVERIFY(xfreq == true);
    QVERIFY(yfreq == true);
    QVERIFY(xpha == true);
    QVERIFY(ypha == true);
    QVERIFY(prop == true);
    QVERIFY(stas == true);
    QVERIFY(stos == true);
}

void EFX_Test::armSuccess()
{
    Doc* doc = new Doc(this, m_cache);

    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    const QLCFixtureDef* def = m_cache.fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    doc->addFixture(fxi2);

    Scene* s1 = new Scene(doc);
    s1->setName("INIT");
    s1->setValue(fxi1->id(), 0, 205);// Shutter open
    s1->setValue(fxi2->id(), 0, 205);// Shutter open
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("DEINIT");
    s2->setValue(fxi1->id(), 0, 0);// Shutter closed
    s2->setValue(fxi2->id(), 0, 0);// Shutter closed
    doc->addFunction(s2);

    EFX* e = new EFX(doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setFixture(fxi1->id());
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setFixture(fxi2->id());
    e->addFixture(ef2);

    e->setStartScene(s1->id());
    e->setStartSceneEnabled(true);
    e->setStopScene(s2->id());
    e->setStopSceneEnabled(true);

    e->arm();

    QVERIFY(e->pointFunc == e->circlePoint);

    QVERIFY(e->m_fixtures.size() == 2);

    QVERIFY(e->m_fixtures.at(0)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(0)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(0)->m_serialNumber == 0);
    QVERIFY(e->m_fixtures.at(0)->m_msbPanChannel == 0 + 7);
    QVERIFY(e->m_fixtures.at(0)->m_msbTiltChannel == 0 + 9);
    QVERIFY(e->m_fixtures.at(0)->m_lsbPanChannel == 0 + 8);
    QVERIFY(e->m_fixtures.at(0)->m_lsbTiltChannel == 0 + 10);

    QVERIFY(e->m_fixtures.at(1)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(1)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(1)->m_serialNumber == 1);
    QVERIFY(e->m_fixtures.at(1)->m_msbPanChannel == 512 + 7);
    QVERIFY(e->m_fixtures.at(1)->m_msbTiltChannel == 512 + 9);
    QVERIFY(e->m_fixtures.at(1)->m_lsbPanChannel == 512 + 8);
    QVERIFY(e->m_fixtures.at(1)->m_lsbTiltChannel == 512 + 10);

    delete doc;
}

void EFX_Test::armMissingStartScene()
{
    Doc* doc = new Doc(this, m_cache);

    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    const QLCFixtureDef* def = m_cache.fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    doc->addFixture(fxi2);

    Scene* s1 = new Scene(doc);
    s1->setName("INIT");
    s1->setValue(fxi1->id(), 0, 205);// Shutter open
    s1->setValue(fxi2->id(), 0, 205);// Shutter open
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("DEINIT");
    s2->setValue(fxi1->id(), 0, 0);// Shutter closed
    s2->setValue(fxi2->id(), 0, 0);// Shutter closed
    doc->addFunction(s2);

    EFX* e = new EFX(doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setFixture(fxi1->id());
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setFixture(fxi2->id());
    e->addFixture(ef2);

    e->setStartScene(42);
    e->setStartSceneEnabled(true);
    e->setStopScene(s2->id());
    e->setStopSceneEnabled(true);

    e->arm();

    QVERIFY(e->pointFunc == e->circlePoint);

    QVERIFY(e->m_fixtures.size() == 2);

    QVERIFY(e->m_fixtures.at(0)->m_startScene == NULL);
    QVERIFY(e->m_fixtures.at(0)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(0)->m_serialNumber == 0);
    QVERIFY(e->m_fixtures.at(0)->m_msbPanChannel == 0 + 7);
    QVERIFY(e->m_fixtures.at(0)->m_msbTiltChannel == 0 + 9);
    QVERIFY(e->m_fixtures.at(0)->m_lsbPanChannel == 0 + 8);
    QVERIFY(e->m_fixtures.at(0)->m_lsbTiltChannel == 0 + 10);

    QVERIFY(e->m_fixtures.at(1)->m_startScene == NULL);
    QVERIFY(e->m_fixtures.at(1)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(1)->m_serialNumber == 1);
    QVERIFY(e->m_fixtures.at(1)->m_msbPanChannel == 512 + 7);
    QVERIFY(e->m_fixtures.at(1)->m_msbTiltChannel == 512 + 9);
    QVERIFY(e->m_fixtures.at(1)->m_lsbPanChannel == 512 + 8);
    QVERIFY(e->m_fixtures.at(1)->m_lsbTiltChannel == 512 + 10);

    delete doc;
}

void EFX_Test::armMissingStopScene()
{
    Doc* doc = new Doc(this, m_cache);

    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    const QLCFixtureDef* def = m_cache.fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    doc->addFixture(fxi2);

    Scene* s1 = new Scene(doc);
    s1->setName("INIT");
    s1->setValue(fxi1->id(), 0, 205);// Shutter open
    s1->setValue(fxi2->id(), 0, 205);// Shutter open
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("DEINIT");
    s2->setValue(fxi1->id(), 0, 0);// Shutter closed
    s2->setValue(fxi2->id(), 0, 0);// Shutter closed
    doc->addFunction(s2);

    EFX* e = new EFX(doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setFixture(fxi1->id());
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setFixture(fxi2->id());
    e->addFixture(ef2);

    e->setStartScene(s1->id());
    e->setStartSceneEnabled(true);
    e->setStopScene(42);
    e->setStopSceneEnabled(true);

    e->arm();

    QVERIFY(e->pointFunc == e->circlePoint);

    QVERIFY(e->m_fixtures.size() == 2);

    QVERIFY(e->m_fixtures.at(0)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(0)->m_stopScene == NULL);
    QVERIFY(e->m_fixtures.at(0)->m_serialNumber == 0);
    QVERIFY(e->m_fixtures.at(0)->m_msbPanChannel == 0 + 7);
    QVERIFY(e->m_fixtures.at(0)->m_msbTiltChannel == 0 + 9);
    QVERIFY(e->m_fixtures.at(0)->m_lsbPanChannel == 0 + 8);
    QVERIFY(e->m_fixtures.at(0)->m_lsbTiltChannel == 0 + 10);

    QVERIFY(e->m_fixtures.at(1)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(1)->m_stopScene == NULL);
    QVERIFY(e->m_fixtures.at(1)->m_serialNumber == 1);
    QVERIFY(e->m_fixtures.at(1)->m_msbPanChannel == 512 + 7);
    QVERIFY(e->m_fixtures.at(1)->m_msbTiltChannel == 512 + 9);
    QVERIFY(e->m_fixtures.at(1)->m_lsbPanChannel == 512 + 8);
    QVERIFY(e->m_fixtures.at(1)->m_lsbTiltChannel == 512 + 10);

    delete doc;
}

void EFX_Test::armMissingFixture()
{
    Doc* doc = new Doc(this, m_cache);

    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    const QLCFixtureDef* def = m_cache.fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    doc->addFixture(fxi2);

    Scene* s1 = new Scene(doc);
    s1->setName("INIT");
    s1->setValue(fxi1->id(), 0, 205);// Shutter open
    s1->setValue(fxi2->id(), 0, 205);// Shutter open
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("DEINIT");
    s2->setValue(fxi1->id(), 0, 0);// Shutter closed
    s2->setValue(fxi2->id(), 0, 0);// Shutter closed
    doc->addFunction(s2);

    EFX* e = new EFX(doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setFixture(fxi1->id());
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setFixture(42);
    e->addFixture(ef2);

    e->setStartScene(s1->id());
    e->setStartSceneEnabled(true);
    e->setStopScene(s2->id());
    e->setStopSceneEnabled(true);

    e->arm();

    QVERIFY(e->pointFunc == e->circlePoint);

    QVERIFY(e->m_fixtures.size() == 2);

    QVERIFY(e->m_fixtures.at(0)->fixture() == fxi1->id());
    QVERIFY(e->m_fixtures.at(0)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(0)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(0)->m_serialNumber == 0);
    QVERIFY(e->m_fixtures.at(0)->m_msbPanChannel == 0 + 7);
    QVERIFY(e->m_fixtures.at(0)->m_msbTiltChannel == 0 + 9);
    QVERIFY(e->m_fixtures.at(0)->m_lsbPanChannel == 0 + 8);
    QVERIFY(e->m_fixtures.at(0)->m_lsbTiltChannel == 0 + 10);

    QVERIFY(e->m_fixtures.at(1)->fixture() == 42);
    QVERIFY(e->m_fixtures.at(1)->m_startScene == s1);
    QVERIFY(e->m_fixtures.at(1)->m_stopScene == s2);
    QVERIFY(e->m_fixtures.at(1)->m_serialNumber == 1);
    QVERIFY(e->m_fixtures.at(1)->m_msbPanChannel == KChannelInvalid);
    QVERIFY(e->m_fixtures.at(1)->m_msbTiltChannel == KChannelInvalid);
    QVERIFY(e->m_fixtures.at(1)->m_lsbPanChannel == KChannelInvalid);
    QVERIFY(e->m_fixtures.at(1)->m_lsbTiltChannel == KChannelInvalid);

    delete doc;
}

void EFX_Test::writeStartStopScenes()
{
    Doc* doc = new Doc(this, m_cache);

    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    const QLCFixtureDef* def = m_cache.fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    const QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    doc->addFixture(fxi2);

    Scene* s1 = new Scene(doc);
    s1->setName("INIT");
    s1->setValue(fxi1->id(), 0, 205);// Shutter open
    s1->setValue(fxi2->id(), 0, 205);// Shutter open
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("DEINIT");
    s2->setValue(fxi1->id(), 0, 0);// Shutter closed
    s2->setValue(fxi2->id(), 0, 0);// Shutter closed
    doc->addFunction(s2);

    EFX* e = new EFX(doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setFixture(fxi1->id());
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setFixture(fxi2->id());
    e->addFixture(ef2);

    e->setStartScene(s1->id());
    e->setStartSceneEnabled(true);
    e->setStopScene(s2->id());
    e->setStopSceneEnabled(true);

    e->arm();
    s1->arm();
    s2->arm();

    Bus::instance()->setValue(0, 50);

    QByteArray unis(512 * 4, 0);
    OutputMapStub* oms = new OutputMapStub(this);
    oms->setUniverses(&unis);
    MasterTimerStub* mts = new MasterTimerStub(this, oms, unis);

    QVERIFY(e->stopped() == true);
    mts->startFunction(e, false);
    QVERIFY(e->stopped() == false);

    e->write(mts, &unis);
    QVERIFY(e->stopped() == false);
    QVERIFY(unis[0] == (char) 205); // Start scene: shutter open
    QVERIFY(unis[512 + 0] == (char) 205); // Start scene: shutter open

    e->stop();
    mts->stopFunction(e); // Runs postRun, that writes stop scene stuff
    QVERIFY(unis[0] == (char) 0); // Stop scene: shutter closed
    QVERIFY(unis[512 + 0] == (char) 0); // Stop scene: shutter closed

    delete doc;
}

