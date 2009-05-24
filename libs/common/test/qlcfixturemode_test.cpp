#include <QtTest>

#include "../qlcfixturemode_test.h"
#include "../qlcfixturemode.h"
#include "../qlcfixturedef.h"

void QLCFixtureMode_Test::initTestCase()
{
	m_fixtureDef = new QLCFixtureDef();
	QVERIFY(m_fixtureDef != NULL);
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

	/* First channel */
	QLCChannel* ch1 = new QLCChannel();
	mode->insertChannel(ch1, 0);
	QVERIFY(mode->channel(0) == ch1);

	/* Second prepended */
	QLCChannel* ch2 = new QLCChannel();
	mode->insertChannel(ch2, 0);
	QVERIFY(mode->channel(0) == ch2);
	QVERIFY(mode->channel(1) == ch1);

	/* Third appended way over the end */
	QLCChannel* ch3 = new QLCChannel();
	mode->insertChannel(ch3, 10);
	QVERIFY(mode->channel(0) == ch2);
	QVERIFY(mode->channel(1) == ch1);
	QVERIFY(mode->channel(2) == ch3);

	/* Fourth inserted in-between */
	QLCChannel* ch4 = new QLCChannel();
	mode->insertChannel(ch4, 1);
	QVERIFY(mode->channel(0) == ch2);
	QVERIFY(mode->channel(1) == ch4);
	QVERIFY(mode->channel(2) == ch1);
	QVERIFY(mode->channel(3) == ch3);

	delete mode;
	delete ch1;
	delete ch2;
	delete ch3;
	delete ch4;
}

void QLCFixtureMode_Test::removeChannel()
{
	QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

	QLCChannel* ch1 = new QLCChannel();
	mode->insertChannel(ch1, 0);
	QLCChannel* ch2 = new QLCChannel();
	mode->insertChannel(ch2, 1);
	QLCChannel* ch3 = new QLCChannel();
	mode->insertChannel(ch3, 2);
	QLCChannel* ch4 = new QLCChannel();
	mode->insertChannel(ch4, 3);

	/* Remove one channel in the middle */
	QVERIFY(mode->channels().size() == 4);
	mode->removeChannel(ch2);
	QVERIFY(mode->channels().size() == 3);
	QVERIFY(mode->channel(0) == ch1);
	QVERIFY(mode->channel(1) == ch3);
	QVERIFY(mode->channel(2) == ch4);
	QVERIFY(mode->channel(3) == NULL);

	/* Remove the same channel again. Shouldn't change anything. */
	mode->removeChannel(ch2);
	QVERIFY(mode->channels().size() == 3);
	QVERIFY(mode->channel(0) == ch1);
	QVERIFY(mode->channel(1) == ch3);
	QVERIFY(mode->channel(2) == ch4);
	QVERIFY(mode->channel(3) == NULL);

	/* Remove last channel. */
	mode->removeChannel(ch4);
	QVERIFY(mode->channels().size() == 2);
	QVERIFY(mode->channel(0) == ch1);
	QVERIFY(mode->channel(1) == ch3);
	QVERIFY(mode->channel(2) == NULL);
	QVERIFY(mode->channel(3) == NULL);

	/* Remove first channel. */
	mode->removeChannel(ch1);
	QVERIFY(mode->channels().size() == 1);
	QVERIFY(mode->channel(0) == ch3);
	QVERIFY(mode->channel(1) == NULL);
	QVERIFY(mode->channel(2) == NULL);
	QVERIFY(mode->channel(3) == NULL);

	/* Remove last channel. */
	mode->removeChannel(ch3);
	QVERIFY(mode->channels().size() == 0);
	QVERIFY(mode->channel(0) == NULL);
	QVERIFY(mode->channel(1) == NULL);
	QVERIFY(mode->channel(2) == NULL);
	QVERIFY(mode->channel(3) == NULL);

	delete mode;
	delete ch1;
	delete ch2;
	delete ch3;
	delete ch4;
}

void QLCFixtureMode_Test::channelByName()
{
	QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

	QLCChannel* ch1 = new QLCChannel();
	ch1->setName("ch1");
	mode->insertChannel(ch1, 0);

	QLCChannel* ch2 = new QLCChannel();
	ch2->setName("ch2");
	mode->insertChannel(ch2, 1);

	QLCChannel* ch3 = new QLCChannel();
	ch3->setName("ch3");
	mode->insertChannel(ch3, 2);

	QLCChannel* ch4 = new QLCChannel();
	ch4->setName("ch4");
	mode->insertChannel(ch4, 3);

	QVERIFY(mode->channel("ch1") == ch1);
	QVERIFY(mode->channel("ch2") == ch2);
	QVERIFY(mode->channel("ch3") == ch3);
	QVERIFY(mode->channel("ch4") == ch4);
	QVERIFY(mode->channel("ch12") == NULL);
	QVERIFY(mode->channel("") == NULL);

	delete mode;
	delete ch1;
	delete ch2;
	delete ch3;
	delete ch4;
}

void QLCFixtureMode_Test::channelByIndex()
{
	QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

	QLCChannel* ch1 = new QLCChannel();
	mode->insertChannel(ch1, 0);

	QLCChannel* ch2 = new QLCChannel();
	mode->insertChannel(ch2, 1);

	QLCChannel* ch3 = new QLCChannel();
	mode->insertChannel(ch3, 2);

	QLCChannel* ch4 = new QLCChannel();
	mode->insertChannel(ch4, 3);

	QVERIFY(mode->channel(0) == ch1);
	QVERIFY(mode->channel(1) == ch2);
	QVERIFY(mode->channel(2) == ch3);
	QVERIFY(mode->channel(3) == ch4);
	QVERIFY(mode->channel(12) == NULL);

	delete mode;
	delete ch1;
	delete ch2;
	delete ch3;
	delete ch4;
}

void QLCFixtureMode_Test::channels()
{
	QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
	QVERIFY(mode->channels().size() == 0);

	QLCChannel* ch1 = new QLCChannel();
	mode->insertChannel(ch1, 0);
	QVERIFY(mode->channels().size() == 1);

	QLCChannel* ch2 = new QLCChannel();
	mode->insertChannel(ch2, 1);
	QVERIFY(mode->channels().size() == 2);

	delete mode;
	delete ch1;
	delete ch2;
}

void QLCFixtureMode_Test::channelNumber()
{
	QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

	QLCChannel* ch1 = new QLCChannel();
	ch1->setName("ch1");
	mode->insertChannel(ch1, 0);

	QLCChannel* ch2 = new QLCChannel();
	ch2->setName("ch2");
	mode->insertChannel(ch2, 1);

	QLCChannel* ch3 = new QLCChannel();
	ch3->setName("ch3");
	mode->insertChannel(ch3, 2);

	QLCChannel* ch4 = new QLCChannel();
	ch4->setName("ch4");
	mode->insertChannel(ch4, 3);

	QVERIFY(mode->channelNumber(ch1) == 0);
	QVERIFY(mode->channelNumber(ch2) == 1);
	QVERIFY(mode->channelNumber(ch3) == 2);
	QVERIFY(mode->channelNumber(ch4) == 3);

	QLCChannel* ch5 = new QLCChannel();
	QVERIFY(mode->channelNumber(ch5) == KChannelInvalid);
	QVERIFY(mode->channelNumber(NULL) == KChannelInvalid);

	delete mode;
	delete ch1;
	delete ch2;
	delete ch3;
	delete ch4;
	delete ch5;
}

void QLCFixtureMode_Test::copy()
{
	qWarning() << "TODO";
}

void QLCFixtureMode_Test::cleanupTestCase()
{
	QVERIFY(m_fixtureDef != NULL);
	delete m_fixtureDef;
	m_fixtureDef = NULL;
}
