/*
  Q Light Controller - Unit test
  main.cpp

  Copyright (c) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QCoreApplication>
#include <QtTest>

// Fixture metadata
#include "qlcfixturedefcache_test.h"
#include "qlcinputchannel_test.h"
#include "qlcinputprofile_test.h"
#include "qlcfixturemode_test.h"
#include "qlcfixturedef_test.h"
#include "qlccapability_test.h"
#include "qlcphysical_test.h"
#include "qlcchannel_test.h"
#include "qlcmacros_test.h"
#include "qlcfile_test.h"
#include "qlci18n_test.h"

// Engine
#include "palettegenerator_test.h"
#include "universearray_test.h"
#include "chaserrunner_test.h"
#include "mastertimer_test.h"
#include "outputpatch_test.h"
#include "fadechannel_test.h"
#include "inputpatch_test.h"
#include "scenevalue_test.h"
#include "collection_test.h"
#include "efxfixture_test.h"
#include "outputmap_test.h"
#include "inputmap_test.h"
#include "function_test.h"
#include "fixture_test.h"
#include "chaser_test.h"
#include "scene_test.h"
#include "bus_test.h"
#include "efx_test.h"
#include "doc_test.h"

/* This file includes tests for QLC's ENGINE components. UI tests are done
   separately. */
int main(int argc, char** argv)
{
    QCoreApplication qapp(argc, argv);
    int r;

    /********************************************************************
     * Fixture metadata
     ********************************************************************/

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

    QLCFile_Test file;
    r = QTest::qExec(&file, argc, argv);
    if (r != 0)
        return r;

    QLCi18n_Test i18n;
    r = QTest::qExec(&i18n, argc, argv);
    if (r != 0)
        return r;

    /********************************************************************
     * Engine
     ********************************************************************/

    UniverseArray_Test universearray;
    r = QTest::qExec(&universearray, argc, argv);
    if (r != 0)
        return r;

    OutputPatch_Test outputpatch;
    r = QTest::qExec(&outputpatch, argc, argv);
    if (r != 0)
        return r;

    OutputMap_Test outputmap;
    r = QTest::qExec(&outputmap, argc, argv);
    if (r != 0)
        return r;

    InputPatch_Test inputpatch;
    r = QTest::qExec(&inputpatch, argc, argv);
    if (r != 0)
        return r;

    InputMap_Test inputmap;
    r = QTest::qExec(&inputmap, argc, argv);
    if (r != 0)
        return r;

    Bus_Test bus;
    r = QTest::qExec(&bus, argc, argv);
    if (r != 0)
        return r;

    Fixture_Test fixture;
    r = QTest::qExec(&fixture, argc, argv);
    if (r != 0)
        return r;

    Function_Test function;
    r = QTest::qExec(&function, argc, argv);
    if (r != 0)
        return r;

    FadeChannel_Test fadechannel;
    r = QTest::qExec(&fadechannel, argc, argv);
    if (r != 0)
        return r;

    SceneValue_Test scenevalue;
    r = QTest::qExec(&scenevalue, argc, argv);
    if (r != 0)
        return r;

    Scene_Test scene;
    r = QTest::qExec(&scene, argc, argv);
    if (r != 0)
        return r;

    ChaserRunner_Test chaserrunner;
    r = QTest::qExec(&chaserrunner, argc, argv);
    if (r != 0)
        return r;

    Chaser_Test chaser;
    r = QTest::qExec(&chaser, argc, argv);
    if (r != 0)
        return r;

    Collection_Test collection;
    r = QTest::qExec(&collection, argc, argv);
    if (r != 0)
        return r;

    EFXFixture_Test efxfixture;
    r = QTest::qExec(&efxfixture, argc, argv);
    if (r != 0)
        return r;

    EFX_Test efx;
    r = QTest::qExec(&efx, argc, argv);
    if (r != 0)
        return r;

    MasterTimer_Test mt;
    r = QTest::qExec(&mt, argc, argv);
    if (r != 0)
        return r;

    Doc_Test doc;
    r = QTest::qExec(&doc, argc, argv);
    if (r != 0)
        return r;

    PaletteGenerator_Test palette;
    r = QTest::qExec(&palette, argc, argv);
    if (r != 0)
        return r;

    return 0;
}

