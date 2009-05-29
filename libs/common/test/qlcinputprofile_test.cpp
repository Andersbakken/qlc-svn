#include <QtTest>

#include "qlcinputprofile_test.h"
#include "../qlcinputprofile.h"
#include "../qlcinputchannel.h"

void QLCInputProfile_Test::manufacturer()
{
	QLCInputProfile ip;
	QVERIFY(ip.manufacturer() == QString::null);
	ip.setManufacturer("Behringer");
	QVERIFY(ip.manufacturer() == "Behringer");
}

void QLCInputProfile_Test::model()
{
	QLCInputProfile ip;
	QVERIFY(ip.model() == QString::null);
	ip.setModel("BCF2000");
	QVERIFY(ip.model() == "BCF2000");
}

void QLCInputProfile_Test::name()
{
	QLCInputProfile ip;
	QVERIFY(ip.name() == " ");
	ip.setManufacturer("Behringer");
	QVERIFY(ip.name() == "Behringer ");
	ip.setModel("BCF2000");
	QVERIFY(ip.name() == "Behringer BCF2000");
}

void QLCInputProfile_Test::addChannel()
{
	QLCInputProfile ip;

	QLCInputChannel* ich1 = new QLCInputChannel();
	ip.insertChannel(0, ich1);
	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channels().size() == 1);

	/* Shouldn't overwrite the existing mapping */
	QLCInputChannel* ich2 = new QLCInputChannel();
	ip.insertChannel(0, ich2);
	QVERIFY(ip.channel(0) == ich1);

	ip.insertChannel(5, ich2);
	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channel(1) == NULL);
	QVERIFY(ip.channel(2) == NULL);
	QVERIFY(ip.channel(3) == NULL);
	QVERIFY(ip.channel(4) == NULL);
	QVERIFY(ip.channel(5) == ich2);
}

void QLCInputProfile_Test::removeChannel()
{
	QLCInputProfile ip;

	QLCInputChannel* ich1 = new QLCInputChannel();
	ip.insertChannel(0, ich1);
	QVERIFY(ip.channel(0) == ich1);

	QLCInputChannel* ich2 = new QLCInputChannel();
	ip.insertChannel(5, ich2);
	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channel(5) == ich2);

	QVERIFY(ip.removeChannel(1) == false);
	QVERIFY(ip.removeChannel(2) == false);
	QVERIFY(ip.removeChannel(3) == false);
	QVERIFY(ip.removeChannel(4) == false);

	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channel(5) == ich2);
	QVERIFY(ip.removeChannel(0) == true);
	QVERIFY(ip.channel(0) == NULL);
	QVERIFY(ip.channel(5) == ich2);
	QVERIFY(ip.removeChannel(5) == true);
	QVERIFY(ip.channel(0) == NULL);
	QVERIFY(ip.channel(5) == NULL);
}

void QLCInputProfile_Test::remapChannel()
{
	QLCInputProfile ip;

	QLCInputChannel* ich1 = new QLCInputChannel();
	ich1->setName("Foobar");
	ip.insertChannel(0, ich1);
	QVERIFY(ip.channel(0) == ich1);

	QLCInputChannel* ich2 = new QLCInputChannel();
	ip.insertChannel(5, ich2);
	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channel(5) == ich2);

	QVERIFY(ip.remapChannel(ich1, 9000) == true);
	QVERIFY(ip.channel(0) == NULL);
	QVERIFY(ip.channel(5) == ich2);
	QVERIFY(ip.channel(9000) == ich1);
	QVERIFY(ip.channel(9000)->name() == "Foobar");

	QVERIFY(ip.remapChannel(NULL, 9000) == false);
	QVERIFY(ip.channels().size() == 2);
	QVERIFY(ip.channel(0) == NULL);
	QVERIFY(ip.channel(5) == ich2);
	QVERIFY(ip.channel(9000) == ich1);
	QVERIFY(ip.channel(9000)->name() == "Foobar");

	QLCInputChannel* ich3 = new QLCInputChannel();
	QVERIFY(ip.remapChannel(ich3, 5) == false);
	QVERIFY(ip.channels().size() == 2);
	QVERIFY(ip.channel(0) == NULL);
	QVERIFY(ip.channel(5) == ich2);
	QVERIFY(ip.channel(9000) == ich1);
	QVERIFY(ip.channel(9000)->name() == "Foobar");
	
	delete ich3;
}

void QLCInputProfile_Test::channel()
{
	QLCInputProfile ip;

	QLCInputChannel* ich1 = new QLCInputChannel();
	ip.insertChannel(0, ich1);

	QLCInputChannel* ich2 = new QLCInputChannel();
	ip.insertChannel(5, ich2);

	QVERIFY(ip.channel(0) == ich1);
	QVERIFY(ip.channel(1) == NULL);
	QVERIFY(ip.channel(2) == NULL);
	QVERIFY(ip.channel(3) == NULL);
	QVERIFY(ip.channel(4) == NULL);
	QVERIFY(ip.channel(5) == ich2);
}

void QLCInputProfile_Test::channels()
{
	QLCInputProfile ip;
	QVERIFY(ip.channels().size() == 0);

	QLCInputChannel* ich1 = new QLCInputChannel();
	ip.insertChannel(0, ich1);

	QLCInputChannel* ich2 = new QLCInputChannel();
	ip.insertChannel(5, ich2);

	QVERIFY(ip.channels().size() == 2);
	QVERIFY(ip.channels().contains(0) == true);
	QVERIFY(ip.channels().contains(1) == false);
	QVERIFY(ip.channels().contains(2) == false);
	QVERIFY(ip.channels().contains(3) == false);
	QVERIFY(ip.channels().contains(4) == false);
	QVERIFY(ip.channels().contains(5) == true);
	QVERIFY(ip.channels()[0] == ich1);
	QVERIFY(ip.channels()[5] == ich2);
}

void QLCInputProfile_Test::copy()
{
	QLCInputProfile ip;
	ip.setManufacturer("Behringer");
	ip.setModel("BCF2000");

	QLCInputChannel* ich1 = new QLCInputChannel();
	ich1->setName("Channel 1");
	ip.insertChannel(0, ich1);

	QLCInputChannel* ich2 = new QLCInputChannel();
	ich2->setName("Channel 2");
	ip.insertChannel(5, ich2);

	QLCInputChannel* ich3 = new QLCInputChannel();
	ich3->setName("Channel 3");
	ip.insertChannel(2, ich3);

	QLCInputChannel* ich4 = new QLCInputChannel();
	ich4->setName("Channel 4");
	ip.insertChannel(9000, ich4);

	QLCInputProfile copy(ip);
	QVERIFY(copy.manufacturer() == "Behringer");
	QVERIFY(copy.model() == "BCF2000");

	QVERIFY(copy.channels().size() == 4);

	QVERIFY(copy.channel(0) != ich1);
	QVERIFY(copy.channel(0) != NULL);
	QVERIFY(copy.channel(0)->name() == "Channel 1");

	QVERIFY(copy.channel(5) != ich2);
	QVERIFY(copy.channel(5) != NULL);
	QVERIFY(copy.channel(5)->name() == "Channel 2");

	QVERIFY(copy.channel(2) != ich3);
	QVERIFY(copy.channel(2) != NULL);
	QVERIFY(copy.channel(2)->name() == "Channel 3");

	QVERIFY(copy.channel(9000) != ich4);
	QVERIFY(copy.channel(9000) != NULL);
	QVERIFY(copy.channel(9000)->name() == "Channel 4");
}

void QLCInputProfile_Test::load()
{
	qDebug() << "TODO";
}
