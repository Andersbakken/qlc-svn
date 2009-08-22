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
#include "mastertimer_test.h"
#include "outputpatch_test.h"
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
	QApplication qapp(argc, argv);
	int r;
	
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

	SceneValue_Test scenevalue;
	r = QTest::qExec(&scenevalue, argc, argv);
	if (r != 0)
		return r;

	SceneChannel_Test scenechannel;
	r = QTest::qExec(&scenechannel, argc, argv);
	if (r != 0)
		return r;

	Scene_Test scene;
	r = QTest::qExec(&scene, argc, argv);
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

	return 0;
}

