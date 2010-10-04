#include <QApplication>
#include <QUdpSocket>
#include <QByteArray>
#include <QTest>

#include "eprogramwing.h"
#include "testeprogramwing.h"

#define PRG_FIRMWARE 178
#define PRG_FLAGS (1 << 7) /* Page Up */	\
		| (1 << 6) /* Page Down */	\
		| (1 << 5) /* Back */		\
		| (1 << 4) /* Go */		\
		| (1 << 1) /* Product (1=PLB, 2=SHC, 3=PGM) */	\
		| (1 << 0) /* Product */

QByteArray TestEProgramWing::data()
{
    QByteArray data(28, 0);
    data[0] = 'W'; /* HEADER */
    data[1] = 'O'; /* HEADER */
    data[2] = 'D'; /* HEADER */
    data[3] = 'D'; /* HEADER */

    data[4] = PRG_FIRMWARE; /* Firmware */

    data[5] = PRG_FLAGS; /* Flags */

    data[6] = 255; /* Keys 33, 59, 60, 61 and push encoders 0, 1, 2 */
    data[7] = 255; /* Keys 51, 52, 53, 41, 42, 43, 31, 32 */
    data[8] = 255; /* Keys 58, 54, 55, 44, 45, 34, 35, 57 */
    data[9] = 255; /* Keys 39, 40, 46, 47, 48, 49, 50, 56 */
    data[10] = 255; /* Keys 26, 27, 28, 29, 30, 36, 37, 38 */
    data[11] = 255; /* Keys 18, 19, 20, 21, 22, 23, 24, 25 */
    data[12] = 255; /* Keys 15, 16, 17 */
    data[13] = 255; /* Keys 7, 8, 9, 10, 11, 12, 13, 14 */
    data[14] = 255; /* Keys 0, 1, 2, 3, 4, 5, 6 */

    data[15] = 0; /* Unused */
    data[16] = 0; /* Unused */
    data[17] = 0; /* Unused */
    data[18] = 0; /* Unused */
    data[19] = 0; /* Unused */
    data[20] = 0; /* Unused */
    data[21] = 0; /* Unused */
    data[22] = 0; /* Unused */
    data[23] = 0; /* Unused */
    data[24] = 0; /* Unused */

    data[25] = 0; /* Encoder 0 direction */
    data[26] = 0; /* Encoder 1 direction */
    data[27] = 0; /* Encoder 2 direction */

    return data;
}

void TestEProgramWing::initTestCase()
{
    m_ewing = new EProgramWing(this, QHostAddress::LocalHost, data());
    QVERIFY(m_ewing != NULL);
    QVERIFY(m_ewing->type() == EWing::Program);
}

void TestEProgramWing::firmware()
{
    QVERIFY(m_ewing->firmware() == PRG_FIRMWARE);
}

void TestEProgramWing::address()
{
    QVERIFY(m_ewing->address() == QHostAddress::LocalHost);
}

void TestEProgramWing::isOutputData()
{
    QByteArray ba(data());

    QVERIFY(EWing::isOutputData(ba) == true);

    ba[1] = 'I';
    QVERIFY(EWing::isOutputData(ba) == false);
}

void TestEProgramWing::name()
{
    QCOMPARE(m_ewing->name(), QString("Program ") + tr("at") + QString(" ")
             + QHostAddress(QHostAddress::LocalHost).toString());
}

void TestEProgramWing::infoText()
{
    QString str = QString("<B>%1</B><BR>").arg(m_ewing->name());
    str += tr("Firmware version %1").arg(PRG_FIRMWARE);
    str += tr("<P>Device is operating correctly</P>");
    QCOMPARE(m_ewing->infoText(), str);
}

void TestEProgramWing::buttons_data()
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

    /********************************************************************/

    /*             ROW NAME     DATA   CH   VAL */
    QTest::newRow("Button 0 OFF") << ba << 0 << 0;
    ba[14] = 127; /* 0111 1111 */
    QTest::newRow("Button 0") << ba << 0 << 255;

    QTest::newRow("Button 1 OFF") << ba << 1 << 0;
    ba[14] = 191; /* 1011 1111 */
    QTest::newRow("Button 1") << ba << 1 << 255;

    QTest::newRow("Button 2 OFF") << ba << 2 << 0;
    ba[14] = 223; /* 1101 1111 */
    QTest::newRow("Button 2") << ba << 2 << 255;

    QTest::newRow("Button 3 OFF") << ba << 3 << 0;
    ba[14] = 239; /* 1110 1111 */
    QTest::newRow("Button 3") << ba << 3 << 255;

    QTest::newRow("Button 4 OFF") << ba << 4 << 0;
    ba[14] = 247; /* 1111 0111 */
    QTest::newRow("Button 4") << ba << 4 << 255;

    QTest::newRow("Button 5 OFF") << ba << 5 << 0;
    ba[14] = 253; /* 1111 1101 */
    QTest::newRow("Button 5") << ba << 5 << 255;

    QTest::newRow("Button 6 OFF") << ba << 6 << 0;
    ba[14] = 254; /* 1111 1110 */
    QTest::newRow("Button 6") << ba << 6 << 255;

    /********************************************************************/

    QTest::newRow("Button 7 OFF") << ba << 7 << 0;
    ba[13] = 127; /* 0111 1111 */
    QTest::newRow("Button 7") << ba << 7 << 255;

    QTest::newRow("Button 8 OFF") << ba << 8 << 0;
    ba[13] = 191; /* 1011 1111 */
    QTest::newRow("Button 8") << ba << 8 << 255;

    QTest::newRow("Button 9 OFF") << ba << 9 << 0;
    ba[13] = 223; /* 1101 1111 */
    QTest::newRow("Button 9") << ba << 9 << 255;

    QTest::newRow("Button 10 OFF") << ba << 10 << 0;
    ba[13] = 239; /* 1110 1111 */
    QTest::newRow("Button 10") << ba << 10 << 255;

    QTest::newRow("Button 11 OFF") << ba << 11 << 0;
    ba[13] = 247; /* 1111 0111 */
    QTest::newRow("Button 11") << ba << 11 << 255;

    QTest::newRow("Button 12 OFF") << ba << 12 << 0;
    ba[13] = 251; /* 1111 1011 */
    QTest::newRow("Button 12") << ba << 12 << 255;

    QTest::newRow("Button 13 OFF") << ba << 13 << 0;
    ba[13] = 253; /* 1111 1101 */
    QTest::newRow("Button 13") << ba << 13 << 255;

    QTest::newRow("Button 14 OFF") << ba << 14 << 0;
    ba[13] = 254; /* 1111 1110 */
    QTest::newRow("Button 14") << ba << 14 << 255;

    /********************************************************************/

    QTest::newRow("Button 15 OFF") << ba << 15 << 0;
    ba[12] = 127; /* 0111 1111 */
    QTest::newRow("Button 15") << ba << 15 << 255;

    QTest::newRow("Button 16 OFF") << ba << 16 << 0;
    ba[12] = 253; /* 1111 1101 */
    QTest::newRow("Button 16") << ba << 16 << 255;

    QTest::newRow("Button 17 OFF") << ba << 17 << 0;
    ba[12] = 254; /* 1111 1110 */
    QTest::newRow("Button 17") << ba << 17 << 255;

    /********************************************************************/

    QTest::newRow("Button 18 OFF") << ba << 18 << 0;
    ba[11] = 127; /* 0111 1111 */
    QTest::newRow("Button 18") << ba << 18 << 255;

    QTest::newRow("Button 19 OFF") << ba << 19 << 0;
    ba[11] = 191; /* 1011 1111 */
    QTest::newRow("Button 19") << ba << 19 << 255;

    QTest::newRow("Button 20 OFF") << ba << 20 << 0;
    ba[11] = 223; /* 1101 1111 */
    QTest::newRow("Button 20") << ba << 20 << 255;

    QTest::newRow("Button 21 OFF") << ba << 21 << 0;
    ba[11] = 239; /* 1110 1111 */
    QTest::newRow("Button 21") << ba << 21 << 255;

    QTest::newRow("Button 22 OFF") << ba << 22 << 0;
    ba[11] = 247; /* 1111 0111 */
    QTest::newRow("Button 22") << ba << 22 << 255;

    QTest::newRow("Button 23 OFF") << ba << 23 << 0;
    ba[11] = 251; /* 1111 1011 */
    QTest::newRow("Button 23") << ba << 23 << 255;

    QTest::newRow("Button 24 OFF") << ba << 24 << 0;
    ba[11] = 253; /* 1110 1101 */
    QTest::newRow("Button 24") << ba << 24 << 255;

    QTest::newRow("Button 25 OFF") << ba << 25 << 0;
    ba[11] = 254; /* 1111 1110 */
    QTest::newRow("Button 25") << ba << 25 << 255;

    /********************************************************************/

    QTest::newRow("Button 26 OFF") << ba << 26 << 0;
    ba[10] = 127; /* 0111 1111 */
    QTest::newRow("Button 26") << ba << 26 << 255;

    QTest::newRow("Button 27 OFF") << ba << 27 << 0;
    ba[10] = 191; /* 1011 1111 */
    QTest::newRow("Button 27") << ba << 27 << 255;

    QTest::newRow("Button 28 OFF") << ba << 28 << 0;
    ba[10] = 223; /* 1101 1111 */
    QTest::newRow("Button 28") << ba << 28 << 255;

    QTest::newRow("Button 29 OFF") << ba << 29 << 0;
    ba[10] = 239; /* 1110 1111 */
    QTest::newRow("Button 29") << ba << 29 << 255;

    QTest::newRow("Button 30 OFF") << ba << 30 << 0;
    ba[10] = 247; /* 1111 0111 */
    QTest::newRow("Button 30") << ba << 30 << 255;

    QTest::newRow("Button 31 OFF") << ba << 31 << 0;
    ba[7] = 253; /* 1111 1101 */
    QTest::newRow("Button 31") << ba << 31 << 255;

    QTest::newRow("Button 32 OFF") << ba << 32 << 0;
    ba[7] = 254; /* 1111 1110 */
    QTest::newRow("Button 32") << ba << 32 << 255;

    QTest::newRow("Button 33 OFF") << ba << 33 << 0;
    ba[6] = 127; /* 0111 1111 */
    QTest::newRow("Button 33") << ba << 33 << 255;

    /********************************************************************/

    QTest::newRow("Button 34 OFF") << ba << 34 << 0;
    ba[8] = 251; /* 1111 1011 */
    QTest::newRow("Button 34") << ba << 34 << 255;

    QTest::newRow("Button 35 OFF") << ba << 35 << 0;
    ba[8] = 253; /* 1111 1101 */
    QTest::newRow("Button 35") << ba << 35 << 255;

    QTest::newRow("Button 36 OFF") << ba << 36 << 0;
    ba[10] = 251; /* 1111 1011 */
    QTest::newRow("Button 36") << ba << 36 << 255;

    QTest::newRow("Button 37 OFF") << ba << 37 << 0;
    ba[10] = 253; /* 1111 1101 */
    QTest::newRow("Button 37") << ba << 37 << 255;

    QTest::newRow("Button 38 OFF") << ba << 38 << 0;
    ba[10] = 254; /* 1111 1110 */
    QTest::newRow("Button 38") << ba << 38 << 255;

    QTest::newRow("Button 39 OFF") << ba << 39 << 0;
    ba[9] = 127; /* 0111 1111 */
    QTest::newRow("Button 39") << ba << 39 << 255;

    QTest::newRow("Button 40 OFF") << ba << 40 << 0;
    ba[9] = 191; /* 1011 1111 */
    QTest::newRow("Button 40") << ba << 40 << 255;

    QTest::newRow("Button 41 OFF") << ba << 41 << 0;
    ba[7] = 239; /* 1110 1111 */
    QTest::newRow("Button 41") << ba << 41 << 255;

    /********************************************************************/

    QTest::newRow("Button 42 OFF") << ba << 42 << 0;
    ba[7] = 247; /* 1111 0111 */
    QTest::newRow("Button 42") << ba << 42 << 255;

    QTest::newRow("Button 43 OFF") << ba << 43 << 0;
    ba[7] = 251; /* 1111 1011 */
    QTest::newRow("Button 43") << ba << 43 << 255;

    QTest::newRow("Button 44 OFF") << ba << 44 << 0;
    ba[8] = 239; /* 1110 1111 */
    QTest::newRow("Button 44") << ba << 44 << 255;

    QTest::newRow("Button 45 OFF") << ba << 45 << 0;
    ba[8] = 247; /* 1111 0111 */
    QTest::newRow("Button 45") << ba << 45 << 255;

    QTest::newRow("Button 46 OFF") << ba << 46 << 0;
    ba[9] = 223; /* 1101 1111 */
    QTest::newRow("Button 46") << ba << 46 << 255;

    QTest::newRow("Button 47 OFF") << ba << 47 << 0;
    ba[9] = 239; /* 1110 1111 */
    QTest::newRow("Button 47") << ba << 47 << 255;

    QTest::newRow("Button 48 OFF") << ba << 48 << 0;
    ba[9] = 247; /* 1111 0111 */
    QTest::newRow("Button 48") << ba << 48 << 255;

    QTest::newRow("Button 49 OFF") << ba << 49 << 0;
    ba[9] = 251; /* 1111 1011 */
    QTest::newRow("Button 49") << ba << 49 << 255;

    /********************************************************************/

    QTest::newRow("Button 50 OFF") << ba << 50 << 0;
    ba[9] = 253; /* 1111 1101 */
    QTest::newRow("Button 50") << ba << 50 << 255;

    QTest::newRow("Button 51 OFF") << ba << 51 << 0;
    ba[7] = 127; /* 0111 1111 */
    QTest::newRow("Button 51") << ba << 51 << 255;

    QTest::newRow("Button 52 OFF") << ba << 52 << 0;
    ba[7] = 191; /* 1011 1111 */
    QTest::newRow("Button 52") << ba << 52 << 255;

    QTest::newRow("Button 53 OFF") << ba << 53 << 0;
    ba[7] = 223; /* 1101 1111 */
    QTest::newRow("Button 53") << ba << 53 << 255;

    QTest::newRow("Button 54 OFF") << ba << 54 << 0;
    ba[8] = 191; /* 1011 1111 */
    QTest::newRow("Button 54") << ba << 54 << 255;

    QTest::newRow("Button 55 OFF") << ba << 55 << 0;
    ba[8] = 223; /* 1101 1111 */
    QTest::newRow("Button 55") << ba << 55 << 255;

    QTest::newRow("Button 56 OFF") << ba << 56 << 0;
    ba[9] = 254; /* 1111 1110 */
    QTest::newRow("Button 56") << ba << 56 << 255;

    QTest::newRow("Button 57 OFF") << ba << 57 << 0;
    ba[8] = 254; /* 1111 1110 */
    QTest::newRow("Button 57") << ba << 57 << 255;

    /********************************************************************/

    QTest::newRow("Button 58 OFF") << ba << 58 << 0;
    ba[8] = 127; /* 0111 1111 */
    QTest::newRow("Button 58 OFF") << ba << 58 << 255;

    QTest::newRow("Button 59 OFF") << ba << 59 << 0;
    ba[6] = 191; /* 1011 1111 */
    QTest::newRow("Button 59") << ba << 59 << 255;

    QTest::newRow("Button 60 OFF") << ba << 60 << 0;
    ba[6] = 223; /* 1101 1111 */
    QTest::newRow("Button 60") << ba << 60 << 255;

    QTest::newRow("Button 61 OFF") << ba << 61 << 0;
    ba[6] = 239; /* 1110 1111 */
    QTest::newRow("Button 61") << ba << 61 << 255;

    QTest::newRow("Button 62: Encoder 0 OFF") << ba << 62 << 0;
    ba[6] = 247; /* 1111 0111 */
    QTest::newRow("Button 62: Encoder 0") << ba << 62 << 255;

    QTest::newRow("Button 63: Encoder 1 OFF") << ba << 63 << 0;
    ba[6] = 251; /* 1111 1011 */
    QTest::newRow("Button 63: Encoder 1") << ba << 63 << 255;

    QTest::newRow("Button 64: Encoder 2 OFF") << ba << 64 << 0;
    ba[6] = 253; /* 1111 1101 */
    QTest::newRow("Button 64: Encoder 2") << ba << 64 << 255;
}

void TestEProgramWing::buttons()
{
    QFETCH(QByteArray, ba);
    QFETCH(int, channel);
    QFETCH(int, value);

    m_ewing->parseData(ba);
    QCOMPARE(m_ewing->cacheValue(channel), (unsigned char) value);
}

void TestEProgramWing::encoders_data()
{
    QByteArray ba(data());

    QTest::addColumn<QByteArray> ("ba"); /* Data to feed to parseData() */
    QTest::addColumn<int> ("channel"); /* Input channel whose value should change */
    QTest::addColumn<int> ("value"); /* The new value expected for the channel */

    /* Encoders are relative, so just feeding direction information
       (0 = No change, 255 = Clockwise, 1 = Counter-clockwise) keeps
       increasing/decreasing the value. */

    QTest::newRow("Encoder 0 Initial") << ba << 65 << 0;
    ba[25] = 255;
    for (int i = 1; i <= 255; i++)
        QTest::newRow("Encoder 0 CW") << ba << 65 << i;

    ba[25] = 1;
    for (int i = 254; i >= 0; i--)
        QTest::newRow("Encoder 0 CCW") << ba << 65 << i;

    /********************************************************************/

    QTest::newRow("Encoder 1 Initial") << ba << 66 << 0;
    ba[26] = 255;
    for (int i = 1; i <= 255; i++)
        QTest::newRow("Encoder 1 CW") << ba << 66 << i;

    ba[26] = 1;
    for (int i = 254; i >= 0; i--)
        QTest::newRow("Encoder 1 CCW") << ba << 66 << i;

    /********************************************************************/

    QTest::newRow("Encoder 2 CW Initial") << ba << 67 << 0;
    ba[27] = 255;
    for (int i = 1; i <= 255; i++)
        QTest::newRow("Encoder 2 CW") << ba << 67 << i;

    ba[27] = 1;
    for (int i = 254; i >= 0; i--)
        QTest::newRow("Encoder 2 CCW") << ba << 67 << i;
}

void TestEProgramWing::encoders()
{
    QFETCH(QByteArray, ba);
    QFETCH(int, channel);
    QFETCH(int, value);

    m_ewing->parseData(ba);
    QCOMPARE(int(m_ewing->cacheValue(channel)), value);
}

void TestEProgramWing::cleanupTestCase()
{
    delete m_ewing;
    m_ewing = NULL;
}
