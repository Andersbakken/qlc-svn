#include <QtTest>
#include <QtXml>

#include "qlcfixturedefcache_test.h"
#include "../qlcfixturedefcache.h"
#include "../qlcfixturedef.h"

void QLCFixtureDefCache_Test::adding()
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
	QVERIFY(cache.manufacturers().at(0) == "Martin");
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

	/* Another fixtureDef, same manufacturer, different model */
	def2->setManufacturer("Martin");
	def2->setModel("MAC500");
	QVERIFY(cache.addFixtureDef(def2) == true);
	QVERIFY(cache.manufacturers().count() == 1);
	QVERIFY(cache.manufacturers().at(0) == "Martin");
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
	QVERIFY(spy.count() == 3);
	QVERIFY(spy.at(2).count() == 2);
	QVERIFY(spy.at(2).at(0).toString() == "Futurelight");
	QVERIFY(spy.at(2).at(1).toString() == "PHS700");
}
