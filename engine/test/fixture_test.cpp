/*
  Q Light Controller - Unit test
  fixture_test.cpp

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
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"

#include "fixture_test.h"
#include "fixture.h"
#include "doc.h"
#include "bus.h"

void Fixture_Test::initTestCase()
{
	Bus::init(this);
#ifdef WIN32
	QVERIFY(m_fixtureDefCache.load("../../../fixtures/") == true);
#else
	QVERIFY(m_fixtureDefCache.load("../../fixtures/") == true);
#endif
}

void Fixture_Test::id()
{
	QVERIFY(Fixture::invalidId() == -1);

	Fixture fxi(this);
	QVERIFY(fxi.id() == Fixture::invalidId());

	fxi.setID(50);
	QVERIFY(fxi.id() == 50);

	fxi.setID(INT_MAX);
	QVERIFY(fxi.id() == INT_MAX);
}

void Fixture_Test::name()
{
	Fixture fxi(this);
	QVERIFY(fxi.name() == QString::null);

	fxi.setName("MyFixture");
	QVERIFY(fxi.name() == "MyFixture");
}

void Fixture_Test::address()
{
	Fixture fxi(this);
	fxi.setChannels(5);

	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.universeAddress() == 0);

	fxi.setUniverse(1);
	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 1);
	QVERIFY(fxi.universeAddress() == (1 << 9));

	fxi.setUniverse(2);
	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 2);
	QVERIFY(fxi.universeAddress() == (2 << 9));

	fxi.setUniverse(3);
	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 3);
	QVERIFY(fxi.universeAddress() == (3 << 9));

	/* The application might support only 4 universes, but there's no
	   reason why Fixture itself couldn't support a million universes,
	   as long as it fits into a uint minus 9 bits. */
	fxi.setUniverse(100);
	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 100);
	QVERIFY(fxi.universeAddress() == (100 << 9));

	fxi.setAddress(15);
	fxi.setUniverse(0);
	QVERIFY(fxi.address() == 15);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.universeAddress() == 15);

	/* Fixture should allow address overflow; maybe the first two channels
	   that still fit to the universe here are enough for some fixture,
	   who knows? */
	fxi.setAddress(510);
	QVERIFY(fxi.address() == 510);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.universeAddress() == 510);

	/* Invalid addresses should not be allowed */
	fxi.setAddress(600);
	QVERIFY(fxi.address() == 510);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.universeAddress() == 510);

	fxi.setAddress(100);
	QVERIFY(fxi.channelAddress(0) == 100);
	QVERIFY(fxi.channelAddress(1) == 101);
	QVERIFY(fxi.channelAddress(2) == 102);
	QVERIFY(fxi.channelAddress(3) == 103);
	QVERIFY(fxi.channelAddress(4) == 104);
	QVERIFY(fxi.channelAddress(5) == Fixture::invalidChannel());
	QVERIFY(fxi.channelAddress(20) == Fixture::invalidChannel());
}

void Fixture_Test::dimmer()
{
	Fixture fxi(this);

	QVERIFY(fxi.fixtureDef() == NULL);
	QVERIFY(fxi.fixtureMode() == NULL);
	QVERIFY(fxi.channels() == 0);
	QVERIFY(fxi.channel(0) == NULL);
	QVERIFY(fxi.channel(42) == NULL);

	/* All channels point to the same generic channel instance */
	fxi.setChannels(5);
	QVERIFY(fxi.channels() == 5);
	QVERIFY(fxi.channel(0) != NULL);
	const QLCChannel* ch = fxi.channel(0);
	QVERIFY(fxi.channel(1) == ch);
	QVERIFY(fxi.channel(2) == ch);
	QVERIFY(fxi.channel(3) == ch);
	QVERIFY(fxi.channel(4) == ch);
	QVERIFY(fxi.channel(5) == NULL);
	QVERIFY(fxi.channel(42) == NULL);
	QVERIFY(fxi.channel(Fixture::invalidChannel()) == NULL);

	QVERIFY(ch->capabilities().count() == 1);
	QVERIFY(ch->capabilities().at(0)->min() == 0);
	QVERIFY(ch->capabilities().at(0)->max() == 255);
	QVERIFY(ch->capabilities().at(0)->name() == "Intensity");

	/* Although the dimmer fixture HAS a channel with this name, it is
	   not returned, because all channels have the same name. */
	QVERIFY(fxi.channel("Intensity") == Fixture::invalidChannel());
}

void Fixture_Test::fixtureDef()
{
	Fixture fxi(this);

	QVERIFY(fxi.fixtureDef() == NULL);
	QVERIFY(fxi.fixtureMode() == NULL);
	QVERIFY(fxi.channels() == 0);
	QVERIFY(fxi.channel(0) == NULL);

	const QLCFixtureDef* fixtureDef;
	fixtureDef = m_fixtureDefCache.fixtureDef("Martin", "MAC250+");
	Q_ASSERT(fixtureDef != NULL);

	fxi.setFixtureDefinition(fixtureDef, NULL);
	QVERIFY(fxi.fixtureDef() == NULL);
	QVERIFY(fxi.fixtureMode() == NULL);

	const QLCFixtureMode* fixtureMode;
	fixtureMode = fixtureDef->modes().at(0);
	Q_ASSERT(fixtureMode != NULL);

	fxi.setFixtureDefinition(NULL, fixtureMode);
	QVERIFY(fxi.fixtureDef() == NULL);
	QVERIFY(fxi.fixtureMode() == NULL);

	fxi.setFixtureDefinition(fixtureDef, fixtureMode);
	QVERIFY(fxi.fixtureDef() != NULL);
	QVERIFY(fxi.fixtureMode() != NULL);
	QVERIFY(fxi.fixtureDef() == fixtureDef);
	QVERIFY(fxi.fixtureMode() == fixtureMode);

	QVERIFY(fxi.channels() == fixtureMode->channels().count());
	QVERIFY(fxi.channel(fxi.channels() - 1) != NULL);
	QVERIFY(fxi.channel(fxi.channels()) == NULL);

	QVERIFY(fxi.channel("Pan") != Fixture::invalidChannel());
	const QLCChannel* ch = fxi.channel(fxi.channel("Pan"));
	QVERIFY(ch != NULL);
	QVERIFY(ch->name().toLower() == "pan");

	ch = fxi.channel(fxi.channel("Pan", Qt::CaseInsensitive, "Colour"));
	QVERIFY(ch == NULL);

	QVERIFY(fxi.channel("otation") != Fixture::invalidChannel());
	ch = fxi.channel(fxi.channel("otati"));
	QVERIFY(ch != NULL);
	QVERIFY(ch->name().toLower() == "gobo rotation");
}

void Fixture_Test::loadWrongRoot()
{
	QDomDocument doc;
	Fixture fxi(this);

	QDomElement root = doc.createElement("Function");
	doc.appendChild(root);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == false);
}

void Fixture_Test::loadFixtureDef()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("9");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("0");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("MAC250+");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Mode 1");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Martin");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("21");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == true);
	QVERIFY(fxi.name() == "Foobar");
	QVERIFY(fxi.channels() == 9);
	QVERIFY(fxi.address() == 21);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.fixtureDef() != NULL);
	QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadFixtureDefWrongChannels()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("15");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Mode 1");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("0");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("MAC250+");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Martin");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("21");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == true);
	QVERIFY(fxi.name() == "Foobar");
	QVERIFY(fxi.channels() == 9);
	QVERIFY(fxi.address() == 21);
	QVERIFY(fxi.universe() == 0);
	QVERIFY(fxi.fixtureDef() != NULL);
	QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadDimmer()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("18");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("3");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("Foobar");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Foobar");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Foobar");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("21");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == true);
	QVERIFY(fxi.name() == "Foobar");
	QVERIFY(fxi.channels() == 18);
	QVERIFY(fxi.address() == 21);
	QVERIFY(fxi.universe() == 3);
	QVERIFY(fxi.fixtureDef() == NULL);
	QVERIFY(fxi.fixtureMode() == NULL);
}

void Fixture_Test::loadWrongAddress()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("18");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("0");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("Foobar");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Foobar");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Foobar");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("512");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == true);
	QVERIFY(fxi.name() == "Foobar");
	QVERIFY(fxi.channels() == 18);
	QVERIFY(fxi.address() == 0);
	QVERIFY(fxi.universe() == 0);
}

void Fixture_Test::loadWrongUniverse()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("18");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("4");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("Foobar");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Foobar");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Foobar");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("25");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == true);
	QVERIFY(fxi.name() == "Foobar");
	QVERIFY(fxi.channels() == 18);
	QVERIFY(fxi.address() == 25);
	QVERIFY(fxi.universe() == 0);
}

void Fixture_Test::loadWrongID()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("9");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("0");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("MAC250+");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Mode 1");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Martin");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode(QString("%1")
						.arg(KFixtureArraySize));
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("21");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Fixture fxi(this);
	QVERIFY(fxi.loadXML(&root, m_fixtureDefCache) == false);
}

void Fixture_Test::loader()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Fixture");
	doc.appendChild(root);

	QDomElement chs = doc.createElement("Channels");
	QDomText chsText = doc.createTextNode("18");
	chs.appendChild(chsText);
	root.appendChild(chs);

	QDomElement name = doc.createElement("Name");
	QDomText nameText = doc.createTextNode("Foobar");
	name.appendChild(nameText);
	root.appendChild(name);

	QDomElement uni = doc.createElement("Universe");
	QDomText uniText = doc.createTextNode("3");
	uni.appendChild(uniText);
	root.appendChild(uni);

	QDomElement model = doc.createElement("Model");
	QDomText modelText = doc.createTextNode("Foobar");
	model.appendChild(modelText);
	root.appendChild(model);

	QDomElement mode = doc.createElement("Mode");
	QDomText modeText = doc.createTextNode("Foobar");
	mode.appendChild(modeText);
	root.appendChild(mode);

	QDomElement type = doc.createElement("Manufacturer");
	QDomText typeText = doc.createTextNode("Foobar");
	type.appendChild(typeText);
	root.appendChild(type);

	QDomElement id = doc.createElement("ID");
	QDomText idText = doc.createTextNode("42");
	id.appendChild(idText);
	root.appendChild(id);

	QDomElement addr = doc.createElement("Address");
	QDomText addrText = doc.createTextNode("21");
	addr.appendChild(addrText);
	root.appendChild(addr);

	Doc* qlcdoc = new Doc(this, m_fixtureDefCache);
	QVERIFY(qlcdoc != NULL);
	QVERIFY(qlcdoc->fixtures() == 0);

	QVERIFY(Fixture::loader(&root, qlcdoc) == true);
	QVERIFY(qlcdoc->fixtures() == 1);
	QVERIFY(qlcdoc->fixture(0) == NULL); // No ID auto-assignment

	Fixture* fxi = qlcdoc->fixture(42);
	QVERIFY(fxi != NULL);
	QVERIFY(fxi->name() == "Foobar");
	QVERIFY(fxi->channels() == 18);
	QVERIFY(fxi->address() == 21);
	QVERIFY(fxi->universe() == 3);
	QVERIFY(fxi->fixtureDef() == NULL);
	QVERIFY(fxi->fixtureMode() == NULL);

	delete qlcdoc;
}

void Fixture_Test::save()
{
	const QLCFixtureDef* fixtureDef;
	fixtureDef = m_fixtureDefCache.fixtureDef("Martin", "MAC250+");
	Q_ASSERT(fixtureDef != NULL);

	const QLCFixtureMode* fixtureMode;
	fixtureMode = fixtureDef->modes().at(0);
	Q_ASSERT(fixtureMode != NULL);

	Fixture fxi(this);
	fxi.setID(1337);
	fxi.setName("Test Fixture");
	fxi.setUniverse(2);
	fxi.setAddress(438);
	fxi.setFixtureDefinition(fixtureDef, fixtureMode);

	QDomDocument doc;
	QDomElement root = doc.createElement("TestRoot");
	QVERIFY(fxi.saveXML(&doc, &root) == true);
	QDomNode node = root.firstChild();
	QVERIFY(node.toElement().tagName() == "Fixture");

	bool manufacturer = false, model = false, mode = false, name = false,
		channels = false, universe = false, address = false, id = false;

	node = node.firstChild();
	while (node.isNull() == false)
	{
		QDomElement e = node.toElement();
		
		if (e.tagName() == "Manufacturer")
		{
			QVERIFY(e.text() == "Martin");
			manufacturer = true;
		}
		else if (e.tagName() == "Model")
		{
			QVERIFY(e.text() == "MAC250+");
			model = true;
		}
		else if (e.tagName() == "Mode")
		{
			QVERIFY(e.text() == fixtureMode->name());
			mode = true;
		}
		else if (e.tagName() == "ID")
		{
			QVERIFY(e.text() == "1337");
			id = true;
		}
		else if (e.tagName() == "Name")
		{
			QVERIFY(e.text() == "Test Fixture");
			name = true;
		}
		else if (e.tagName() == "Universe")
		{
			QVERIFY(e.text() == "2");
			universe = true;
		}
		else if (e.tagName() == "Address")
		{
			QVERIFY(e.text() == "438");
			address = true;
		}
		else if (e.tagName() == "Channels")
		{
			QVERIFY(e.text().toInt()
					== fixtureMode->channels().count());
			channels = true;
		}
		else
		{
			QFAIL(QString("Unexpected tag: %1").arg(e.tagName())
				.toAscii());
		}

		node = node.nextSibling();
	}

	QVERIFY(manufacturer == true);
	QVERIFY(model == true);
	QVERIFY(mode == true);
	QVERIFY(id == true);
	QVERIFY(name == true);
	QVERIFY(universe == true);
	QVERIFY(address == true);
	QVERIFY(channels == true);
}


















