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
#include "function.h"
#include "doc.h"

void Function_Test::initTestCase()
{
    Bus::init(this);
}

void Function_Test::invalidId()
{
    QVERIFY(Function::invalidId() == -1);
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

