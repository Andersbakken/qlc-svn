#include <QtTest>
#include <QtXml>

#include "qlcinputchannel_test.h"
#include "../qlcinputchannel.h"

void QLCInputChannel_Test::channel()
{
	QLCInputChannel ch;
	QVERIFY(ch.channel() == 0);
	ch.setChannel(5);
	QVERIFY(ch.channel() == 5);
}

void QLCInputChannel_Test::type()
{
	QLCInputChannel ch;
	QVERIFY(ch.type() == QLCInputChannel::NoType);
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
	ch.setChannel(5);
	ch.setType(QLCInputChannel::Slider);
	ch.setName("Foobar");

	QLCInputChannel copy(ch);
	QVERIFY(copy.channel() == 5);
	QVERIFY(copy.type() == QLCInputChannel::Slider);
	QVERIFY(copy.name() == "Foobar");

	QLCInputChannel another = ch;
	QVERIFY(another.channel() == 5);
	QVERIFY(another.type() == QLCInputChannel::Slider);
	QVERIFY(another.name() == "Foobar");
}

void QLCInputChannel_Test::load()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Channel");
	root.setAttribute("Number", 5);
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
	QVERIFY(ch.channel() == 5);
	QVERIFY(ch.type() == QLCInputChannel::Slider);
}

void QLCInputChannel_Test::loadWrongType()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Channel");
	root.setAttribute("Number", 5);
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
	QVERIFY(ch.channel() == 5);
	QVERIFY(ch.type() == QLCInputChannel::NoType);
}
