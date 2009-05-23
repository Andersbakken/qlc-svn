#include <QPointer>
#include <QtTest>

#include "../qlcchannel_test.h"
#include "../qlccapability.h"
#include "../qlcchannel.h"

void QLCChannel_Test::groupList()
{
	QStringList list(QLCChannel::groupList());

	QCOMPARE(list[0], KQLCChannelGroupBeam);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupBeam), 0);
	QCOMPARE(QLCChannel::indexToGroup(0), KQLCChannelGroupBeam);

	QCOMPARE(list[1], KQLCChannelGroupColour);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupColour), 1);
	QCOMPARE(QLCChannel::indexToGroup(1), KQLCChannelGroupColour);

	QCOMPARE(list[2], KQLCChannelGroupEffect);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupEffect), 2);
	QCOMPARE(QLCChannel::indexToGroup(2), KQLCChannelGroupEffect);

	QCOMPARE(list[3], KQLCChannelGroupGobo);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupGobo), 3);
	QCOMPARE(QLCChannel::indexToGroup(3), KQLCChannelGroupGobo);

	QCOMPARE(list[4], KQLCChannelGroupIntensity);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupIntensity), 4);
	QCOMPARE(QLCChannel::indexToGroup(4), KQLCChannelGroupIntensity);

	QCOMPARE(list[5], KQLCChannelGroupMaintenance);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupMaintenance), 5);
	QCOMPARE(QLCChannel::indexToGroup(5), KQLCChannelGroupMaintenance);

	QCOMPARE(list[6], KQLCChannelGroupNothing);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupNothing), 6);
	QCOMPARE(QLCChannel::indexToGroup(6), KQLCChannelGroupNothing);

	QCOMPARE(list[7], KQLCChannelGroupPan);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupPan), 7);
	QCOMPARE(QLCChannel::indexToGroup(7), KQLCChannelGroupPan);

	QCOMPARE(list[8], KQLCChannelGroupPrism);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupPrism), 8);
	QCOMPARE(QLCChannel::indexToGroup(8), KQLCChannelGroupPrism);

	QCOMPARE(list[9], KQLCChannelGroupShutter);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupShutter), 9);
	QCOMPARE(QLCChannel::indexToGroup(9), KQLCChannelGroupShutter);

	QCOMPARE(list[10], KQLCChannelGroupSpeed);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupSpeed), 10);
	QCOMPARE(QLCChannel::indexToGroup(10), KQLCChannelGroupSpeed);

	QCOMPARE(list[11], KQLCChannelGroupTilt);
	QCOMPARE(QLCChannel::groupToIndex(KQLCChannelGroupTilt), 11);
	QCOMPARE(QLCChannel::indexToGroup(11), KQLCChannelGroupTilt);
}

void QLCChannel_Test::name()
{
	/* Verify that a name can be set & get for the channel */
	QLCChannel c;
	QVERIFY(c.name() == QString::null);
	c.setName("Channel");
	QVERIFY(c.name() == "Channel");
}

void QLCChannel_Test::group()
{
	QLCChannel c;
	QVERIFY(c.group() == KQLCChannelGroupIntensity);
	c.setGroup("TestGroup");
	QVERIFY(c.group() == "TestGroup");
}

void QLCChannel_Test::controlByte()
{
	QLCChannel c;
	QVERIFY(c.controlByte() == 0);
	c.setControlByte(1);
	QVERIFY(c.controlByte() == 1);
}

void QLCChannel_Test::searchCapabilityByValue()
{
	QLCChannel c;
	QVERIFY(c.capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
	QVERIFY(c.addCapability(cap1) == true);
	QVERIFY(c.capabilities().size() == 1);

	QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
	QVERIFY(c.addCapability(cap2) == true);
	QVERIFY(c.capabilities().size() == 2);

	QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
	QVERIFY(c.addCapability(cap3) == true);
	QVERIFY(c.capabilities().size() == 3);

	QVERIFY(c.searchCapability(0) == cap1);
	QVERIFY(c.searchCapability(1) == cap1);
	QVERIFY(c.searchCapability(2) == cap1);
	QVERIFY(c.searchCapability(3) == cap1);
	QVERIFY(c.searchCapability(4) == cap1);
	QVERIFY(c.searchCapability(5) == cap1);
	QVERIFY(c.searchCapability(6) == cap1);
	QVERIFY(c.searchCapability(7) == cap1);
	QVERIFY(c.searchCapability(8) == cap1);
	QVERIFY(c.searchCapability(9) == cap1);

	QVERIFY(c.searchCapability(10) == cap2);
	QVERIFY(c.searchCapability(11) == cap2);
	QVERIFY(c.searchCapability(12) == cap2);
	QVERIFY(c.searchCapability(13) == cap2);
	QVERIFY(c.searchCapability(14) == cap2);
	QVERIFY(c.searchCapability(15) == cap2);
	QVERIFY(c.searchCapability(16) == cap2);
	QVERIFY(c.searchCapability(17) == cap2);
	QVERIFY(c.searchCapability(18) == cap2);
	QVERIFY(c.searchCapability(19) == cap2);

	QVERIFY(c.searchCapability(30) == NULL);
}

void QLCChannel_Test::searchCapabilityByName()
{
	QLCChannel c;
	QVERIFY(c.capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
	QVERIFY(c.addCapability(cap1) == true);

	QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
	QVERIFY(c.addCapability(cap2) == true);

	QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
	QVERIFY(c.addCapability(cap3) == true);

	QVERIFY(c.searchCapability("0-9") == cap1);
	QVERIFY(c.searchCapability("10-19") == cap2);
	QVERIFY(c.searchCapability("20-29") == cap3);
	QVERIFY(c.searchCapability("foo") == NULL);
}

void QLCChannel_Test::addCapability()
{
	QLCChannel c;
	QVERIFY(c.capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(15, 19, "15-19");
	QVERIFY(c.addCapability(cap1) == true);
	QVERIFY(c.capabilities().size() == 1);
	QVERIFY(c.capabilities()[0] == cap1);

	QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
	QVERIFY(c.addCapability(cap2) == true);
	QVERIFY(c.capabilities().size() == 2);
	QVERIFY(c.capabilities()[0] == cap1);
	QVERIFY(c.capabilities()[1] == cap2);

	/* Completely overlapping with cap2 */
	QLCCapability* cap3 = new QLCCapability(5, 6, "5-6");
	QVERIFY(c.addCapability(cap3) == false);
	delete cap3;

	/* Partially overlapping from low-end with cap1 */
	QLCCapability* cap4 = new QLCCapability(19, 25, "19-25");
	QVERIFY(c.addCapability(cap4) == false);
	delete cap4;

	/* Partially overlapping from high end with cap1 */
	QLCCapability* cap5 = new QLCCapability(10, 15, "10-15");
	QVERIFY(c.addCapability(cap5) == false);
	delete cap5;

	/* Partially overlapping with two ranges at both ends (cap1 & cap2) */
	QLCCapability* cap6 = new QLCCapability(8, 16, "8-16");
	QVERIFY(c.addCapability(cap6) == false);
	delete cap6;

	/* Completely containing cap1 */
	QLCCapability* cap7 = new QLCCapability(14, 20, "14-20");
	QVERIFY(c.addCapability(cap7) == false);
	delete cap7;

	/* Non-overlapping, between cap1 & cap2*/
	QLCCapability* cap8 = new QLCCapability(10, 14, "10-14");
	QVERIFY(c.addCapability(cap8) == true);
	delete cap8;
}

void QLCChannel_Test::removeCapability()
{
	QLCChannel c;
	QVERIFY(c.capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(10, 20, "10-20");
	QVERIFY(c.addCapability(cap1) == true);
	QVERIFY(c.capabilities().size() == 1);

	QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
	QVERIFY(c.addCapability(cap2) == true);
	QVERIFY(c.capabilities().size() == 2);

	QVERIFY(c.removeCapability(cap2) == true);
	QVERIFY(c.capabilities().size() == 1);

	QVERIFY(c.removeCapability(cap2) == false);
	QVERIFY(c.capabilities().size() == 1);

	QVERIFY(c.removeCapability(cap1) == true);
	QVERIFY(c.capabilities().size() == 0);
}

void QLCChannel_Test::sortCapabilities()
{
}

void QLCChannel_Test::copy()
{
}
