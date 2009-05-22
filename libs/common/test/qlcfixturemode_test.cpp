#include <QtTest>

#include "../qlcfixturemode_test.h"
#include "../qlcfixturemode.h"

void QLCFixtureMode_Test::name()
{
	/* Verify that a name can be set & get for the mode */
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));
	QVERIFY(m.name() == QString::null);
	m.setName("Normal");
	QVERIFY(m.name() == "Normal");
}

void QLCFixtureMode_Test::physical()
{
	/* Verify that a QLCPhysical can be set & get for the mode */
	QLCPhysical p;
	p.setBulbType("Foobar");

	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));
	QVERIFY(m.physical().bulbType() == QString::null);

	m.setPhysical(p);
	QVERIFY(m.physical().bulbType() == "Foobar");
}

void QLCFixtureMode_Test::insertChannel()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));

	/* First channel */
	QLCChannel ch1;
	m.insertChannel(&ch1, 0);
	QVERIFY(m.channel(0) == &ch1);

	/* Second prepended */
	QLCChannel ch2;
	m.insertChannel(&ch2, 0);
	QVERIFY(m.channel(0) == &ch2);
	QVERIFY(m.channel(1) == &ch1);

	/* Third appended way over the end */
	QLCChannel ch3;
	m.insertChannel(&ch3, 10);
	QVERIFY(m.channel(0) == &ch2);
	QVERIFY(m.channel(1) == &ch1);
	QVERIFY(m.channel(2) == &ch3);

	/* Fourth inserted in-between */
	QLCChannel ch4;
	m.insertChannel(&ch4, 1);
	QVERIFY(m.channel(0) == &ch2);
	QVERIFY(m.channel(1) == &ch4);
	QVERIFY(m.channel(2) == &ch1);
	QVERIFY(m.channel(3) == &ch3);
}

void QLCFixtureMode_Test::removeChannel()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));

	QLCChannel ch1;
	m.insertChannel(&ch1, 0);
	QLCChannel ch2;
	m.insertChannel(&ch2, 1);
	QLCChannel ch3;
	m.insertChannel(&ch3, 2);
	QLCChannel ch4;
	m.insertChannel(&ch4, 3);

	/* Remove one channel in the middle */
	QVERIFY(m.channels() == 4);
	m.removeChannel(&ch2);
	QVERIFY(m.channels() == 3);
	QVERIFY(m.channel(0) == &ch1);
	QVERIFY(m.channel(1) == &ch3);
	QVERIFY(m.channel(2) == &ch4);
	QVERIFY(m.channel(3) == NULL);

	/* Remove the same channel again. Shouldn't change anything. */
	m.removeChannel(&ch2);
	QVERIFY(m.channels() == 3);
	QVERIFY(m.channel(0) == &ch1);
	QVERIFY(m.channel(1) == &ch3);
	QVERIFY(m.channel(2) == &ch4);
	QVERIFY(m.channel(3) == NULL);

	/* Remove last channel. */
	m.removeChannel(&ch4);
	QVERIFY(m.channels() == 2);
	QVERIFY(m.channel(0) == &ch1);
	QVERIFY(m.channel(1) == &ch3);
	QVERIFY(m.channel(2) == NULL);
	QVERIFY(m.channel(3) == NULL);

	/* Remove first channel. */
	m.removeChannel(&ch1);
	QVERIFY(m.channels() == 1);
	QVERIFY(m.channel(0) == &ch3);
	QVERIFY(m.channel(1) == NULL);
	QVERIFY(m.channel(2) == NULL);
	QVERIFY(m.channel(3) == NULL);

	/* Remove last channel. */
	m.removeChannel(&ch3);
	QVERIFY(m.channels() == 0);
	QVERIFY(m.channel(0) == NULL);
	QVERIFY(m.channel(1) == NULL);
	QVERIFY(m.channel(2) == NULL);
	QVERIFY(m.channel(3) == NULL);
}

void QLCFixtureMode_Test::channelByName()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));

	QLCChannel ch1;
	ch1.setName("ch1");
	m.insertChannel(&ch1, 0);

	QLCChannel ch2;
	ch2.setName("ch2");
	m.insertChannel(&ch2, 1);

	QLCChannel ch3;
	ch3.setName("ch3");
	m.insertChannel(&ch3, 2);

	QLCChannel ch4;
	ch4.setName("ch4");
	m.insertChannel(&ch4, 3);

	QVERIFY(m.channel("ch1") == &ch1);
	QVERIFY(m.channel("ch2") == &ch2);
	QVERIFY(m.channel("ch3") == &ch3);
	QVERIFY(m.channel("ch4") == &ch4);
	QVERIFY(m.channel("ch12") == NULL);
	QVERIFY(m.channel("") == NULL);
}

void QLCFixtureMode_Test::channelByIndex()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));

	QLCChannel ch1;
	m.insertChannel(&ch1, 0);

	QLCChannel ch2;
	m.insertChannel(&ch2, 1);

	QLCChannel ch3;
	m.insertChannel(&ch3, 2);

	QLCChannel ch4;
	m.insertChannel(&ch4, 3);

	QVERIFY(m.channel(0) == &ch1);
	QVERIFY(m.channel(1) == &ch2);
	QVERIFY(m.channel(2) == &ch3);
	QVERIFY(m.channel(3) == &ch4);
	QVERIFY(m.channel(12) == NULL);
	QVERIFY(m.channel(-1) == NULL);
}

void QLCFixtureMode_Test::channels()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));
	
	QVERIFY(m.channels() == 0);

	m.insertChannel(NULL, 0);
	QVERIFY(m.channels() == 0);

	QLCChannel ch1;
	m.insertChannel(&ch1, 0);
	QVERIFY(m.channels() == 1);

	QLCChannel ch2;
	m.insertChannel(&ch2, 1);
	QVERIFY(m.channels() == 2);
}

void QLCFixtureMode_Test::channelNumber()
{
	QLCFixtureMode m(reinterpret_cast<QLCFixtureMode*> (NULL));

	QLCChannel ch1;
	ch1.setName("ch1");
	m.insertChannel(&ch1, 0);

	QLCChannel ch2;
	ch2.setName("ch2");
	m.insertChannel(&ch2, 1);

	QLCChannel ch3;
	ch3.setName("ch3");
	m.insertChannel(&ch3, 2);

	QLCChannel ch4;
	ch4.setName("ch4");
	m.insertChannel(&ch4, 3);

	QVERIFY(m.channelNumber(&ch1) == 0);
	QVERIFY(m.channelNumber(&ch2) == 1);
	QVERIFY(m.channelNumber(&ch3) == 2);
	QVERIFY(m.channelNumber(&ch4) == 3);

	QLCChannel ch5;
	QVERIFY(m.channelNumber(&ch5) == KChannelInvalid);
	QVERIFY(m.channelNumber(NULL) == KChannelInvalid);
}
