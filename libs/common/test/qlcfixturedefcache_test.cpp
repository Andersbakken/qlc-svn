#include <QtTest>
#include <QtXml>

#include "qlcfixturedefcache_test.h"
#include "../qlcfixturedefcache.h"
#include "../qlcfixturedef.h"
#include "../qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void QLCFixtureDefCache_Test::add()
{
	QLCFixtureDefCache cache(this);
	QVERIFY(cache.manufacturers().count() == 0);

	QSignalSpy spy(&cache, SIGNAL(fixtureDefAdded(const QString&,
						      const QString&)));

	/* Add the first fixtureDef */
	QLCFixtureDef* def = new QLCFixtureDef();
	def->setManufacturer("Martin");
	def->setModel("MAC250");

	QVERIFY(cache.addFixtureDef(def) == true);
	QVERIFY(cache.manufacturers().count() == 1);
	QVERIFY(cache.manufacturers().contains("Martin") == true);
	QVERIFY(cache.manufacturers().contains("MAC250") == false);

	QVERIFY(cache.models("Martin").count() == 1);
	QVERIFY(cache.models("Martin").contains("MAC250") == true);
	QVERIFY(cache.models("Foo").count() == 0);
	
	QVERIFY(spy.count() == 1);
	QVERIFY(spy.at(0).count() == 2);
	QVERIFY(spy.at(0).at(0).toString() == "Martin");
	QVERIFY(spy.at(0).at(1).toString() == "MAC250");

	/* Another fixtureDef, same manufacturer & model. Should be ignored. */
	QLCFixtureDef* def2 = new QLCFixtureDef();
	def2->setManufacturer("Martin");
	def2->setModel("MAC250");

	QVERIFY(cache.addFixtureDef(def2) == false);
	QVERIFY(cache.manufacturers().count() == 1);
	QVERIFY(cache.manufacturers().contains("Martin") == true);

	QVERIFY(spy.count() == 1);
	delete def2;
	def2 = NULL;

	/* Another fixtureDef, same manufacturer, different model */
	def2 = new QLCFixtureDef();
	def2->setManufacturer("Martin");
	def2->setModel("MAC500");

	QVERIFY(cache.addFixtureDef(def2) == true);
	QVERIFY(cache.manufacturers().count() == 1);
	QVERIFY(cache.manufacturers().contains("Martin") == true);
	QVERIFY(cache.manufacturers().contains("MAC500") == false);

	QVERIFY(cache.models("Martin").count() == 2);
	QVERIFY(cache.models("Martin").contains("MAC250") == true);
	QVERIFY(cache.models("Martin").contains("MAC500") == true);

	QVERIFY(spy.count() == 2);
	QVERIFY(spy.at(1).count() == 2);
	QVERIFY(spy.at(1).at(0).toString() == "Martin");
	QVERIFY(spy.at(1).at(1).toString() == "MAC500");

	/* Another fixtureDef, different manufacturer, different model */
	QLCFixtureDef* def3 = new QLCFixtureDef();
	def3->setManufacturer("Futurelight");
	def3->setModel("PHS700");

	QVERIFY(cache.addFixtureDef(def3) == true);
	QVERIFY(cache.manufacturers().count() == 2);
	QVERIFY(cache.manufacturers().contains("Martin") == true);
	QVERIFY(cache.manufacturers().contains("Futurelight") == true);
	QVERIFY(cache.manufacturers().contains("PHS700") == false);

	QVERIFY(spy.count() == 3);
	QVERIFY(spy.at(2).count() == 2);
	QVERIFY(spy.at(2).at(0).toString() == "Futurelight");
	QVERIFY(spy.at(2).at(1).toString() == "PHS700");

	/* Another fixtureDef, different manufacturer, same model */
	QLCFixtureDef* def4 = new QLCFixtureDef();
	def4->setManufacturer("Yoyodyne");
	def4->setModel("MAC250");

	QVERIFY(cache.addFixtureDef(def4) == true);
	QVERIFY(cache.manufacturers().count() == 3);
	QVERIFY(cache.manufacturers().contains("Martin") == true);
	QVERIFY(cache.manufacturers().contains("Futurelight") == true);
	QVERIFY(cache.manufacturers().contains("Yoyodyne") == true);
	QVERIFY(cache.manufacturers().contains("MAC250") == false);

	QVERIFY(cache.models("Yoyodyne").count() == 1);
	QVERIFY(cache.models("Yoyodyne").contains("MAC250") == true);

	QVERIFY(spy.count() == 4);
	QVERIFY(spy.at(3).count() == 2);
	QVERIFY(spy.at(3).at(0).toString() == "Yoyodyne");
	QVERIFY(spy.at(3).at(1).toString() == "MAC250");
}

void QLCFixtureDefCache_Test::fixtureDef()
{
	QLCFixtureDefCache cache(this);

	QLCFixtureDef* def1 = new QLCFixtureDef();
	def1->setManufacturer("Martin");
	def1->setModel("MAC250");
	cache.addFixtureDef(def1);

	QLCFixtureDef* def2 = new QLCFixtureDef();
	def2->setManufacturer("Martin");
	def2->setModel("MAC500");
	cache.addFixtureDef(def2);

	QLCFixtureDef* def3 = new QLCFixtureDef();
	def3->setManufacturer("Robe");
	def3->setModel("WL250");
	cache.addFixtureDef(def3);

	QLCFixtureDef* def4 = new QLCFixtureDef();
	def4->setManufacturer("Futurelight");
	def4->setModel("DJ Scan 250");
	cache.addFixtureDef(def4);

	QVERIFY(cache.fixtureDef("Martin", "MAC250") == def1);
	QVERIFY(cache.fixtureDef("Martin", "MAC500") == def2);
	QVERIFY(cache.fixtureDef("Robe", "WL250") == def3);
	QVERIFY(cache.fixtureDef("Futurelight", "DJ Scan 250") == def4);
	QVERIFY(cache.fixtureDef("Martin", "MAC 250") == NULL);
	QVERIFY(cache.fixtureDef("Mar tin", "MAC250") == NULL);
	QVERIFY(cache.fixtureDef("Foobar", "Foobar") == NULL);
	QVERIFY(cache.fixtureDef("", "") == NULL);
}

void QLCFixtureDefCache_Test::load()
{
	QLCFixtureDefCache cache(this);

	QSignalSpy spy(&cache, SIGNAL(fixtureDefAdded(const QString&,
						      const QString&)));
	QVERIFY(cache.load(INTERNAL_FIXTUREDIR) == true);

	/* Test only that all available files get loaded properly. Actual
	   contents of fixtureDefs should be tested in QLCFixtureDef_Test. */
	QDir dir(INTERNAL_FIXTUREDIR, QString("*%1").arg(KExtFixture));
	QVERIFY(spy.count() == dir.entryList().count());

	/* At least these should be available */
	QVERIFY(cache.manufacturers().contains("Elation") == true);
	QVERIFY(cache.manufacturers().contains("Eurolite") == true);
	QVERIFY(cache.manufacturers().contains("Futurelight") == true);
	QVERIFY(cache.manufacturers().contains("GLP") == true);
	QVERIFY(cache.manufacturers().contains("JB-Lighting") == true);
	QVERIFY(cache.manufacturers().contains("Lite-Works") == true);
	QVERIFY(cache.manufacturers().contains("Martin") == true);
	QVERIFY(cache.manufacturers().contains("Robe") == true);
	QVERIFY(cache.manufacturers().contains("SGM") == true);
}
