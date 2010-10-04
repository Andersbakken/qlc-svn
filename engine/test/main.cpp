/*
  Q Light Controller - Unit tests
  main.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#include <QCoreApplication>
#include <QtTest>

#include "qlcfixturedefcache_test.h"
#include "qlcinputchannel_test.h"
#include "qlcinputprofile_test.h"
#include "qlcfixturemode_test.h"
#include "qlcfixturedef_test.h"
#include "qlccapability_test.h"
#include "qlcphysical_test.h"
#include "qlcchannel_test.h"
#include "qlcmacros_test.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    int r;

    QLCMacros_Test macros;
    r = QTest::qExec(&macros, argc, argv);
    if (r != 0)
        return r;

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

    return 0;
}
