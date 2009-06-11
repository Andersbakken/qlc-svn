#include <QPointer>
#include <QtTest>
#include <QtXml>

#include "qlcchannel_test.h"
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
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->name() == QString::null);

	channel->setName("Channel");
	QVERIFY(channel->name() == "Channel");

	delete channel;
}

void QLCChannel_Test::group()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->group() == KQLCChannelGroupIntensity);

	channel->setGroup("TestGroup");
	QVERIFY(channel->group() == "TestGroup");

	delete channel;
}

void QLCChannel_Test::controlByte()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->controlByte() == 0);

	channel->setControlByte(1);
	QVERIFY(channel->controlByte() == 1);

	delete channel;
}

void QLCChannel_Test::searchCapabilityByValue()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap1) == true);
	QVERIFY(channel->capabilities().size() == 1);

	QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
	QVERIFY(channel->addCapability(cap2) == true);
	QVERIFY(channel->capabilities().size() == 2);

	QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
	QVERIFY(channel->addCapability(cap3) == true);
	QVERIFY(channel->capabilities().size() == 3);

	QVERIFY(channel->searchCapability(0) == cap1);
	QVERIFY(channel->searchCapability(1) == cap1);
	QVERIFY(channel->searchCapability(2) == cap1);
	QVERIFY(channel->searchCapability(3) == cap1);
	QVERIFY(channel->searchCapability(4) == cap1);
	QVERIFY(channel->searchCapability(5) == cap1);
	QVERIFY(channel->searchCapability(6) == cap1);
	QVERIFY(channel->searchCapability(7) == cap1);
	QVERIFY(channel->searchCapability(8) == cap1);
	QVERIFY(channel->searchCapability(9) == cap1);

	QVERIFY(channel->searchCapability(10) == cap2);
	QVERIFY(channel->searchCapability(11) == cap2);
	QVERIFY(channel->searchCapability(12) == cap2);
	QVERIFY(channel->searchCapability(13) == cap2);
	QVERIFY(channel->searchCapability(14) == cap2);
	QVERIFY(channel->searchCapability(15) == cap2);
	QVERIFY(channel->searchCapability(16) == cap2);
	QVERIFY(channel->searchCapability(17) == cap2);
	QVERIFY(channel->searchCapability(18) == cap2);
	QVERIFY(channel->searchCapability(19) == cap2);

	QVERIFY(channel->searchCapability(30) == NULL);

	delete channel;
}

void QLCChannel_Test::searchCapabilityByName()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap1) == true);

	QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
	QVERIFY(channel->addCapability(cap2) == true);

	QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
	QVERIFY(channel->addCapability(cap3) == true);

	QVERIFY(channel->searchCapability("0-9") == cap1);
	QVERIFY(channel->searchCapability("10-19") == cap2);
	QVERIFY(channel->searchCapability("20-29") == cap3);
	QVERIFY(channel->searchCapability("foo") == NULL);

	delete channel;
}

void QLCChannel_Test::addCapability()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(15, 19, "15-19");
	QVERIFY(channel->addCapability(cap1) == true);
	QVERIFY(channel->capabilities().size() == 1);
	QVERIFY(channel->capabilities()[0] == cap1);

	QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap2) == true);
	QVERIFY(channel->capabilities().size() == 2);
	QVERIFY(channel->capabilities()[0] == cap1);
	QVERIFY(channel->capabilities()[1] == cap2);

	/* Completely overlapping with cap2 */
	QLCCapability* cap3 = new QLCCapability(5, 6, "5-6");
	QVERIFY(channel->addCapability(cap3) == false);
	delete cap3;
	cap3 = NULL;

	/* Partially overlapping from low-end with cap1 */
	QLCCapability* cap4 = new QLCCapability(19, 25, "19-25");
	QVERIFY(channel->addCapability(cap4) == false);
	delete cap4;
	cap4 = NULL;

	/* Partially overlapping from high end with cap1 */
	QLCCapability* cap5 = new QLCCapability(10, 15, "10-15");
	QVERIFY(channel->addCapability(cap5) == false);
	delete cap5;
	cap5 = NULL;

	/* Partially overlapping with two ranges at both ends (cap1 & cap2) */
	QLCCapability* cap6 = new QLCCapability(8, 16, "8-16");
	QVERIFY(channel->addCapability(cap6) == false);
	delete cap6;
	cap6 = NULL;

	/* Completely containing cap1 */
	QLCCapability* cap7 = new QLCCapability(14, 20, "14-20");
	QVERIFY(channel->addCapability(cap7) == false);
	delete cap7;
	cap7 = NULL;

	/* Non-overlapping, between cap1 & cap2*/
	QLCCapability* cap8 = new QLCCapability(10, 14, "10-14");
	QVERIFY(channel->addCapability(cap8) == true);
	/* Don't delete cap8 because it's now a member of the channel and gets
	   deleted from the channel's destructor. */

	delete channel;
}

void QLCChannel_Test::removeCapability()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(10, 20, "10-20");
	QVERIFY(channel->addCapability(cap1) == true);
	QVERIFY(channel->capabilities().size() == 1);

	QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap2) == true);
	QVERIFY(channel->capabilities().size() == 2);

	QVERIFY(channel->removeCapability(cap2) == true);
	QVERIFY(channel->capabilities().size() == 1);
	/* cap2 is deleted by QLCChannel::removeCapability() */

	QVERIFY(channel->removeCapability(cap2) == false);
	QVERIFY(channel->capabilities().size() == 1);

	QVERIFY(channel->removeCapability(cap1) == true);
	QVERIFY(channel->capabilities().size() == 0);
	/* cap1 is deleted by QLCChannel::removeCapability() */

	delete channel;
}

void QLCChannel_Test::sortCapabilities()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	QLCCapability* cap1 = new QLCCapability(10, 19, "10-19");
	QVERIFY(channel->addCapability(cap1) == true);

	QLCCapability* cap2 = new QLCCapability(50, 59, "50-59");
	QVERIFY(channel->addCapability(cap2) == true);

	QLCCapability* cap3 = new QLCCapability(40, 49, "40-49");
	QVERIFY(channel->addCapability(cap3) == true);

	QLCCapability* cap4 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap4) == true);

	QLCCapability* cap5 = new QLCCapability(200, 209, "200-209");
	QVERIFY(channel->addCapability(cap5) == true);

	QLCCapability* cap6 = new QLCCapability(30, 39, "30-39");
	QVERIFY(channel->addCapability(cap6) == true);

	QLCCapability* cap7 = new QLCCapability(26, 29, "26-29");
	QVERIFY(channel->addCapability(cap7) == true);

	QLCCapability* cap8 = new QLCCapability(20, 25, "20-25");
	QVERIFY(channel->addCapability(cap8) == true);

	QList <QLCCapability*> orig(channel->capabilities());
	QVERIFY(orig.at(0) == cap1);
	QVERIFY(orig.at(1) == cap2);
	QVERIFY(orig.at(2) == cap3);
	QVERIFY(orig.at(3) == cap4);
	QVERIFY(orig.at(4) == cap5);
	QVERIFY(orig.at(5) == cap6);
	QVERIFY(orig.at(6) == cap7);
	QVERIFY(orig.at(7) == cap8);

	channel->sortCapabilities();

	QList <QLCCapability*> sorted(channel->capabilities());
	QVERIFY(sorted.at(0) == cap4);
	QVERIFY(sorted.at(1) == cap1);
	QVERIFY(sorted.at(2) == cap8);
	QVERIFY(sorted.at(3) == cap7);
	QVERIFY(sorted.at(4) == cap6);
	QVERIFY(sorted.at(5) == cap3);
	QVERIFY(sorted.at(6) == cap2);
	QVERIFY(sorted.at(7) == cap5);

	delete channel;
}

void QLCChannel_Test::copy()
{
	QLCChannel* channel = new QLCChannel();
	QVERIFY(channel->capabilities().size() == 0);

	channel->setName("Foobar");
	channel->setGroup("Tilt");
	channel->setControlByte(3);
	
	QLCCapability* cap1 = new QLCCapability(10, 19, "10-19");
	QVERIFY(channel->addCapability(cap1) == true);

	QLCCapability* cap2 = new QLCCapability(50, 59, "50-59");
	QVERIFY(channel->addCapability(cap2) == true);

	QLCCapability* cap3 = new QLCCapability(40, 49, "40-49");
	QVERIFY(channel->addCapability(cap3) == true);

	QLCCapability* cap4 = new QLCCapability(0, 9, "0-9");
	QVERIFY(channel->addCapability(cap4) == true);

	QLCCapability* cap5 = new QLCCapability(200, 209, "200-209");
	QVERIFY(channel->addCapability(cap5) == true);

	QLCCapability* cap6 = new QLCCapability(30, 39, "30-39");
	QVERIFY(channel->addCapability(cap6) == true);

	QLCCapability* cap7 = new QLCCapability(26, 29, "26-29");
	QVERIFY(channel->addCapability(cap7) == true);

	QLCCapability* cap8 = new QLCCapability(20, 25, "20-25");
	QVERIFY(channel->addCapability(cap8) == true);

	/* Create a copy of the original channel */
	QLCChannel* copy = new QLCChannel(channel);

	QVERIFY(copy->name() == "Foobar");
	QVERIFY(copy->group() == "Tilt");
	QVERIFY(copy->controlByte() == 3);

	/* Verify that the capabilities in the copied channel are also
	   copies i.e. their pointers are not the same as the originals. */
	QList <QLCCapability*> caps(copy->capabilities());
	QVERIFY(caps.size() == 8);
	QVERIFY(caps.at(0) != cap1);
	QVERIFY(caps.at(0)->name() == cap1->name());
	QVERIFY(caps.at(0)->min() == cap1->min());
	QVERIFY(caps.at(0)->max() == cap1->max());

	QVERIFY(caps.at(1) != cap2);
	QVERIFY(caps.at(1)->name() == cap2->name());
	QVERIFY(caps.at(1)->min() == cap2->min());
	QVERIFY(caps.at(1)->max() == cap2->max());

	QVERIFY(caps.at(2) != cap3);
	QVERIFY(caps.at(2)->name() == cap3->name());
	QVERIFY(caps.at(2)->min() == cap3->min());
	QVERIFY(caps.at(2)->max() == cap3->max());

	QVERIFY(caps.at(3) != cap4);
	QVERIFY(caps.at(3)->name() == cap4->name());
	QVERIFY(caps.at(3)->min() == cap4->min());
	QVERIFY(caps.at(3)->max() == cap4->max());

	QVERIFY(caps.at(4) != cap5);
	QVERIFY(caps.at(4)->name() == cap5->name());
	QVERIFY(caps.at(4)->min() == cap5->min());
	QVERIFY(caps.at(4)->max() == cap5->max());

	QVERIFY(caps.at(5) != cap6);
	QVERIFY(caps.at(5)->name() == cap6->name());
	QVERIFY(caps.at(5)->min() == cap6->min());
	QVERIFY(caps.at(5)->max() == cap6->max());

	QVERIFY(caps.at(6) != cap7);
	QVERIFY(caps.at(6)->name() == cap7->name());
	QVERIFY(caps.at(6)->min() == cap7->min());
	QVERIFY(caps.at(6)->max() == cap7->max());

	QVERIFY(caps.at(7) != cap8);
	QVERIFY(caps.at(7)->name() == cap8->name());
	QVERIFY(caps.at(7)->min() == cap8->min());
	QVERIFY(caps.at(7)->max() == cap8->max());
}

void QLCChannel_Test::load()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Channel");
	root.setAttribute("Name", "Channel1");
	doc.appendChild(root);

	QDomElement group = doc.createElement("Group");
	root.appendChild(group);
	group.setAttribute("Byte", 1);
	QDomText groupName = doc.createTextNode("Tilt");
	group.appendChild(groupName);

	QDomElement cap1 = doc.createElement("Capability");
	root.appendChild(cap1);
	cap1.setAttribute("Min", 0);
	cap1.setAttribute("Max", 10);
	QDomText cap1name = doc.createTextNode("Cap1");
	cap1.appendChild(cap1name);

	/* Overlaps with cap1, shouldn't appear in the channel */
	QDomElement cap2 = doc.createElement("Capability");
	root.appendChild(cap2);
	cap2.setAttribute("Min", 5);
	cap2.setAttribute("Max", 15);
	QDomText cap2name = doc.createTextNode("Cap2");
	cap2.appendChild(cap2name);

	QDomElement cap3 = doc.createElement("Capability");
	root.appendChild(cap3);
	cap3.setAttribute("Min", 11);
	cap3.setAttribute("Max", 20);
	QDomText cap3name = doc.createTextNode("Cap3");
	cap3.appendChild(cap3name);

	/* Invalid capability tag, shouldn't appear in the channel, since it
	   is not recognized by the channel. */
	QDomElement cap4 = doc.createElement("apability");
	root.appendChild(cap4);
	cap4.setAttribute("Min", 21);
	cap4.setAttribute("Max", 30);
	QDomText cap4name = doc.createTextNode("Cap4");
	cap4.appendChild(cap4name);

	/* Missing minimum value, shouldn't appear in the channel, because
	   loadXML() fails. */
	QDomElement cap5 = doc.createElement("Capability");
	root.appendChild(cap5);
	cap5.setAttribute("Max", 30);
	QDomText cap5name = doc.createTextNode("Cap5");
	cap5.appendChild(cap5name);

	QLCChannel ch;
	QVERIFY(ch.loadXML(&root) == true);
	QVERIFY(ch.name() == "Channel1");
	QVERIFY(ch.group() == "Tilt");
	QVERIFY(ch.controlByte() == 1);
	QVERIFY(ch.capabilities().size() == 2);
	QVERIFY(ch.capabilities()[0]->name() == "Cap1");
	QVERIFY(ch.capabilities()[1]->name() == "Cap3");
}

void QLCChannel_Test::loadWrongRoot()
{
	QDomDocument doc;

	QDomElement root = doc.createElement("Chanel");
	root.setAttribute("Name", "Channel1");
	doc.appendChild(root);

	QDomElement group = doc.createElement("Group");
	root.appendChild(group);
	group.setAttribute("Byte", 1);
	QDomText groupName = doc.createTextNode("Tilt");
	group.appendChild(groupName);

	QDomElement cap1 = doc.createElement("Capability");
	root.appendChild(cap1);
	cap1.setAttribute("Min", 0);
	cap1.setAttribute("Max", 10);
	QDomText cap1name = doc.createTextNode("Cap1");
	cap1.appendChild(cap1name);

	/* Overlaps with cap1, shouldn't appear in the channel */
	QDomElement cap2 = doc.createElement("Capability");
	root.appendChild(cap2);
	cap2.setAttribute("Min", 5);
	cap2.setAttribute("Max", 15);
	QDomText cap2name = doc.createTextNode("Cap2");
	cap2.appendChild(cap2name);

	QDomElement cap3 = doc.createElement("Capability");
	root.appendChild(cap3);
	cap3.setAttribute("Min", 11);
	cap3.setAttribute("Max", 20);
	QDomText cap3name = doc.createTextNode("Cap3");
	cap3.appendChild(cap3name);

	QLCChannel ch;
	QVERIFY(ch.loadXML(&root) == false);
	QVERIFY(ch.name() == QString::null);
	QVERIFY(ch.group() == KQLCChannelGroupIntensity);
	QVERIFY(ch.controlByte() == 0);
	QVERIFY(ch.capabilities().size() == 0);
}
