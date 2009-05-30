#include <QtTest>
#include <QtXml>

#include "qlccapability_test.h"
#include "../qlccapability.h"

void QLCCapability_Test::initial()
{
	QLCCapability cap;
	QVERIFY(cap.min() == KChannelValueMin);
	QVERIFY(cap.max() == KChannelValueMax);
	QVERIFY(cap.name() == QString::null);
}

void QLCCapability_Test::min_data()
{
	QTest::addColumn<t_value> ("value");
	for (t_value i = 0; i < KChannelValueMax; i++)
		QTest::newRow("foo") << i;
	QTest::newRow("foo") << t_value(255);
}

void QLCCapability_Test::min()
{
	QLCCapability cap;
	QVERIFY(cap.min() == KChannelValueMin);

	QFETCH(t_value, value);

	cap.setMin(value);
	QCOMPARE(cap.min(), value);
}

void QLCCapability_Test::max_data()
{
	QTest::addColumn<t_value> ("value");
	for (t_value i = 0; i < KChannelValueMax; i++)
		QTest::newRow("foo") << i;
	QTest::newRow("foo") << t_value(255);
}

void QLCCapability_Test::max()
{
	QLCCapability cap;
	QVERIFY(cap.max() == KChannelValueMax);

	QFETCH(t_value, value);

	cap.setMax(value);
	QCOMPARE(cap.max(), value);
}

void QLCCapability_Test::name()
{
	QLCCapability cap;
	QVERIFY(cap.name() == QString::null);

	cap.setName("Foobar");
	QVERIFY(cap.name() == "Foobar");
}

void QLCCapability_Test::overlaps()
{
	QLCCapability cap1;
	QLCCapability cap2;
	QVERIFY(cap1.overlaps(cap2) == true);
	QVERIFY(cap2.overlaps(cap1) == true);

	/* cap2 contains cap1 completely */
	cap1.setMin(10);
	cap1.setMax(245);
	QVERIFY(cap1.overlaps(cap2) == true);
	QVERIFY(cap2.overlaps(cap1) == true);

	/* cap2's max overlaps cap1 */
	cap2.setMin(0);
	cap2.setMax(10);
	QVERIFY(cap1.overlaps(cap2) == true);
	QVERIFY(cap2.overlaps(cap1) == true);

	/* cap2's min overlaps cap1 */
	cap2.setMin(245);
	cap2.setMax(255);
	QVERIFY(cap1.overlaps(cap2) == true);
	QVERIFY(cap2.overlaps(cap1) == true);

	/* cap1 contains cap2 completely */
	cap2.setMin(20);
	cap2.setMax(235);
	QVERIFY(cap1.overlaps(cap2) == true);
	QVERIFY(cap2.overlaps(cap1) == true);

	cap2.setMin(0);
	cap2.setMax(9);
	QVERIFY(cap1.overlaps(cap2) == false);
	QVERIFY(cap2.overlaps(cap1) == false);
}

void QLCCapability_Test::copy()
{
	QLCCapability cap1;
	QVERIFY(cap1.min() == KChannelValueMin);
	QVERIFY(cap1.max() == KChannelValueMax);
	QVERIFY(cap1.name() == QString::null);

	cap1.setMin(5);
	cap1.setMax(15);
	cap1.setName("Foobar");
	
	QLCCapability cap2 = cap1;
	QVERIFY(cap2.min() == 5);
	QVERIFY(cap2.max() == 15);
	QVERIFY(cap2.name() == "Foobar");
}

void QLCCapability_Test::load()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Capability");
	doc.appendChild(root);

	root.setAttribute("Min", 13);
	root.setAttribute("Max", 19);

	QDomText name = doc.createTextNode("Test1");
	root.appendChild(name);

	QLCCapability cap;
	QVERIFY(cap.loadXML(&root) == true);
	QVERIFY(cap.name() == "Test1");
	QVERIFY(cap.min() == 13);
	QVERIFY(cap.max() == 19);
}

void QLCCapability_Test::loadWrongRoot()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("apability");
	doc.appendChild(root);

	root.setAttribute("Min", 13);
	root.setAttribute("Max", 19);

	QDomText name = doc.createTextNode("Test1");
	root.appendChild(name);

	QLCCapability cap;
	QVERIFY(cap.loadXML(&root) == false);
	QVERIFY(cap.name() == QString::null);
	QVERIFY(cap.min() == KChannelValueMin);
	QVERIFY(cap.max() == KChannelValueMax);
}

void QLCCapability_Test::loadNoMin()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Capability");
	doc.appendChild(root);

	root.setAttribute("Max", 19);

	QDomText name = doc.createTextNode("Test1");
	root.appendChild(name);

	QLCCapability cap;
	QVERIFY(cap.loadXML(&root) == false);
	QVERIFY(cap.name() == QString::null);
	QVERIFY(cap.min() == KChannelValueMin);
	QVERIFY(cap.max() == KChannelValueMax);
}

void QLCCapability_Test::loadNoMax()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Capability");
	doc.appendChild(root);

	root.setAttribute("Min", 13);

	QDomText name = doc.createTextNode("Test1");
	root.appendChild(name);

	QLCCapability cap;
	QVERIFY(cap.loadXML(&root) == false);
	QVERIFY(cap.name() == QString::null);
	QVERIFY(cap.min() == KChannelValueMin);
	QVERIFY(cap.max() == KChannelValueMax);
}

void QLCCapability_Test::loadMinGreaterThanMax()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Capability");
	doc.appendChild(root);

	root.setAttribute("Min", 20);
	root.setAttribute("Max", 19);

	QDomText name = doc.createTextNode("Test1");
	root.appendChild(name);

	QLCCapability cap;
	QVERIFY(cap.loadXML(&root) == false);
	QVERIFY(cap.name() == QString::null);
	QVERIFY(cap.min() == KChannelValueMin);
	QVERIFY(cap.max() == KChannelValueMax);
}
