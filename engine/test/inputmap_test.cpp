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
#include "inputpatch.h"
#include "inputmap.h"
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

	InputPluginStub* stub = new InputPluginStub();
	QVERIFY(im.appendPlugin(stub) == true);
	QVERIFY(im.m_plugins.size() == 1);
	QVERIFY(im.plugin(stub->name()) == stub);
	QVERIFY(im.plugin("Foobar") == NULL);
}

void InputMap_Test::pluginNames()
{
	InputMap im(this);

	QVERIFY(im.pluginNames().size() == 0);

	InputPluginStub* stub = new InputPluginStub();
	im.appendPlugin(stub);

	QVERIFY(im.pluginNames().size() == 1);
	QVERIFY(im.pluginNames().at(0) == stub->name());
}

void InputMap_Test::pluginInputs()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();

	QVERIFY(im.pluginInputs(stub->name()).size() == 0);

	im.appendPlugin(stub);

	QVERIFY(im.pluginInputs(stub->name()).size() == 4);
	QVERIFY(im.pluginInputs(stub->name()) == stub->inputs());
}

void InputMap_Test::configurePlugin()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();
	im.configurePlugin(stub->name()); // Just a crash check
	QVERIFY(stub->m_configureCalled == 0);
	im.appendPlugin(stub);

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

	InputPluginStub* stub = new InputPluginStub();
	QVERIFY(im.pluginStatus(stub->name()).contains("No input plugin"));
	QVERIFY(im.pluginStatus(stub->name(), 0).contains("No input plugin"));
	QVERIFY(im.pluginStatus(stub->name(), 1).contains("No input plugin"));
	QVERIFY(im.pluginStatus(stub->name(), 2).contains("No input plugin"));
	QVERIFY(im.pluginStatus(stub->name(), 3).contains("No input plugin"));

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

	InputPluginStub* stub = new InputPluginStub();
	im.appendPlugin(stub);

	QLCInputProfile* prof = new QLCInputProfile();
	prof->setManufacturer("Foo");
	prof->setModel("Bar");
	im.addProfile(prof);

	QVERIFY(im.patch(0)->plugin() == NULL);
	QVERIFY(im.patch(0)->input() == KInputInvalid);
	QVERIFY(im.patch(0)->profile() == NULL);
	QVERIFY(im.patch(0)->feedbackEnabled() == true);

	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->profile() == NULL);
	QVERIFY(im.patch(1)->feedbackEnabled() == true);

	QVERIFY(im.patch(2)->plugin() == NULL);
	QVERIFY(im.patch(2)->input() == KInputInvalid);
	QVERIFY(im.patch(2)->profile() == NULL);
	QVERIFY(im.patch(2)->feedbackEnabled() == true);

	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->profile() == NULL);
	QVERIFY(im.patch(3)->feedbackEnabled() == true);

	QVERIFY(im.setPatch(0, "Foobar", 0, false, prof->name()) == true);
	QVERIFY(im.patch(0)->plugin() == NULL);
	QVERIFY(im.patch(0)->input() == KInputInvalid);
	QVERIFY(im.patch(0)->profile() == prof);
	QVERIFY(im.patch(0)->feedbackEnabled() == false);

	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->profile() == NULL);
	QVERIFY(im.patch(1)->feedbackEnabled() == true);

	QVERIFY(im.patch(2)->plugin() == NULL);
	QVERIFY(im.patch(2)->input() == KInputInvalid);
	QVERIFY(im.patch(2)->profile() == NULL);
	QVERIFY(im.patch(2)->feedbackEnabled() == true);

	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->profile() == NULL);
	QVERIFY(im.patch(3)->feedbackEnabled() == true);

	QVERIFY(im.setPatch(0, stub->name(), 0, true) == true);
	QVERIFY(im.patch(0)->plugin() == stub);
	QVERIFY(im.patch(0)->input() == 0);
	QVERIFY(im.patch(0)->profile() == NULL);
	QVERIFY(im.patch(0)->feedbackEnabled() == true);

	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->profile() == NULL);
	QVERIFY(im.patch(1)->feedbackEnabled() == true);

	QVERIFY(im.patch(2)->plugin() == NULL);
	QVERIFY(im.patch(2)->input() == KInputInvalid);
	QVERIFY(im.patch(2)->profile() == NULL);
	QVERIFY(im.patch(2)->feedbackEnabled() == true);

	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->profile() == NULL);
	QVERIFY(im.patch(3)->feedbackEnabled() == true);

	QVERIFY(im.setPatch(2, stub->name(), 3, false, prof->name()) == true);
	QVERIFY(im.patch(0)->plugin() == stub);
	QVERIFY(im.patch(0)->input() == 0);
	QVERIFY(im.patch(0)->profile() == NULL);
	QVERIFY(im.patch(0)->feedbackEnabled() == true);

	QVERIFY(im.patch(1)->plugin() == NULL);
	QVERIFY(im.patch(1)->input() == KInputInvalid);
	QVERIFY(im.patch(1)->profile() == NULL);
	QVERIFY(im.patch(1)->feedbackEnabled() == true);

	QVERIFY(im.patch(2)->plugin() == stub);
	QVERIFY(im.patch(2)->input() == 3);
	QVERIFY(im.patch(2)->profile() == prof);
	QVERIFY(im.patch(2)->feedbackEnabled() == false);

	QVERIFY(im.patch(3)->plugin() == NULL);
	QVERIFY(im.patch(3)->input() == KInputInvalid);
	QVERIFY(im.patch(3)->profile() == NULL);
	QVERIFY(im.patch(3)->feedbackEnabled() == true);
}

void InputMap_Test::feedBack()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();
	im.appendPlugin(stub);
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

#if 0
void InputMap_Test::slotValueChanged()
{
	InputMap im(this);

	InputPluginStub* stub = new InputPluginStub();
	im.appendPlugin(stub);

	QVERIFY(im.setPatch(0, stub->name(), 0) == true);
	QVERIFY(im.patch(0)->plugin() == stub);
	QVERIFY(im.patch(0)->input() == 0);

	QSignalSpy spy(&im, SIGNAL(inputValueChanged(t_input_universe,
						     t_input_channel,
						     t_input_value)));
	im.slotValueChanged(stub, 0, 15, 255);
	QVERIFY(spy.size() == 1);
	QVERIFY(spy.at(0).at(0) == 0);
	QVERIFY(spy.at(0).at(1) == 15);
	QVERIFY(spy.at(0).at(2) == 255);

	/* Invalid mapping for this plugin -> no signal */
	im.slotValueChanged(stub, 3, 15, 255);
	QVERIFY(spy.size() == 1);
	QVERIFY(spy.at(0).at(0) == 0);
	QVERIFY(spy.at(0).at(1) == 15);
	QVERIFY(spy.at(0).at(2) == 255);

	/* Invalid mapping for this plugin -> no signal */
	im.slotValueChanged(stub, 1, 15, 255);
	QVERIFY(spy.size() == 1);
	QVERIFY(spy.at(0).at(0) == 0);
	QVERIFY(spy.at(0).at(1) == 15);
	QVERIFY(spy.at(0).at(2) == 255);

	im.slotValueChanged(stub, 0, 5, 127);
	QVERIFY(spy.size() == 2);
	QVERIFY(spy.at(0).at(0) == 0);
	QVERIFY(spy.at(0).at(1) == 15);
	QVERIFY(spy.at(0).at(2) == 255);
	QVERIFY(spy.at(1).at(0) == 0);
	QVERIFY(spy.at(1).at(1) == 5);
	QVERIFY(spy.at(1).at(2) == 127);
}
#endif
