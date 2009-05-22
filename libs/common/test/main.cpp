#include <QtTest>

#include "qlcphysical_test.h"
#include "qlcfixturemode_test.h"

int main(int argc, char** argv)
{
	QCoreApplication(argc, argv);

	QLCPhysical_Test physical;
	QTest::qExec(&physical, argc, argv);

	QLCFixtureMode_Test mode;
	QTest::qExec(&mode, argc, argv);
}
