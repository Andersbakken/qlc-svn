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

#include "outputpluginstub.h"
#include "outputmap_test.h"
#include "universearray.h"
#include "qlcoutplugin.h"

#define protected public
#include "outputpatch.h"
#include "outputmap.h"
#undef protected

#define TESTPLUGINDIR "../outputpluginstub"
#define INPUT_TESTPLUGINDIR "../inputpluginstub"
#define ENGINEDIR "../src"

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

    // Test that peekUniverses() returns valid stuff
    for (quint32 i = 0; i < 512 * om.universes(); i++)
        QVERIFY(om.peekUniverses()->preGMValues().data()[i] == 0);

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
    QVERIFY(om.m_plugins.size() == 1);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() == 2);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QVERIFY(om.appendPlugin(stub) == false);
    QVERIFY(om.plugin(stub->name()) == stub);
    QVERIFY(om.plugin(om.m_dummyOut->name()) == om.m_dummyOut);

    // Nonexistent location
    om.loadPlugins("foobarxyzzy42");
    QVERIFY(om.m_plugins.size() == 2);
    QVERIFY(om.plugin(stub->name()) == stub);
    QVERIFY(om.plugin(om.m_dummyOut->name()) == om.m_dummyOut);
}

void OutputMap_Test::notOutputPlugin()
{
    OutputMap om(this);
    QCOMPARE(om.m_plugins.size(), 1);

    // Loading should fail because the plugin is not an output plugin
    om.loadPlugins(INPUT_TESTPLUGINDIR);
    QCOMPARE(om.m_plugins.size(), 1);

    // Loading should fail because the engine lib is not a plugin at all
    om.loadPlugins(ENGINEDIR);
    QCOMPARE(om.m_plugins.size(), 1);
}

void OutputMap_Test::setPatch()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

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

void OutputMap_Test::claimReleaseDumpReset()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 0);
    om.setPatch(1, stub->name(), 1);
    om.setPatch(2, stub->name(), 2);
    om.setPatch(3, stub->name(), 3);

    UniverseArray* unis = om.claimUniverses();
    for (int i = 0; i < 512; i++)
        unis->write(i, 'a', QLCChannel::Intensity);
    for (int i = 512; i < 1024; i++)
        unis->write(i, 'b', QLCChannel::Intensity);
    for (int i = 1024; i < 1536; i++)
        unis->write(i, 'c', QLCChannel::Intensity);
    for (int i = 1536; i < 2048; i++)
        unis->write(i, 'd', QLCChannel::Intensity);
    om.releaseUniverses();

    om.dumpUniverses();

    for (int i = 0; i < 512; i++)
        QCOMPARE(stub->m_array.data()[i], 'a');

    for (int i = 512; i < 1024; i++)
        QCOMPARE(stub->m_array.data()[i], 'b');

    for (int i = 1024; i < 1536; i++)
        QCOMPARE(stub->m_array.data()[i], 'c');

    for (int i = 1536; i < 2048; i++)
        QCOMPARE(stub->m_array.data()[i], 'd');

    om.resetUniverses();
    for (quint32 i = 0; i < 512 * om.universes(); i++)
        QVERIFY(om.peekUniverses()->preGMValues().data()[i] == 0);
}

void OutputMap_Test::blackout()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 0);
    om.setPatch(1, stub->name(), 1);
    om.setPatch(2, stub->name(), 2);
    om.setPatch(3, stub->name(), 3);

    UniverseArray* unis = om.claimUniverses();
    for (int i = 0; i < 512; i++)
        unis->write(i, 'a', QLCChannel::Intensity);
    for (int i = 512; i < 1024; i++)
        unis->write(i, 'b', QLCChannel::Intensity);
    for (int i = 1024; i < 1536; i++)
        unis->write(i, 'c', QLCChannel::Intensity);
    for (int i = 1536; i < 2048; i++)
        unis->write(i, 'd', QLCChannel::Intensity);
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

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QVERIFY(om.pluginNames().size() == 2);
    QVERIFY(om.pluginNames().at(0) == om.m_dummyOut->name());
    QVERIFY(om.pluginNames().at(1) == stub->name());
}

void OutputMap_Test::pluginOutputs()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QStringList ls(om.pluginOutputs(stub->name()));
    QVERIFY(ls == stub->outputs());
    QVERIFY(ls != om.m_dummyOut->outputs());
}

void OutputMap_Test::universeNames()
{
    OutputMap om(this);

    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Dummy Output (1: Dummy Out 1)"));
    QCOMPARE(om.universeNames().at(1), QString("2: Dummy Output (2: Dummy Out 2)"));
    QCOMPARE(om.universeNames().at(2), QString("3: Dummy Output (3: Dummy Out 3)"));
    QCOMPARE(om.universeNames().at(3), QString("4: Dummy Output (4: Dummy Out 4)"));

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 3);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: Dummy Output (2: Dummy Out 2)"));
    QCOMPARE(om.universeNames().at(2), QString("3: Dummy Output (3: Dummy Out 3)"));
    QCOMPARE(om.universeNames().at(3), QString("4: Dummy Output (4: Dummy Out 4)"));

    om.setPatch(3, stub->name(), 2);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: Dummy Output (2: Dummy Out 2)"));
    QCOMPARE(om.universeNames().at(2), QString("3: Dummy Output (3: Dummy Out 3)"));
    QCOMPARE(om.universeNames().at(3), QString("4: Output Plugin Stub (3: Stub 3)"));
}

void OutputMap_Test::configure()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QCOMPARE(om.canConfigurePlugin("Foo"), false);
    QCOMPARE(om.canConfigurePlugin(stub->name()), false);
    stub->m_canConfigure = true;
    QCOMPARE(om.canConfigurePlugin(stub->name()), true);

    om.configurePlugin("Foo");
    QCOMPARE(stub->m_configureCalled, 0);
    om.configurePlugin(stub->name());
    QCOMPARE(stub->m_configureCalled, 1);
}

void OutputMap_Test::slotConfigurationChanged()
{
    OutputMap om(this);

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QSignalSpy spy(&om, SIGNAL(pluginConfigurationChanged(QString)));
    stub->configure();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(stub->name()));
}

void OutputMap_Test::mapping()
{
    OutputMap om(this);

    QCOMPARE(om.mapping("Dummy Output", 0), quint32(0));
    QCOMPARE(om.mapping("Dummy Output", 1), quint32(1));
    QCOMPARE(om.mapping("Dummy Output", 2), quint32(2));
    QCOMPARE(om.mapping("Dummy Output", 3), quint32(3));

    om.loadPlugins(TESTPLUGINDIR);
    QVERIFY(om.m_plugins.size() > 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(1));
    QVERIFY(stub != NULL);

    QVERIFY(om.setPatch(1, stub->name(), quint32(0)) == true);
    QCOMPARE(om.mapping("Dummy Output", 0), quint32(0));
    QCOMPARE(om.mapping("Dummy Output", 1), KOutputInvalid);
    QCOMPARE(om.mapping("Dummy Output", 2), quint32(2));
    QCOMPARE(om.mapping("Dummy Output", 3), quint32(3));
    QCOMPARE(om.mapping(stub->name(), 0), quint32(1));
    QCOMPARE(om.mapping(stub->name(), 1), KOutputInvalid);
    QCOMPARE(om.mapping(stub->name(), 2), KOutputInvalid);
    QCOMPARE(om.mapping(stub->name(), 3), KOutputInvalid);
}
