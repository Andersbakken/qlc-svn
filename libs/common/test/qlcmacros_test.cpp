#include <QtTest/QtTest>

#include "qlcmacros_test.h"
#include "qlctypes.h"

void QLCMacros_Test::min()
{
	QVERIFY(MIN(0, 5) == 0);
	QVERIFY(MIN(5, 0) == 0);
	QVERIFY(MIN(0.4, -4.3) == -4.3);
	QVERIFY(MIN('a', 'h') == 'a');
}

void QLCMacros_Test::max()
{
	QVERIFY(MAX(7, 4) == 7);
	QVERIFY(MAX(3, 7) == 7);
	QVERIFY(MAX(12.5, -500.2) == 12.5);
	QVERIFY(MAX('f', 'o') == 'o');
}

void QLCMacros_Test::clamp()
{
	QVERIFY(CLAMP(500, 0, 10) == 10);
	QVERIFY(CLAMP(11, 0, 10) == 10);
	QVERIFY(CLAMP(9, 0, 10) == 9);
	QVERIFY(CLAMP(10, 0, 10) == 10);
	QVERIFY(CLAMP(0, 0, 10) == 0);
	QVERIFY(CLAMP(10, 20, 30) == 20);
}

void QLCMacros_Test::scale()
{
	QVERIFY(SCALE(5, 0, 10, 0, 20) == 10);
	QVERIFY(SCALE(0, 0, 10, 0, 20000) == 0);
	QVERIFY(SCALE(64, 0, 127, 0, 255) == 128);
}
