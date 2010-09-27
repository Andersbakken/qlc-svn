#include <QApplication>
#include <QUdpSocket>
#include <QByteArray>
#include <QtTest>

#include "eplaybackwing.h"
#include "testeplaybackwing.h"

#define PLB_FIRMWARE 192
#define PLB_FLAGS (1 << 7) /* Page Up */	\
		| (1 << 6) /* Page Down */	\
		| (1 << 5) /* Back */		\
		| (1 << 4) /* Go */		\
		| (0 << 1) /* Product (1=PLB, 2=SHC, 3=PGM) */	\
		| (1 << 0) /* Product */

QByteArray TestEPlaybackWing::data()
{
	QByteArray data;

	data.resize(28);
	data[0] = 'W'; /* HEADER */
	data[1] = 'O'; /* HEADER */
	data[2] = 'D'; /* HEADER */
	data[3] = 'D'; /* HEADER */

	data[4] = PLB_FIRMWARE; /* Firmware */

	data[5] = PLB_FLAGS; /* Flags */

	data[6] = 0; /* Unused */

	data[7] = 255; /* Buttons */
	data[8] = 255; /* Buttons */
	data[9] = 255; /* Buttons */
	data[10] = 255; /* Buttons */
	data[11] = 255; /* Buttons */

	data[12] = 0; /* Unused */
	data[13] = 0; /* Unused */
	data[14] = 0; /* Unused */

	data[15] = 0; /* Fader 0 */
	data[16] = 0; /* Fader 1 */
	data[17] = 0; /* Fader 2 */
	data[18] = 0; /* Fader 3 */
	data[19] = 0; /* Fader 4 */
	data[20] = 0; /* Fader 5 */
	data[21] = 0; /* Fader 6 */
	data[22] = 0; /* Fader 7 */
	data[23] = 0; /* Fader 8 */
	data[24] = 0; /* Fader 9 */

	data[25] = 0; /* Unused */
	data[26] = 0; /* Unused */
	data[27] = 0; /* Unused */

	return data;
}

void TestEPlaybackWing::initTestCase()
{
	m_ewing = new EPlaybackWing(this, QHostAddress::LocalHost, data());
	QVERIFY(m_ewing != NULL);
}

void TestEPlaybackWing::firmware()
{
	QVERIFY(m_ewing->firmware() == PLB_FIRMWARE);
}

void TestEPlaybackWing::address()
{
	QVERIFY(m_ewing->address() == QHostAddress::LocalHost);
}

void TestEPlaybackWing::isOutputData()
{
	QByteArray ba(data());

	QVERIFY(EWing::isOutputData(ba) == true);

	ba[1] = 'I';
	QVERIFY(EWing::isOutputData(ba) == false);
}

void TestEPlaybackWing::name()
{
	QCOMPARE(m_ewing->name(), QString("Playback ") + tr("at") + QString(" ")
		 + QHostAddress(QHostAddress::LocalHost).toString());
}

void TestEPlaybackWing::infoText()
{
	QString str = QString("<B>%1</B><BR>").arg(m_ewing->name());
	str += tr("Firmware version %1").arg(PLB_FIRMWARE);
	str += tr("<P>Device is operating correctly</P>");
	QCOMPARE(m_ewing->infoText(), str);
}

void TestEPlaybackWing::buttons_data()
{
	QByteArray ba(data());

	/* Create columns for a QByteArray that is fed to EWing::parseData()
	   on each row, the channel number to read and the value expected for
	   that channel after each parseData() call. */
	QTest::addColumn<QByteArray> ("ba");
	QTest::addColumn<int> ("channel");
	QTest::addColumn<int> ("value");

	/* First test that the button state is read as OFF, then set the
	   button's state ON in the byte array (simulating a UDP packet that
	   has been read from the network) and read the value again. Low bit (0)
	   means that the button is down, high bit (1) means it's up. */

	/*             ROW NAME     DATA   CH   VAL */
	QTest::newRow("Button 0") << ba << 10 << 0;
	ba[11] = 127; /* 0111 1111 */
	QTest::newRow("Button 0") << ba << 10 << 255;

	QTest::newRow("Button 1") << ba << 11 << 0;
	ba[11] = 191; /* 1011 1111 */
	QTest::newRow("Button 1") << ba << 11 << 255;

	QTest::newRow("Button 2") << ba << 12 << 0;
	ba[11] = 223; /* 1101 1111 */
	QTest::newRow("Button 2") << ba << 12 << 255;

	QTest::newRow("Button 3") << ba << 13 << 0;
	ba[11] = 239; /* 1110 1111 */
	QTest::newRow("Button 3") << ba << 13 << 255;

	QTest::newRow("Button 4") << ba << 14 << 0;
	ba[11] = 247; /* 1111 0111 */
	QTest::newRow("Button 4") << ba << 14 << 255;

	QTest::newRow("Button 5") << ba << 15 << 0;
	ba[11] = 251; /* 1111 1011 */
	QTest::newRow("Button 5") << ba << 15 << 255;

	QTest::newRow("Button 6") << ba << 16 << 0;
	ba[11] = 253; /* 1111 1101 */
	QTest::newRow("Button 6") << ba << 16 << 255;

	QTest::newRow("Button 7") << ba << 17 << 0;
	ba[11] = 254; /* 1111 1110 */
	QTest::newRow("Button 7") << ba << 17 << 255;

	QTest::newRow("Button 8") << ba << 18 << 0;
	ba[10] = 127; /* 0111 1111 */
	QTest::newRow("Button 8") << ba << 18 << 255;

	QTest::newRow("Button 9") << ba << 19 << 0;
	ba[10] = 191; /* 1011 1111 */
	QTest::newRow("Button 9") << ba << 19 << 255;

	QTest::newRow("Button 10") << ba << 20 << 0;
	ba[10] = 223; /* 1101 1111 */
	QTest::newRow("Button 10") << ba << 20 << 255;

	QTest::newRow("Button 11") << ba << 21 << 0;
	ba[10] = 239; /* 1110 1111 */
	QTest::newRow("Button 11") << ba << 21 << 255;

	QTest::newRow("Button 12") << ba << 22 << 0;
	ba[10] = 247; /* 1111 0111 */
	QTest::newRow("Button 12") << ba << 22 << 255;

	QTest::newRow("Button 13") << ba << 23 << 0;
	ba[10] = 251; /* 1111 1011 */
	QTest::newRow("Button 13") << ba << 23 << 255;

	QTest::newRow("Button 14") << ba << 24 << 0;
	ba[10] = 253; /* 1111 1101 */
	QTest::newRow("Button 14") << ba << 24 << 255;

	QTest::newRow("Button 15") << ba << 25 << 0;
	ba[10] = 254; /* 1111 1110 */
	QTest::newRow("Button 15") << ba << 25 << 255;

	QTest::newRow("Button 16") << ba << 26 << 0;
	ba[9] = 127; /* 0111 1111 */
	QTest::newRow("Button 16") << ba << 26 << 255;

	QTest::newRow("Button 17") << ba << 27 << 0;
	ba[9] = 191; /* 1011 1111 */
	QTest::newRow("Button 17") << ba << 27 << 255;

	QTest::newRow("Button 18") << ba << 28 << 0;
	ba[9] = 223; /* 1101 1111 */
	QTest::newRow("Button 18") << ba << 28 << 255;

	QTest::newRow("Button 19") << ba << 29 << 0;
	ba[9] = 239; /* 1110 1111 */
	QTest::newRow("Button 19") << ba << 29 << 255;

	QTest::newRow("Button 20") << ba << 30 << 0;
	ba[9] = 247; /* 1111 0111 */
	QTest::newRow("Button 20") << ba << 30 << 255;

	QTest::newRow("Button 21") << ba << 31 << 0;
	ba[9] = 251; /* 1111 1011 */
	QTest::newRow("Button 21") << ba << 31 << 255;

	QTest::newRow("Button 22") << ba << 32 << 0;
	ba[8] = 191; /* 1011 1111 */
	QTest::newRow("Button 22") << ba << 32 << 255;

	QTest::newRow("Button 23") << ba << 33 << 0;
	ba[8] = 223; /* 1101 1111 */
	QTest::newRow("Button 23") << ba << 33 << 255;

	QTest::newRow("Button 24") << ba << 34 << 0;
	ba[8] = 239; /* 1110 1111 */
	QTest::newRow("Button 24") << ba << 34 << 255;

	QTest::newRow("Button 25") << ba << 35 << 0;
	ba[8] = 247; /* 1111 0111 */
	QTest::newRow("Button 25") << ba << 35 << 255;

	QTest::newRow("Button 26") << ba << 36 << 0;
	ba[8] = 251; /* 1111 1011 */
	QTest::newRow("Button 26") << ba << 36 << 255;

	QTest::newRow("Button 27") << ba << 37 << 0;
	ba[9] = 253; /* 1111 1101 */
	QTest::newRow("Button 27") << ba << 37 << 255;

	QTest::newRow("Button 28") << ba << 38 << 0;
	ba[9] = 254; /* 1111 1110 */
	QTest::newRow("Button 28") << ba << 38 << 255;

	QTest::newRow("Button 29") << ba << 39 << 0;
	ba[8] = 127; /* 0111 1111 */
	QTest::newRow("Button 29") << ba << 39 << 255;

	QTest::newRow("Button 30") << ba << 40 << 0;
	ba[8] = 253; /* 1111 1101 */
	QTest::newRow("Button 30") << ba << 40 << 255;

	QTest::newRow("Button 31") << ba << 41 << 0;
	ba[8] = 254; /* 1111 1110 */
	QTest::newRow("Button 31") << ba << 41 << 255;
}

void TestEPlaybackWing::buttons()
{
	QFETCH(QByteArray, ba);
	QFETCH(int, channel);
	QFETCH(int, value);

	m_ewing->parseData(ba);
	QVERIFY(m_ewing->cacheValue(channel) == (unsigned char) value);
}

void TestEPlaybackWing::faders_data()
{
	QByteArray ba(data());

	QTest::addColumn<QByteArray> ("ba");
	QTest::addColumn<int> ("channel");
	QTest::addColumn<int> ("value");

	ba[15] = 127;
	QTest::newRow("Fader 0") << ba << 0 << 127;
	ba[16] = 191;
	QTest::newRow("Fader 1") << ba << 1 << 191;
	ba[17] = 223;
	QTest::newRow("Fader 2") << ba << 2 << 223;
	ba[18] = 239;
	QTest::newRow("Fader 3") << ba << 3 << 239;
	ba[19] = 247;
	QTest::newRow("Fader 4") << ba << 4 << 247;
	ba[20] = 251;
	QTest::newRow("Fader 5") << ba << 5 << 251;
	ba[21] = 253;
	QTest::newRow("Fader 6") << ba << 6 << 253;
	ba[22] = 254;
	QTest::newRow("Fader 7") << ba << 7 << 254;
	ba[23] = 1;
	QTest::newRow("Fader 8") << ba << 8 << 1;
	ba[24] = 13;
	QTest::newRow("Fader 9") << ba << 9 << 13;
}

void TestEPlaybackWing::faders()
{
	QFETCH(QByteArray, ba);
	QFETCH(int, channel);
	QFETCH(int, value);

	m_ewing->parseData(ba);
	QVERIFY(m_ewing->cacheValue(channel) == (unsigned char) value);
}

void TestEPlaybackWing::cleanupTestCase()
{
	delete m_ewing;
	m_ewing = NULL;
}
