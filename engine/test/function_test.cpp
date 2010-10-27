/*
  Q Light Controller - Unit test
  function_test.cpp

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

#include "qlcfixturedefcache.h"
#include "function_test.h"

#define protected public
#include "function_stub.h"
#undef protected

#include "doc.h"

void Function_Test::initTestCase()
{
    Bus::init(this);
}

void Function_Test::initial()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->name(), QString());
    QCOMPARE(stub->runOrder(), Function::Loop);
    QCOMPARE(stub->direction(), Function::Forward);
    QCOMPARE(stub->busID(), Bus::defaultFade());
    QCOMPARE(stub->elapsed(), quint32(0));
    QCOMPARE(stub->stopped(), true);
}

void Function_Test::copyFrom()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub1 = new Function_Stub(&doc);
    QVERIFY(stub1->copyFrom(NULL) == false);
    stub1->setName("Stub1");
    stub1->setRunOrder(Function::PingPong);
    stub1->setDirection(Function::Backward);
    stub1->setBus(15);

    Function_Stub* stub2 = new Function_Stub(&doc);
    stub2->copyFrom(stub1);
    QCOMPARE(stub1->name(), stub2->name());
    QCOMPARE(stub1->runOrder(), stub2->runOrder());
    QCOMPARE(stub1->direction(), stub2->direction());
    QCOMPARE(stub1->busID(), stub2->busID());
}

void Function_Test::flashUnflash()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    QSignalSpy spy(stub, SIGNAL(flashing(t_function_id,bool)));

    QVERIFY(stub->flashing() == false);
    stub->flash(NULL);
    QCOMPARE(spy.size(), 1);
    QVERIFY(stub->flashing() == true);
    stub->flash(NULL);
    QCOMPARE(spy.size(), 1);
    QVERIFY(stub->flashing() == true);
    stub->unFlash(NULL);
    QCOMPARE(spy.size(), 2);
    QVERIFY(stub->flashing() == false);
}

void Function_Test::elapsed()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->elapsed(), quint32(0));
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), quint32(1));
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), quint32(2));
    stub->incrementElapsed();
    QCOMPARE(stub->elapsed(), quint32(3));
    stub->resetElapsed();
    QCOMPARE(stub->elapsed(), quint32(0));
}

void Function_Test::preRunPostRun()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    QSignalSpy spyRunning(stub, SIGNAL(running(t_function_id)));
    stub->preRun(NULL);
    QVERIFY(stub->stopped() == false);
    QCOMPARE(spyRunning.size(), 1);
    // @todo Check the contents of the signal in spyRunning

    stub->incrementElapsed();

    QSignalSpy spyStopped(stub, SIGNAL(stopped(t_function_id)));
    stub->postRun(NULL, NULL);
    QVERIFY(stub->stopped() == true);
    QCOMPARE(stub->elapsed(), quint32(0));
    QCOMPARE(spyRunning.size(), 1);
    QCOMPARE(spyStopped.size(), 1);
    // @todo Check the contents of the signal in spyStopped
}

void Function_Test::stopAndWait()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    stub->preRun(NULL);
    stub->incrementElapsed();

    // @todo Make stopAndWait() return before the 2s watchdog timer
    //QSignalSpy spyStopped(stub, SIGNAL(stopped(t_function_id)));
    //QVERIFY(stub->stopAndWait() == true);
}

void Function_Test::stopAndWaitFail()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    stub->preRun(NULL);
    stub->incrementElapsed();

    QSignalSpy spyStopped(stub, SIGNAL(stopped(t_function_id)));
    QVERIFY(stub->stopAndWait() == false);
}

void Function_Test::slotFixtureRemoved()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    Fixture* fxi = new Fixture(&doc);
    fxi->setID(42);
    QVERIFY(doc.addFixture(fxi, fxi->id()) == true);
    QVERIFY(doc.addFunction(stub) == true);

    QCOMPARE(stub->m_slotFixtureRemovedId, Fixture::invalidId());
    doc.deleteFixture(42);
    QCOMPARE(stub->m_slotFixtureRemovedId, 42);
}

void Function_Test::invalidId()
{
    QCOMPARE(Function::invalidId(), -1);
}

void Function_Test::typeString()
{
    QLCFixtureDefCache cache;
    Doc doc(this, cache);

    Function_Stub* stub = new Function_Stub(&doc);
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Type(31337)));
    stub->m_type = Function::Scene;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Scene));
    stub->m_type = Function::Chaser;
    QCOMPARE(stub->typeString(), Function::typeToString(Function::Chaser));
}

void Function_Test::typeToString()
{
    QVERIFY(Function::typeToString(Function::Undefined) == "Undefined");
    QVERIFY(Function::typeToString(Function::Scene) == "Scene");
    QVERIFY(Function::typeToString(Function::Chaser) == "Chaser");
    QVERIFY(Function::typeToString(Function::EFX) == "EFX");
    QVERIFY(Function::typeToString(Function::Collection) == "Collection");

    QVERIFY(Function::typeToString(Function::Type(42)) == "Undefined");
    QVERIFY(Function::typeToString(Function::Type(31337)) == "Undefined");
}

void Function_Test::stringToType()
{
    QVERIFY(Function::stringToType("Undefined") == Function::Undefined);
    QVERIFY(Function::stringToType("Scene") == Function::Scene);
    QVERIFY(Function::stringToType("Chaser") == Function::Chaser);
    QVERIFY(Function::stringToType("EFX") == Function::EFX);
    QVERIFY(Function::stringToType("Collection") == Function::Collection);

    QVERIFY(Function::stringToType("Foobar") == Function::Undefined);
    QVERIFY(Function::stringToType("Xyzzy") == Function::Undefined);
}

void Function_Test::runOrderToString()
{
    QVERIFY(Function::runOrderToString(Function::Loop) == "Loop");
    QVERIFY(Function::runOrderToString(Function::SingleShot) == "SingleShot");
    QVERIFY(Function::runOrderToString(Function::PingPong) == "PingPong");

    QVERIFY(Function::runOrderToString(Function::RunOrder(42)) == "Loop");
    QVERIFY(Function::runOrderToString(Function::RunOrder(69)) == "Loop");
}

void Function_Test::stringToRunOrder()
{
    QVERIFY(Function::stringToRunOrder("Loop") == Function::Loop);
    QVERIFY(Function::stringToRunOrder("SingleShot") == Function::SingleShot);
    QVERIFY(Function::stringToRunOrder("PingPong") == Function::PingPong);

    QVERIFY(Function::stringToRunOrder("Foobar") == Function::Loop);
    QVERIFY(Function::stringToRunOrder("Xyzzy") == Function::Loop);
}

void Function_Test::directionToString()
{
    QVERIFY(Function::directionToString(Function::Forward) == "Forward");
    QVERIFY(Function::directionToString(Function::Backward) == "Backward");

    QVERIFY(Function::directionToString(Function::Direction(42)) == "Forward");
    QVERIFY(Function::directionToString(Function::Direction(69)) == "Forward");
}

void Function_Test::stringToDirection()
{
    QVERIFY(Function::stringToDirection("Forward") == Function::Forward);
    QVERIFY(Function::stringToDirection("Backward") == Function::Backward);

    QVERIFY(Function::stringToDirection("Foobar") == Function::Forward);
    QVERIFY(Function::stringToDirection("Xyzzy") == Function::Forward);
}

void Function_Test::loaderWrongRoot()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;
    QDomElement root = doc.createElement("Scene");

    QVERIFY(Function::loader(&root, &d) == false);
    QVERIFY(d.functions() == 0);
}

void Function_Test::loaderWrongID()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("ID", QString("%1").arg(KFunctionArraySize));

    QVERIFY(Function::loader(&root, &d) == false);
    QVERIFY(d.functions() == 0);

    root.setAttribute("ID", "-4");
    QVERIFY(Function::loader(&root, &d) == false);
    QVERIFY(d.functions() == 0);
}

void Function_Test::loaderScene()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Scene");
    root.setAttribute("ID", 15);
    root.setAttribute("Name", "Lipton");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("5");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement v2 = doc.createElement("Value");
    v2.setAttribute("Fixture", 133);
    v2.setAttribute("Channel", 4);
    QDomText v2Text = doc.createTextNode("59");
    v2.appendChild(v2Text);
    root.appendChild(v2);

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(&root, &d) == true);
    QVERIFY(d.functions() == 1);
    QVERIFY(d.function(15) != NULL);
    QVERIFY(d.function(15)->type() == Function::Scene);
    QVERIFY(d.function(15)->name() == QString("Lipton"));
}

void Function_Test::loaderChaser()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");
    root.setAttribute("ID", 1);
    root.setAttribute("Name", "Malarkey");

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

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(&root, &d) == true);
    QVERIFY(d.functions() == 1);
    QVERIFY(d.function(1) != NULL);
    QVERIFY(d.function(1)->type() == Function::Chaser);
    QVERIFY(d.function(1)->name() == QString("Malarkey"));
}

void Function_Test::loaderCollection()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Collection");
    root.setAttribute("ID", "120");
    root.setAttribute("Name", "Spiers");

    QDomElement s3 = doc.createElement("Step");
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(&root, &d) == true);
    QVERIFY(d.functions() == 1);
    QVERIFY(d.function(120) != NULL);
    QVERIFY(d.function(120)->type() == Function::Collection);
    QVERIFY(d.function(120)->name() == QString("Spiers"));
}

void Function_Test::loaderEFX()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "EFX");
    root.setAttribute("Name", "Guarnere");
    root.setAttribute("ID", "0");

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

    /* Just verify that a Chaser function gets loaded. The rest of Chaser
       loading is tested in Chaser_test. */
    QVERIFY(Function::loader(&root, &d) == true);
    QVERIFY(d.functions() == 1);
    QVERIFY(d.function(0) != NULL);
    QVERIFY(d.function(0)->type() == Function::EFX);
    QVERIFY(d.function(0)->name() == QString("Guarnere"));
}

void Function_Test::loaderUnknownType()
{
    QLCFixtureDefCache cache;
    Doc d(this, cache);

    QDomDocument doc;
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Major");
    root.setAttribute("ID", 15);
    root.setAttribute("Name", "Winters");

    /* Just verify that a Scene function gets loaded. The rest of Scene
       loading is tested in Scene_test. */
    QVERIFY(Function::loader(&root, &d) == false);
    QVERIFY(d.functions() == 0);
    QVERIFY(d.function(15) == NULL);
}
