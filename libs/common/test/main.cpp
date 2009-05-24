#include <QtTest>

#include "qlcfixturemode_test.h"
#include "qlccapability_test.h"
#include "qlcphysical_test.h"
#include "qlcchannel_test.h"

int main(int argc, char** argv)
{
	QCoreApplication(argc, argv);

	QLCPhysical_Test physical;
	QTest::qExec(&physical, argc, argv);

	QLCCapability_Test capability;
	QTest::qExec(&capability, argc, argv);

	QLCChannel_Test channel;
	QTest::qExec(&channel, argc, argv);

	QLCFixtureMode_Test mode;
	QTest::qExec(&mode, argc, argv);
}
