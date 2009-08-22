#include <QApplication>
#include <QtTest>

#include "qlcwidgetproperties_test.h"
#include "qlcfixturedefcache_test.h"
#include "qlcinputchannel_test.h"
#include "qlcinputprofile_test.h"
#include "qlcfixturemode_test.h"
#include "qlcfixturedef_test.h"
#include "qlccapability_test.h"
#include "qlcphysical_test.h"
#include "qlcchannel_test.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	int r;

	QLCPhysical_Test physical;
	r = QTest::qExec(&physical, argc, argv);
	if (r != 0)
		return r;

	QLCCapability_Test capability;
	r = QTest::qExec(&capability, argc, argv);
	if (r != 0)
		return r;

	QLCChannel_Test channel;
	r = QTest::qExec(&channel, argc, argv);
	if (r != 0)
		return r;

	QLCFixtureMode_Test mode;
	r = QTest::qExec(&mode, argc, argv);
	if (r != 0)
		return r;

	QLCFixtureDef_Test fixtureDef;
	r = QTest::qExec(&fixtureDef, argc, argv);
	if (r != 0)
		return r;

	QLCFixtureDefCache_Test fixtureDefCache;
	r = QTest::qExec(&fixtureDefCache, argc, argv);
	if (r != 0)
		return r;

	QLCInputChannel_Test inputChannel;
	r = QTest::qExec(&inputChannel, argc, argv);
	if (r != 0)
		return r;

	QLCInputProfile_Test inputProfile;
	r = QTest::qExec(&inputProfile, argc, argv);
	if (r != 0)
		return r;

	QLCWidgetProperties_Test properties;
	r = QTest::qExec(&properties, argc, argv);
	if (r != 0)
		return r;

	return 0;
}
