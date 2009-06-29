/*
  Q Light Controller - Unit test
  scene_test.cpp

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

#include <QtTest>
#include <QtXml>

#include "scene_test.h"
#include "../scene.h"

void Scene_Test::initial()
{
	Scene s(this);
	QVERIFY(s.type() == Function::Scene);
	QVERIFY(s.name().isEmpty() == true);
	QVERIFY(s.values().isEmpty() == true);
}

void Scene_Test::copy()
{
}
