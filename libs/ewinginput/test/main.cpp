#include <QtTest>

#include "testeplaybackwing.h"
#include "testeshortcutwing.h"

int main(int argc, char** argv)
{
	TestEPlaybackWing test1;
	QTest::qExec(&test1);

	TestEShortcutWing test2;
	QTest::qExec(&test2);
}
