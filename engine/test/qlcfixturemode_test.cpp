/*
  Q Light Controller - Unit tests
  qlcfixturemode_test.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#include <QtTest>
#include <QtXml>

#include "qlcfixturemode_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

void QLCFixtureMode_Test::initTestCase()
{
    m_fixtureDef = new QLCFixtureDef();
    QVERIFY(m_fixtureDef != NULL);

    m_ch1 = new QLCChannel();
    m_ch1->setName("Channel 1");
    m_fixtureDef->addChannel(m_ch1);

    m_ch2 = new QLCChannel();
    m_ch2->setName("Channel 2");
    m_fixtureDef->addChannel(m_ch2);

    m_ch3 = new QLCChannel();
    m_ch3->setName("Channel 3");
    m_fixtureDef->addChannel(m_ch3);

    m_ch4 = new QLCChannel();
    m_ch4->setName("Channel 4");
    m_fixtureDef->addChannel(m_ch4);
}

void QLCFixtureMode_Test::name()
{
    /* Verify that a name can be set & get for the mode */
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    QVERIFY(mode->name() == QString::null);
    mode->setName("Normal");
    QVERIFY(mode->name() == "Normal");

    delete mode;
}

void QLCFixtureMode_Test::physical()
{
    /* Verify that a QLCPhysical can be set & get for the mode */
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    QVERIFY(mode->physical().bulbType() == QString::null);

    QLCPhysical p;
    p.setBulbType("Foobar");
    mode->setPhysical(p);
    QVERIFY(mode->physical().bulbType() == "Foobar");

    delete mode;
}

void QLCFixtureMode_Test::insertChannel()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    mode->setName("Test");

    /* Channel that doesn't belong to mode->fixtureDef() */
    QLCChannel* ch = new QLCChannel();
    ch->setName("Rogue");
    mode->insertChannel(ch, 0);
    QVERIFY(mode->channels().size() == 0);
    delete ch;

    /* First channel */
    mode->insertChannel(m_ch1, 0);
    QVERIFY(mode->channel(0) == m_ch1);

    /* Second prepended */
    mode->insertChannel(m_ch2, 0);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch1);

    /* Third appended way over the end */
    mode->insertChannel(m_ch3, 10);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch1);
    QVERIFY(mode->channel(2) == m_ch3);

    /* Fourth inserted in-between */
    mode->insertChannel(m_ch4, 1);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch4);
    QVERIFY(mode->channel(2) == m_ch1);
    QVERIFY(mode->channel(3) == m_ch3);

    delete mode;
}

void QLCFixtureMode_Test::removeChannel()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    /* Remove one channel in the middle */
    QVERIFY(mode->channels().size() == 4);
    mode->removeChannel(m_ch2);
    QVERIFY(mode->channels().size() == 3);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == m_ch4);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove the same channel again. Shouldn't change anything. */
    mode->removeChannel(m_ch2);
    QVERIFY(mode->channels().size() == 3);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == m_ch4);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove last channel. */
    mode->removeChannel(m_ch4);
    QVERIFY(mode->channels().size() == 2);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove first channel. */
    mode->removeChannel(m_ch1);
    QVERIFY(mode->channels().size() == 1);
    QVERIFY(mode->channel(0) == m_ch3);
    QVERIFY(mode->channel(1) == NULL);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove last channel. */
    mode->removeChannel(m_ch3);
    QVERIFY(mode->channels().size() == 0);
    QVERIFY(mode->channel(0) == NULL);
    QVERIFY(mode->channel(1) == NULL);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channelByName()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channel("Channel 1") == m_ch1);
    QVERIFY(mode->channel("Channel 2") == m_ch2);
    QVERIFY(mode->channel("Channel 3") == m_ch3);
    QVERIFY(mode->channel("Channel 4") == m_ch4);
    QVERIFY(mode->channel("Foobar") == NULL);
    QVERIFY(mode->channel("") == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channelByIndex()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch2);
    QVERIFY(mode->channel(2) == m_ch3);
    QVERIFY(mode->channel(3) == m_ch4);
    QVERIFY(mode->channel(12) == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channels()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    QVERIFY(mode->channels().size() == 0);

    mode->insertChannel(m_ch1, 0);
    QVERIFY(mode->channels().size() == 1);

    mode->insertChannel(m_ch2, 1);
    QVERIFY(mode->channels().size() == 2);

    mode->insertChannel(m_ch3, 2);
    QVERIFY(mode->channels().size() == 3);

    delete mode;
}

void QLCFixtureMode_Test::channelNumber()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channelNumber(m_ch1) == 0);
    QVERIFY(mode->channelNumber(m_ch2) == 1);
    QVERIFY(mode->channelNumber(m_ch3) == 2);
    QVERIFY(mode->channelNumber(m_ch4) == 3);

    QLCChannel* ch = new QLCChannel();
    QVERIFY(mode->channelNumber(ch) == KChannelInvalid);
    QVERIFY(mode->channelNumber(NULL) == KChannelInvalid);
    delete ch;

    delete mode;
}

void QLCFixtureMode_Test::copy()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    mode->setName("Test Mode");

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    /* Create a copy of the mode to the same fixtureDef as the original */
    QLCFixtureMode* copy = new QLCFixtureMode(m_fixtureDef, mode);
    QVERIFY(copy != NULL);

    QVERIFY(copy->name() == "Test Mode");
    QVERIFY(copy->channels().size() == 4);
    QVERIFY(copy->channel(0) == m_ch1);
    QVERIFY(copy->channel(1) == m_ch2);
    QVERIFY(copy->channel(2) == m_ch3);
    QVERIFY(copy->channel(3) == m_ch4);
    delete copy;
    copy = NULL;

    /* Create another fixture def with some channels matching, some not */
    QLCFixtureDef* anotherDef = new QLCFixtureDef();
    QLCChannel* ch1 = new QLCChannel();
    ch1->setName("Channel 1"); // Should match
    anotherDef->addChannel(ch1);

    QLCChannel* ch2 = new QLCChannel();
    ch2->setName("Channel 2, not the same name"); // Shouldn't match
    anotherDef->addChannel(ch2);

    QLCChannel* ch3 = new QLCChannel();
    ch3->setName("Channel 3, still not the same name"); // Shouldn't match
    anotherDef->addChannel(ch3);

    QLCChannel* ch4 = new QLCChannel();
    ch4->setName("Channel 4"); // Should match
    anotherDef->addChannel(ch4);

    QLCChannel* ch5 = new QLCChannel();
    ch5->setName("Channel 5"); // Shouldn't match since original has 4 chans
    anotherDef->addChannel(ch5);

    /* Create a copy of the mode to the other fixtureDef */
    copy = new QLCFixtureMode(anotherDef, mode);

    QVERIFY(copy->name() == "Test Mode");
    QVERIFY(copy->channels().size() == 2);

    QVERIFY(copy->channel(0)->name() == "Channel 1");
    QVERIFY(copy->channel(0) == ch1);

    QVERIFY(copy->channel(1)->name() == "Channel 4");
    QVERIFY(copy->channel(1) == ch4);

    delete copy;
    delete anotherDef;
}

void QLCFixtureMode_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Mode");
    root.setAttribute("Name", "Mode1");
    doc.appendChild(root);

    QDomElement ch1 = doc.createElement("Channel");
    ch1.setAttribute("Number", 0);
    QDomText ch1Text = doc.createTextNode("Channel 1");
    ch1.appendChild(ch1Text);
    root.appendChild(ch1);

    /* Shouldn't appear in the mode since Channel 1 is already added */
    QDomElement ch2 = doc.createElement("Channel");
    ch2.setAttribute("Number", 1);
    QDomText ch2Text = doc.createTextNode("Channel 1");
    ch2.appendChild(ch2Text);
    root.appendChild(ch2);

    QDomElement ch3 = doc.createElement("Channel");
    ch3.setAttribute("Number", 1);
    QDomText ch3Text = doc.createTextNode("Channel 3");
    ch3.appendChild(ch3Text);
    root.appendChild(ch3);

    /* Physical */
    QDomElement phys = doc.createElement("Physical");
    root.appendChild(phys);

    /* Bulb */
    QDomElement bulb = doc.createElement("Bulb");
    bulb.setAttribute("Type", "LED");
    bulb.setAttribute("Lumens", 18000);
    bulb.setAttribute("ColourTemperature", 6500);
    phys.appendChild(bulb);

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.physical().bulbType() != "LED");
    QVERIFY(mode.physical().bulbLumens() != 18000);
    QVERIFY(mode.physical().bulbColourTemperature() != 6500);

    QVERIFY(mode.loadXML(&root) == true);
    QVERIFY(mode.physical().bulbType() == "LED");
    QVERIFY(mode.physical().bulbLumens() == 18000);
    QVERIFY(mode.physical().bulbColourTemperature() == 6500);

    QVERIFY(mode.channels().size() == 2);
    QVERIFY(mode.channels()[0] == m_ch1);
    QVERIFY(mode.channels()[1] == m_ch3);
}

void QLCFixtureMode_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("ode");
    root.setAttribute("Name", "Mode1");
    doc.appendChild(root);

    QDomElement ch1 = doc.createElement("Channel");
    ch1.setAttribute("Number", 0);
    QDomText ch1Text = doc.createTextNode("Channel 1");
    ch1.appendChild(ch1Text);
    root.appendChild(ch1);

    /* Shouldn't appear in the mode since Channel 1 is already added */
    QDomElement ch2 = doc.createElement("Channel");
    ch2.setAttribute("Number", 1);
    QDomText ch2Text = doc.createTextNode("Channel 1");
    ch2.appendChild(ch2Text);
    root.appendChild(ch2);

    QDomElement ch3 = doc.createElement("Channel");
    ch3.setAttribute("Number", 1);
    QDomText ch3Text = doc.createTextNode("Channel 3");
    ch3.appendChild(ch3Text);
    root.appendChild(ch3);

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.loadXML(&root) == false);
    QVERIFY(mode.channels().size() == 0);
}

void QLCFixtureMode_Test::loadNoName()
{
    QDomDocument doc;

    /* Loading should fail because mode has no name */
    QDomElement root = doc.createElement("Mode");
    doc.appendChild(root);

    QDomElement ch1 = doc.createElement("Channel");
    ch1.setAttribute("Number", 0);
    QDomText ch1Text = doc.createTextNode("Channel 1");
    ch1.appendChild(ch1Text);
    root.appendChild(ch1);

    /* Shouldn't appear in the mode since Channel 1 is already added */
    QDomElement ch2 = doc.createElement("Channel");
    ch2.setAttribute("Number", 1);
    QDomText ch2Text = doc.createTextNode("Channel 1");
    ch2.appendChild(ch2Text);
    root.appendChild(ch2);

    QDomElement ch3 = doc.createElement("Channel");
    ch3.setAttribute("Number", 1);
    QDomText ch3Text = doc.createTextNode("Channel 3");
    ch3.appendChild(ch3Text);
    root.appendChild(ch3);

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.loadXML(&root) == false);
    QVERIFY(mode.channels().size() == 0);
}

void QLCFixtureMode_Test::cleanupTestCase()
{
    QVERIFY(m_fixtureDef != NULL);
    delete m_fixtureDef;
}
