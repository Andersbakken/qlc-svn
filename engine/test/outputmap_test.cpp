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

#include <QSignalSpy>
#include <QtTest>

#include "outputpluginstub.h"
#include "outputmap_test.h"
#include "universearray.h"
#include "qlcoutplugin.h"
#include "qlcfile.h"

#define protected public
#include "outputpatch.h"
#include "outputmap.h"
#undef protected

#define TESTPLUGINDIR "../outputpluginstub"
#define INPUT_TESTPLUGINDIR "../inputpluginstub"
#define ENGINEDIR "../src"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void OutputMap_Test::initial()
{
    OutputMap om(this);
    QVERIFY(om.m_universes == 4);
    QVERIFY(om.universes() == 4);
    QVERIFY(om.m_blackout == false);
    QVERIFY(om.blackout() == false);
    QVERIFY(om.m_universeArray->size() == 512 * 4);
    QVERIFY(om.m_universeChanged == false);
    QVERIFY(om.m_plugins.size() == 0);
    QVERIFY(om.m_patch.size() == 4);
    QVERIFY(om.m_universeMutex.tryLock() == true);
    om.m_universeMutex.unlock();

    // Test that peekUniverses() returns valid stuff
    for (quint32 i = 0; i < 512 * om.universes(); i++)
        QVERIFY(om.peekUniverses()->preGMValues().data()[i] == 0);
}

void OutputMap_Test::appendPlugin()
{
    OutputMap om(this);
    QVERIFY(om.m_plugins.size() == 0);

    QSignalSpy spy(&om, SIGNAL(pluginAdded(const QString&)));

    om.loadPlugins(testPluginDir());
    QCOMPARE(om.m_plugins.size(), 1);
    QCOMPARE(spy.size(), 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.appendPlugin(stub) == false);
    QVERIFY(om.plugin(stub->name()) == stub);
    QVERIFY(om.plugin("Foobar") == NULL);
    QVERIFY(om.m_plugins.size() == 1);
}

void OutputMap_Test::notOutputPlugin()
{
    OutputMap om(this);
    QCOMPARE(om.m_plugins.size(), 0);

    // Loading should fail because the plugin is not an output plugin
    QDir dir(testPluginDir());
    dir.setPath(INPUT_TESTPLUGINDIR);
    om.loadPlugins(dir);
    QCOMPARE(om.m_plugins.size(), 0);

    // Loading should fail because the engine lib is not a plugin at all
    dir.setPath(ENGINEDIR);
    om.loadPlugins(dir);
    QCOMPARE(om.m_plugins.size(), 0);
}

void OutputMap_Test::setPatch()
{
    OutputMap om(this);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.setPatch(0, "Foobar", 0) == true);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == KOutputInvalid);
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == KOutputInvalid);
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == KOutputInvalid);
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == KOutputInvalid);

    QVERIFY(om.setPatch(KUniverseCount, stub->name(), 0) == false);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == KOutputInvalid);
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == KOutputInvalid);
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == KOutputInvalid);
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == KOutputInvalid);

    QVERIFY(om.setPatch(4, stub->name(), 4) == false);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == KOutputInvalid);
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == KOutputInvalid);
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == KOutputInvalid);
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == KOutputInvalid);

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

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
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

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
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

    QVERIFY(om.pluginNames().size() == 0);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.pluginNames().size() == 1);
    QVERIFY(om.pluginNames().at(0) == stub->name());
}

void OutputMap_Test::pluginOutputs()
{
    OutputMap om(this);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QStringList ls(om.pluginOutputs(stub->name()));
    QVERIFY(ls == stub->outputs());

    QVERIFY(om.pluginOutputs("Foobar").isEmpty() == true);
}

void OutputMap_Test::universeNames()
{
    OutputMap om(this);

    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QVERIFY(om.universeNames().at(0).contains("None"));
    QVERIFY(om.universeNames().at(1).contains("None"));
    QVERIFY(om.universeNames().at(2).contains("None"));
    QVERIFY(om.universeNames().at(3).contains("None"));

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 3);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: None (None)"));
    QCOMPARE(om.universeNames().at(2), QString("3: None (None)"));
    QCOMPARE(om.universeNames().at(3), QString("4: None (None)"));

    om.setPatch(3, stub->name(), 2);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: None (None)"));
    QCOMPARE(om.universeNames().at(2), QString("3: None (None)"));
    QCOMPARE(om.universeNames().at(3), QString("4: Output Plugin Stub (3: Stub 3)"));
}

void OutputMap_Test::configure()
{
    OutputMap om(this);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
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

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
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

    for (quint32 i = 0; i < 20; i++)
        QCOMPARE(om.mapping("Dummy Output", i), KOutputInvalid);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.setPatch(1, stub->name(), quint32(0)) == true);
    QCOMPARE(om.mapping("Dummy Output", 0), KOutputInvalid);
    QCOMPARE(om.mapping("Dummy Output", 1), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping("Dummy Output", 2), KOutputInvalid);
    QCOMPARE(om.mapping("Dummy Output", 3), KOutputInvalid);
    QCOMPARE(om.mapping(stub->name(), 0), quint32(1));
    QCOMPARE(om.mapping(stub->name(), 1), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping(stub->name(), 2), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping(stub->name(), 3), OutputMap::invalidUniverse());
}

void OutputMap_Test::pluginStatus()
{
    OutputMap om(this);

    QVERIFY(om.pluginStatus("Foo").contains("No plugin"));
    QVERIFY(om.pluginStatus("Bar", 0).contains("No plugin"));
    QVERIFY(om.pluginStatus("Baz", 1).contains("No plugin"));
    QVERIFY(om.pluginStatus("Xyzzy", 2).contains("No plugin"));
    QVERIFY(om.pluginStatus("AYBABTU", 3).contains("No plugin"));

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    om.appendPlugin(stub);
    QVERIFY(om.pluginStatus(stub->name()) == stub->infoText(KInputInvalid));
    QVERIFY(om.pluginStatus(stub->name(), 0) == stub->infoText(0));
    QVERIFY(om.pluginStatus(stub->name(), 1) == stub->infoText(1));
    QVERIFY(om.pluginStatus(stub->name(), 2) == stub->infoText(2));
}
