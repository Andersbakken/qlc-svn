/*
  Q Light Controller - Unit test
  inputmap_test.cpp

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

#include "inputplugin_stub.h"
#include "inputmap_test.h"

#define protected public
#include "../inputpatch.h"
#include "../inputmap.h"
#undef protected

void InputMap_Test::initial()
{
	InputMap im(this);
	QVERIFY(im.universes() == 4);
	QVERIFY(im.m_universes == 4);
	QVERIFY(im.editorUniverse() == 0);
	QVERIFY(im.m_editorUniverse == 0);
	QVERIFY(im.m_patch.size() == 4);
	QVERIFY(im.m_plugins.size() == 0);
	QVERIFY(im.pluginNames().size() == 0);
	QVERIFY(im.m_profiles.size() == 0);
	QVERIFY(im.profileNames().size() == 0);
}

void InputMap_Test::appendPlugin()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();
	QVERIFY(im.appendPlugin(stub) == true);
	QVERIFY(im.m_plugins.size() == 1);
	QVERIFY(im.plugin(stub->name()) == stub);
}

void InputMap_Test::setPatch()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();
	im.appendPlugin(stub);

	QVERIFY(im.patch(0)->plugin() == NULL);
	QVERIFY(im.patch(0)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(2)->plugin() == NULL);
	QVERIFY(im.patch(2)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);

	QVERIFY(im.setPatch(0, "Foobar", 0) == true);
	QVERIFY(im.patch(0)->plugin() == NULL);
	QVERIFY(im.patch(0)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(2)->plugin() == NULL);
	QVERIFY(im.patch(2)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);
}

void InputMap_Test::pluginNames()
{
}

void InputMap_Test::pluginInputs()
{
}

