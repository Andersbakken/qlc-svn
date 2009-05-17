#include <QtTest>

#include "testeplaybackwing.h"
#include "testeshortcutwing.h"
#include "testeprogramwing.h"

int main(int argc, char** argv)
{
	TestEPlaybackWing test1;
	QTest::qExec(&test1);

	TestEShortcutWing test2;
	QTest::qExec(&test2);

	TestEProgramWing test3;
	QTest::qExec(&test3);
}
