#include <QtTest>
#include <QtXml>

#include "qlcphysical_test.h"
#include "../qlcphysical.h"

void QLCPhysical_Test::bulbType()
{
	QVERIFY(p.bulbType() == QString::null);
	p.setBulbType("BulbType");
	QVERIFY(p.bulbType() == "BulbType");
}

void QLCPhysical_Test::bulbLumens()
{
	QVERIFY(p.bulbLumens() == 0);
	p.setBulbLumens(10000);
	QVERIFY(p.bulbLumens() == 10000);
}

void QLCPhysical_Test::bulbColourTemp()
{
	QVERIFY(p.bulbColourTemperature() == 0);
	p.setBulbColourTemperature(3200);
	QVERIFY(p.bulbColourTemperature() == 3200);
}

void QLCPhysical_Test::weight()
{
	QVERIFY(p.weight() == 0);
	p.setWeight(7);
	QVERIFY(p.weight() == 7);
}

void QLCPhysical_Test::width()
{
	QVERIFY(p.width() == 0);
	p.setWidth(600);
	QVERIFY(p.width() == 600);
}

void QLCPhysical_Test::height()
{
	QVERIFY(p.height() == 0);
	p.setHeight(1200);
	QVERIFY(p.height() == 1200);
}

void QLCPhysical_Test::depth()
{
	QVERIFY(p.depth() == 0);
	p.setDepth(250);
	QVERIFY(p.depth() == 250);
}

void QLCPhysical_Test::lensName()
{
	QVERIFY(p.lensName() == "Other");
	p.setLensName("Fresnel");
	QVERIFY(p.lensName() == "Fresnel");
}

void QLCPhysical_Test::lensDegreesMin()
{
	QVERIFY(p.lensDegreesMin() == 0);
	p.setLensDegreesMin(9);
	QVERIFY(p.lensDegreesMin() == 9);
}

void QLCPhysical_Test::lensDegreesMax()
{
	QVERIFY(p.lensDegreesMax() == 0);
	p.setLensDegreesMax(40);
	QVERIFY(p.lensDegreesMax() == 40);
}

void QLCPhysical_Test::focusType()
{
	QVERIFY(p.focusType() == "Fixed");
	p.setFocusType("Head");
	QVERIFY(p.focusType() == "Head");
}

void QLCPhysical_Test::focusPanMax()
{
	QVERIFY(p.focusPanMax() == 0);
	p.setFocusPanMax(540);
	QVERIFY(p.focusPanMax() == 540);
}

void QLCPhysical_Test::focusTiltMax()
{
	QVERIFY(p.focusTiltMax() == 0);
	p.setFocusTiltMax(270);
	QVERIFY(p.focusTiltMax() == 270);
}

void QLCPhysical_Test::copy()
{
	QLCPhysical c = p;
	QVERIFY(c.bulbType() == p.bulbType());
	QVERIFY(c.bulbLumens() == p.bulbLumens());
	QVERIFY(c.bulbColourTemperature() == p.bulbColourTemperature());
	QVERIFY(c.weight() == p.weight());
	QVERIFY(c.width() == p.width());
	QVERIFY(c.height() == p.height());
	QVERIFY(c.depth() == p.depth());
	QVERIFY(c.lensName() == p.lensName());
	QVERIFY(c.lensDegreesMin() == p.lensDegreesMin());
	QVERIFY(c.lensDegreesMax() == p.lensDegreesMax());
	QVERIFY(c.focusType() == p.focusType());
	QVERIFY(c.focusPanMax() == p.focusPanMax());
	QVERIFY(c.focusTiltMax() == p.focusTiltMax());

}
