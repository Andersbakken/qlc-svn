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

#include <QSignalSpy>
#include <QtTest>

#include "inputpluginstub.h"
#include "inputmap_test.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define protected public
#include "inputpatch.h"
#include "inputmap.h"
#undef protected

#define TESTPLUGINDIR "../inputpluginstub"
#define OUTPUT_TESTPLUGINDIR "../outputpluginstub"
#define ENGINEDIR "../src"
#define PROFILEDIR "../../inputprofiles"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

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

void InputMap_Test::editorUniverse()
{
    InputMap im(this);

    QVERIFY(im.editorUniverse() == 0);
    im.setEditorUniverse(3);
    QVERIFY(im.editorUniverse() == 3);
    im.setEditorUniverse(4);
    QVERIFY(im.editorUniverse() == 0);
    im.setEditorUniverse(1);
    QVERIFY(im.editorUniverse() == 1);
    im.setEditorUniverse(2);
    QVERIFY(im.editorUniverse() == 2);
}

void InputMap_Test::appendPlugin()
{
    InputMap im(this);
    QCOMPARE(im.m_plugins.size(), 0);

    QSignalSpy spy(&im, SIGNAL(pluginAdded(const QString&)));

    im.loadPlugins(testPluginDir());
    QCOMPARE(im.m_plugins.size(), 1);
    QCOMPARE(spy.size(), 1);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QCOMPARE(im.appendPlugin(stub), false);
    QCOMPARE(im.plugin(stub->name()), stub);
    QVERIFY(im.plugin("Foobar") == NULL);
    QVERIFY(im.m_plugins.size() == 1);
    QCOMPARE(im.plugin(stub->name()), stub);
    QCOMPARE(spy.size(), 1);
}

void InputMap_Test::notInputPlugin()
{
    InputMap im(this);
    QCOMPARE(im.m_plugins.size(), 0);

    // Loading should fail because the plugin is not an input plugin
    QDir dir(testPluginDir());
    dir.setPath(OUTPUT_TESTPLUGINDIR);
    im.loadPlugins(dir);
    QCOMPARE(im.m_plugins.size(), 0);

    // Loading should fail because the engine lib is not a plugin at all
    dir.setPath(ENGINEDIR);
    im.loadPlugins(dir);
    QCOMPARE(im.m_plugins.size(), 0);
}

void InputMap_Test::pluginNames()
{
    InputMap im(this);

    QVERIFY(im.pluginNames().size() == 0);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginNames().size() == 1);
    QVERIFY(im.pluginNames().at(0) == stub->name());
}

void InputMap_Test::pluginInputs()
{
    InputMap im(this);

    QVERIFY(im.pluginInputs("Foo").size() == 0);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginInputs(stub->name()).size() == 4);
    QVERIFY(im.pluginInputs(stub->name()) == stub->inputs());
}

void InputMap_Test::configurePlugin()
{
    InputMap im(this);

    QCOMPARE(im.canConfigurePlugin("Foo"), false);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QCOMPARE(im.canConfigurePlugin("Foo"), false);
    QCOMPARE(im.canConfigurePlugin(stub->name()), false);
    stub->m_canConfigure = true;
    QCOMPARE(im.canConfigurePlugin(stub->name()), true);

    /* Must be able to call multiple times */
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 1);
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 2);
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 3);
}

void InputMap_Test::pluginStatus()
{
    InputMap im(this);

    QVERIFY(im.pluginStatus("Foo").contains("No plugin"));
    QVERIFY(im.pluginStatus("Bar", 0).contains("No plugin"));
    QVERIFY(im.pluginStatus("Baz", 1).contains("No plugin"));
    QVERIFY(im.pluginStatus("Xyzzy", 2).contains("No plugin"));
    QVERIFY(im.pluginStatus("AYBABTU", 3).contains("No plugin"));

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    im.appendPlugin(stub);
    QVERIFY(im.pluginStatus(stub->name()) == stub->infoText(KInputInvalid));
    QVERIFY(im.pluginStatus(stub->name(), 0) == stub->infoText(0));
    QVERIFY(im.pluginStatus(stub->name(), 1) == stub->infoText(1));
    QVERIFY(im.pluginStatus(stub->name(), 2) == stub->infoText(2));
}

void InputMap_Test::profiles()
{
    InputMap im(this);
    QVERIFY(im.m_profiles.size() == 0);

    QLCInputProfile* prof = new QLCInputProfile();
    prof->setManufacturer("Foo");
    prof->setModel("Bar");

    QVERIFY(im.addProfile(prof) == true);
    QVERIFY(im.m_profiles.size() == 1);
    QVERIFY(im.addProfile(prof) == false);
    QVERIFY(im.m_profiles.size() == 1);

    QVERIFY(im.profileNames().size() == 1);
    QVERIFY(im.profileNames().at(0) == prof->name());
    QVERIFY(im.profile(prof->name()) == prof);
    QVERIFY(im.profile("Foobar") == NULL);

    QVERIFY(im.removeProfile("Foobar") == false);
    QVERIFY(im.m_profiles.size() == 1);
    QVERIFY(im.removeProfile(prof->name()) == true);
    QVERIFY(im.m_profiles.size() == 0);
}

void InputMap_Test::setPatch()
{
    InputMap im(this);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QLCInputProfile* prof = new QLCInputProfile();
    prof->setManufacturer("Foo");
    prof->setModel("Bar");
    im.addProfile(prof);

    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == KInputInvalid);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.patch(0)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 0) == InputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == KInputInvalid);
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.patch(1)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == KInputInvalid);
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.patch(2)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == KInputInvalid);
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.patch(3)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, "Foobar", 0, false, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == KInputInvalid);
    QVERIFY(im.patch(0)->profile() == prof);
    QVERIFY(im.patch(0)->feedbackEnabled() == false);
    QVERIFY(im.mapping(stub->name(), 0) == InputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == KInputInvalid);
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.patch(1)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == KInputInvalid);
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.patch(2)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == KInputInvalid);
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.patch(3)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, stub->name(), 0, true) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.patch(0)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == KInputInvalid);
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.patch(1)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == KInputInvalid);
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.patch(2)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == KInputInvalid);
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.patch(3)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(2, stub->name(), 3, false, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.patch(0)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == KInputInvalid);
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.patch(1)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == stub);
    QVERIFY(im.patch(2)->input() == 3);
    QVERIFY(im.patch(2)->profile() == prof);
    QVERIFY(im.patch(2)->feedbackEnabled() == false);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == KInputInvalid);
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.patch(3)->feedbackEnabled() == true);
    QVERIFY(im.mapping(stub->name(), 3) == 2);

    // Universe out of bounds
    QVERIFY(im.setPatch(im.universes(), stub->name(), 0, true) == false);
}

void InputMap_Test::feedBack()
{
    InputMap im(this);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    im.setPatch(0, stub->name(), 0, true);

    QVERIFY(im.feedBack(0, 39, 42) == true);
    QVERIFY(stub->m_feedBackInput == 0);
    QVERIFY(stub->m_feedBackChannel == 39);
    QVERIFY(stub->m_feedBackValue == 42);

    QVERIFY(im.feedBack(1, 2, 3) == false);
    QVERIFY(stub->m_feedBackInput == 0);
    QVERIFY(stub->m_feedBackChannel == 39);
    QVERIFY(stub->m_feedBackValue == 42);

    QVERIFY(im.feedBack(4, 2, 3) == false);
    QVERIFY(stub->m_feedBackInput == 0);
    QVERIFY(stub->m_feedBackChannel == 39);
    QVERIFY(stub->m_feedBackValue == 42);

    QVERIFY(im.feedBack(0, 2, 3) == true);
    QVERIFY(stub->m_feedBackInput == 0);
    QVERIFY(stub->m_feedBackChannel == 2);
    QVERIFY(stub->m_feedBackValue == 3);

    // Disable feedback
    im.setPatch(0, stub->name(), 0, false);
    QVERIFY(im.feedBack(0, 54, 12) == false);
    QVERIFY(stub->m_feedBackInput == 0);
    QVERIFY(stub->m_feedBackChannel == 2);
    QVERIFY(stub->m_feedBackValue == 3);
}

void InputMap_Test::slotValueChanged()
{
    InputMap im(this);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.setPatch(0, stub->name(), 0, false) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);

    QSignalSpy spy(&im, SIGNAL(inputValueChanged(quint32, quint32, uchar)));
    stub->emitValueChanged(0, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    /* Invalid mapping for this plugin -> no signal */
    stub->emitValueChanged(3, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    /* Invalid mapping for this plugin -> no signal */
    stub->emitValueChanged(1, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    stub->emitValueChanged(0, 5, 127);
    QVERIFY(spy.size() == 2);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);
    QVERIFY(spy.at(1).at(0) == 0);
    QVERIFY(spy.at(1).at(1) == 5);
    QVERIFY(spy.at(1).at(2) == 127);
}

void InputMap_Test::slotConfigurationChanged()
{
    InputMap im(this);

    im.loadPlugins(testPluginDir());
    QVERIFY(im.m_plugins.size() > 0);
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));
    QVERIFY(stub != NULL);

    QSignalSpy spy(&im, SIGNAL(pluginConfigurationChanged(QString)));
    stub->configure();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(stub->name()));
}

void InputMap_Test::loadInputProfiles()
{
    InputMap im(this);

    // No profiles in a nonexistent directory
    QDir dir("/path/to/a/nonexistent/place/beyond/this/universe");
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);
    QVERIFY(im.profileNames().isEmpty() == true);

    // No profiles in an existing directory
    dir = testPluginDir();
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);
    QVERIFY(im.profileNames().isEmpty() == true);

    // Should be able to load profiles
    dir.setPath(PROFILEDIR);
    im.loadProfiles(dir);
    QStringList names(im.profileNames());
    QVERIFY(names.size() > 0);

    // Shouldn't load duplicates
    im.loadProfiles(dir);
    QCOMPARE(names, im.profileNames());
}

void InputMap_Test::inputSourceNames()
{
    InputMap im(this);

    im.loadPlugins(testPluginDir());
    InputPluginStub* stub = static_cast<InputPluginStub*> (im.m_plugins.at(0));

    QDir dir(PROFILEDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);

    QString uni, ch;
    QVERIFY(im.inputSourceNames(0, 0, uni, ch) == false);
    QCOMPARE(uni, QString());
    QCOMPARE(ch, QString());

    QVERIFY(im.setPatch(0, stub->name(), 0, true, QString("Generic MIDI")) == true);
    QVERIFY(im.inputSourceNames(0, 0, uni, ch) == true);
    QCOMPARE(uni, tr("%1: Generic MIDI").arg(1));
    QCOMPARE(ch, tr("%1: Bank select MSB").arg(1));

    uni.clear();
    ch.clear();
    QVERIFY(im.inputSourceNames(0, 50000, uni, ch) == true);
    QCOMPARE(uni, tr("%1: Generic MIDI").arg(1));
    QCOMPARE(ch, tr("%1: Unknown").arg(50001));

    QVERIFY(im.setPatch(0, stub->name(), 0, true, QString()) == true);

    uni.clear();
    ch.clear();
    QVERIFY(im.inputSourceNames(0, 0, uni, ch) == true);
    QCOMPARE(uni, tr("%1: %2").arg(1).arg(stub->name()));
    QCOMPARE(ch, tr("%1: Unknown").arg(1));

    QVERIFY(im.inputSourceNames(0, KInputChannelInvalid, uni, ch) == false);
    QVERIFY(im.inputSourceNames(InputMap::invalidUniverse(), 0, uni, ch) == false);
    QVERIFY(im.inputSourceNames(InputMap::invalidUniverse(),
                                KInputChannelInvalid, uni, ch) == false);
}

void InputMap_Test::profileDirectories()
{
    QDir dir = InputMap::systemProfileDirectory();
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(INPUTPROFILEDIR));

    dir = InputMap::userProfileDirectory();
    QVERIFY(dir.exists() == true);
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(USERINPUTPROFILEDIR));
}

void InputMap_Test::pluginDirectories()
{
    QDir dir = InputMap::systemPluginDirectory();
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtPlugin)));
    QVERIFY(dir.absolutePath().contains(INPUTPLUGINDIR));
}
