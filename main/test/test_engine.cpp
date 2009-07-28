/*
  Q Light Controller - Unit test
  test_engine.cpp

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

#include <QApplication>
#include <QtTest>

#include "scenechannel_test.h"
#include "outputpatch_test.h"
#include "inputpatch_test.h"
#include "scenevalue_test.h"
#include "collection_test.h"
#include "efxfixture_test.h"
#include "outputmap_test.h"
#include "fixture_test.h"
#include "chaser_test.h"
#include "scene_test.h"
#include "bus_test.h"
#include "efx_test.h"

/* This file includes tests for QLC's ENGINE components. UI tests are done
   separately. */
int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);

	Bus_Test bus;
	QTest::qExec(&bus, argc, argv);

	Fixture_Test fixture;
	QTest::qExec(&fixture, argc, argv);

	SceneValue_Test scenevalue;
	QTest::qExec(&scenevalue, argc, argv);

	SceneChannel_Test scenechannel;
	QTest::qExec(&scenechannel, argc, argv);

	Scene_Test scene;
	QTest::qExec(&scene, argc, argv);

	Chaser_Test chaser;
	QTest::qExec(&chaser, argc, argv);

	Collection_Test collection;
	QTest::qExec(&collection, argc, argv);

	EFX_Test efx;
	QTest::qExec(&efx, argc, argv);

	EFXFixture_Test efxfixture;
	QTest::qExec(&efxfixture, argc, argv);

	OutputPatch_Test outputpatch;
	QTest::qExec(&outputpatch, argc, argv);

	OutputMap_Test outputmap;
	QTest::qExec(&outputmap, argc, argv);

	InputPatch_Test inputpatch;
	QTest::qExec(&inputpatch, argc, argv);

	return 0;
}
