#include <QtTest>

#include "qlcinputchannel_test.h"
#include "qlcinputprofile_test.h"
#include "qlcfixturemode_test.h"
#include "qlcfixturedef_test.h"
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

	QLCFixtureDef_Test fixtureDef;
	QTest::qExec(&fixtureDef, argc, argv);

	QLCInputChannel_Test inputChannel;
	QTest::qExec(&inputChannel, argc, argv);

	QLCInputProfile_Test inputProfile;
	QTest::qExec(&inputProfile, argc, argv);
}
