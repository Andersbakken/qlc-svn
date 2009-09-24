/*
  Q Light Controller
  unix-midiinput_test.cpp

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
#include <QDebug>

#include "unix-midiinput_test.h"
#define protected public
#include "../unix-midiinput.h"
#include "../unix-midipoller.h"
#undef protected

void MIDIInput_Test::initial()
{
	MIDIInput plugin;
	QVERIFY(plugin.name() == "MIDI Input");
	QVERIFY(plugin.devices().size() == 0);
	QVERIFY(plugin.inputs().size() == 0);

	plugin.init();
	QVERIFY(plugin.alsa() != NULL);
	QVERIFY(plugin.address() != NULL);
	QVERIFY(plugin.m_poller != NULL);
	QVERIFY(plugin.m_poller->m_running == false);

	/* If ALSA has been set up correctly, there should be at least one
	   valid input device. */
	QVERIFY(plugin.devices().size() != 0);
	QVERIFY(plugin.inputs().size() != 0);
}

void MIDIInput_Test::openClose()
{
	MIDIInput plugin;
	plugin.init();

	QVERIFY(plugin.m_poller->m_devices.size() == 0);

	plugin.open(0);
	QVERIFY(plugin.m_poller->m_devices.size() == 1);
	QVERIFY(plugin.m_poller->m_running == true);

	plugin.open(31337);
	QVERIFY(plugin.m_poller->m_devices.size() == 1);

	plugin.open(0);
	QVERIFY(plugin.m_poller->m_devices.size() == 1);

	plugin.close(1);
	QVERIFY(plugin.m_poller->m_devices.size() == 1);

	plugin.close(0);
	QVERIFY(plugin.m_poller->m_devices.size() == 0);
	QVERIFY(plugin.m_poller->m_running == false);
}
