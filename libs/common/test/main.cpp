#include <QtTest>

#include "qlcphysical_test.h"

int main(int argc, char** argv)
{
	QCoreApplication(argc, argv);

	QLCPhysical_Test physical;
	QTest::qExec(&physical, argc, argv);
}
