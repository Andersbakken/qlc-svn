/*
  Q Light Controller - Unit test
  outputmap_test.cpp

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

#include "outputplugin_stub.h"
#include "outputmap_test.h"

#define protected public
#include "../outputpatch.h"
#include "../outputmap.h"
#undef protected

void OutputMap_Test::initial()
{
	OutputMap om(this);
	QVERIFY(om.m_universes == 4);
	QVERIFY(om.universes() == 4);
	QVERIFY(om.m_blackout == false);
	QVERIFY(om.blackout() == false);
	QVERIFY(om.m_universeArray->size() == 512 * 4);
	QVERIFY(om.m_universeChanged == false);
	QVERIFY(om.m_universeMutex.tryLock() == true);
	om.m_universeMutex.unlock();

	/* Dummy Out should be there... */
	QVERIFY(om.m_plugins.size() == 1);

	/* ...assigned by default to each universe */
	QVERIFY(om.m_patch.size() == 4);
	QVERIFY(om.patch(0) != NULL);
	QVERIFY(om.patch(0)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(0)->output() == 0);

	QVERIFY(om.patch(1) != NULL);
	QVERIFY(om.patch(1)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(1)->output() == 1);

	QVERIFY(om.patch(2) != NULL);
	QVERIFY(om.patch(2)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(2)->output() == 2);

	QVERIFY(om.patch(3) != NULL);
	QVERIFY(om.patch(3)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(3)->output() == 3);

	QVERIFY(om.patch(4) == NULL);
}

void OutputMap_Test::appendPlugin()
{
	OutputMap om(this);

	OutputPluginStub* stub = new OutputPluginStub();
	QVERIFY(om.appendPlugin(stub) == true);
	QVERIFY(om.appendPlugin(stub) == false);
	QVERIFY(om.plugin(stub->name()) == stub);
	QVERIFY(om.plugin(om.m_dummyOut->name()) == om.m_dummyOut);
}

void OutputMap_Test::setPatch()
{
	OutputMap om(this);

	OutputPluginStub* stub = new OutputPluginStub();
	om.appendPlugin(stub);

	QVERIFY(om.setPatch(0, "Foobar", 0) == false);
	QVERIFY(om.patch(0)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(0)->output() == 0);
	QVERIFY(om.patch(1)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(1)->output() == 1);
	QVERIFY(om.patch(2)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(2)->output() == 2);
	QVERIFY(om.patch(3)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(3)->output() == 3);

	QVERIFY(om.setPatch(KUniverseCount, stub->name(), 0) == false);
	QVERIFY(om.patch(0)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(0)->output() == 0);
	QVERIFY(om.patch(1)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(1)->output() == 1);
	QVERIFY(om.patch(2)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(2)->output() == 2);
	QVERIFY(om.patch(3)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(3)->output() == 3);

	QVERIFY(om.setPatch(4, stub->name(), 4) == false);
	QVERIFY(om.patch(0)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(0)->output() == 0);
	QVERIFY(om.patch(1)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(1)->output() == 1);
	QVERIFY(om.patch(2)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(2)->output() == 2);
	QVERIFY(om.patch(3)->plugin() == om.m_dummyOut);
	QVERIFY(om.patch(3)->output() == 3);

	QVERIFY(om.setPatch(3, stub->name(), 0) == true);
	QVERIFY(om.patch(3)->plugin() == stub);
	QVERIFY(om.patch(3)->output() == 0);

	QVERIFY(om.setPatch(2, stub->name(), 1) == true);
	QVERIFY(om.patch(2)->plugin() == stub);
	QVERIFY(om.patch(2)->output() == 1);

	QVERIFY(om.setPatch(1, stub->name(), 2) == true);
	QVERIFY(om.patch(1)->plugin() == stub);
	QVERIFY(om.patch(1)->output() == 2);

	QVERIFY(om.setPatch(0, stub->name(), 3) == true);
	QVERIFY(om.patch(0)->plugin() == stub);
	QVERIFY(om.patch(0)->output() == 3);
}

void OutputMap_Test::claimReleaseDump()
{
	OutputMap om(this);

	OutputPluginStub* stub = new OutputPluginStub();
	om.appendPlugin(stub);

	om.setPatch(0, stub->name(), 0);
	om.setPatch(1, stub->name(), 1);
	om.setPatch(2, stub->name(), 2);
	om.setPatch(3, stub->name(), 3);

	QByteArray* unis = om.claimUniverses();
	std::fill(unis->data() + 0, unis->data() + 512, 'a');
	std::fill(unis->data() + 512, unis->data() + 1024, 'b');
	std::fill(unis->data() + 1024, unis->data() + 1536, 'c');
	std::fill(unis->data() + 1536, unis->data() + 2048, 'd');
	om.releaseUniverses();

	om.dumpUniverses();

	for (int i = 0; i < 512; i++)
		QVERIFY(stub->m_array[i] == 'a');
	for (int i = 512; i < 1024; i++)
		QVERIFY(stub->m_array[i] == 'b');
	for (int i = 1024; i < 1536; i++)
		QVERIFY(stub->m_array[i] == 'c');
	for (int i = 1536; i < 2048; i++)
		QVERIFY(stub->m_array[i] == 'd');
}

void OutputMap_Test::blackout()
{
	OutputMap om(this);

	OutputPluginStub* stub = new OutputPluginStub();
	om.appendPlugin(stub);

	om.setPatch(0, stub->name(), 0);
	om.setPatch(1, stub->name(), 1);
	om.setPatch(2, stub->name(), 2);
	om.setPatch(3, stub->name(), 3);

	QByteArray* unis = om.claimUniverses();
	std::fill(unis->data() + 0, unis->data() + 512, 'a');
	std::fill(unis->data() + 512, unis->data() + 1024, 'b');
	std::fill(unis->data() + 1024, unis->data() + 1536, 'c');
	std::fill(unis->data() + 1536, unis->data() + 2048, 'd');
	om.releaseUniverses();
	om.dumpUniverses();

	om.setBlackout(true);
	QVERIFY(om.blackout() == true);
	om.dumpUniverses();

	for (int i = 0; i < 2048; i++)
		QVERIFY(stub->m_array[i] == (char) 0);

	om.setBlackout(true);
	QVERIFY(om.blackout() == true);
	om.dumpUniverses();

	for (int i = 0; i < 2048; i++)
		QVERIFY(stub->m_array[i] == (char) 0);

	om.toggleBlackout();
	QVERIFY(om.blackout() == false);
	om.dumpUniverses();

	for (int i = 0; i < 512; i++)
		QVERIFY(stub->m_array[i] == 'a');
	for (int i = 512; i < 1024; i++)
		QVERIFY(stub->m_array[i] == 'b');
	for (int i = 1024; i < 1536; i++)
		QVERIFY(stub->m_array[i] == 'c');
	for (int i = 1536; i < 2048; i++)
		QVERIFY(stub->m_array[i] == 'd');

	om.setBlackout(false);
	QVERIFY(om.blackout() == false);
	om.dumpUniverses();

	for (int i = 0; i < 512; i++)
		QVERIFY(stub->m_array[i] == 'a');
	for (int i = 512; i < 1024; i++)
		QVERIFY(stub->m_array[i] == 'b');
	for (int i = 1024; i < 1536; i++)
		QVERIFY(stub->m_array[i] == 'c');
	for (int i = 1536; i < 2048; i++)
		QVERIFY(stub->m_array[i] == 'd');

	om.toggleBlackout();
	QVERIFY(om.blackout() == true);
	om.dumpUniverses();

	for (int i = 0; i < 2048; i++)
		QVERIFY(stub->m_array[i] == (char) 0);
}

void OutputMap_Test::pluginNames()
{
	OutputMap om(this);

	QVERIFY(om.pluginNames().size() == 1);
	QVERIFY(om.pluginNames().at(0) == om.m_dummyOut->name());

	OutputPluginStub* stub = new OutputPluginStub();
	om.appendPlugin(stub);

	QVERIFY(om.pluginNames().size() == 2);
	QVERIFY(om.pluginNames().at(0) == om.m_dummyOut->name());
	QVERIFY(om.pluginNames().at(1) == stub->name());
}

void OutputMap_Test::pluginOutputs()
{
	OutputMap om(this);

	OutputPluginStub* stub = new OutputPluginStub();
	om.appendPlugin(stub);

	QStringList ls(om.pluginOutputs(stub->name()));
	QVERIFY(ls == stub->outputs());
	QVERIFY(ls != om.m_dummyOut->outputs());
}
