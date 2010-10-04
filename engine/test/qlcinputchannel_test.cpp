/*
  Q Light Controller - Unit tests
  qlcinputchannel_test.cpp

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

#include "qlcinputchannel_test.h"
#include "qlcinputchannel.h"

void QLCInputChannel_Test::type()
{
    QLCInputChannel ch;
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Slider);
    QVERIFY(ch.type() == QLCInputChannel::Slider);

    ch.setType(QLCInputChannel::Button);
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Knob);
    QVERIFY(ch.type() == QLCInputChannel::Knob);
}

void QLCInputChannel_Test::name()
{
    QLCInputChannel ch;
    QVERIFY(ch.name() == QString::null);
    ch.setName("Foobar");
    QVERIFY(ch.name() == "Foobar");
}

void QLCInputChannel_Test::copy()
{
    QLCInputChannel ch;
    ch.setType(QLCInputChannel::Slider);
    ch.setName("Foobar");

    QLCInputChannel copy(ch);
    QVERIFY(copy.type() == QLCInputChannel::Slider);
    QVERIFY(copy.name() == "Foobar");

    QLCInputChannel another = ch;
    QVERIFY(another.type() == QLCInputChannel::Slider);
    QVERIFY(another.name() == "Foobar");
}

void QLCInputChannel_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Channel");
    doc.appendChild(root);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement type = doc.createElement("Type");
    QDomText typeText = doc.createTextNode("Slider");
    type.appendChild(typeText);
    root.appendChild(type);

    QLCInputChannel ch;
    ch.loadXML(&root);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::Slider);
}

void QLCInputChannel_Test::loadWrongType()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Channel");
    doc.appendChild(root);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement type = doc.createElement("Type");
    QDomText typeText = doc.createTextNode("Xyzzy");
    type.appendChild(typeText);
    root.appendChild(type);

    QLCInputChannel ch;
    ch.loadXML(&root);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::NoType);
}
