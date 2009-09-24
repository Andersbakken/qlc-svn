/*
  Q Light Controller
  unix-mididevice_test.cpp

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
#include "unix-mididevice_test.h"

#define protected public
#include "unix-mididevice.h"
#include "unix-midiinput.h"
#undef protected

void MIDIDevice_Test::initTestCase()
{
	m_plugin = new MIDIInput();
	m_plugin->init();
}

void MIDIDevice_Test::initial()
{
	MIDIDevice* device = new MIDIDevice(m_plugin);
	QVERIFY(device->address() == NULL);
	QVERIFY(device->name().isEmpty() == true);
	QVERIFY(device->infoText().isEmpty() == false);

	delete device;
}

void MIDIDevice_Test::address()
{
	MIDIDevice* device = new MIDIDevice(m_plugin);
	QVERIFY(device->address() == NULL);

	snd_seq_addr_t addr;
	addr.client = 42;
	addr.port = 15;

	/* Verify that address can be set */
	device->setAddress(&addr);
	const snd_seq_addr_t* addr1 = device->address();
	QVERIFY(addr1 != NULL);
	QVERIFY(addr1->client == 42);
	QVERIFY(addr1->port == 15);

	/* Verify that the *values* are copied and not references */
	addr.client = 99;
	addr.port = 7;
	QVERIFY(addr1->client == 42);
	QVERIFY(addr1->port == 15);

	/* Verify that address can be later changed */
	device->setAddress(&addr);
	const snd_seq_addr_t* addr2 = device->address();
	QVERIFY(addr2 != NULL);
	QVERIFY(addr2->client == 99);
	QVERIFY(addr2->port == 7);

	delete device;
}

void MIDIDevice_Test::name()
{
	MIDIDevice* device = new MIDIDevice(m_plugin);
	QVERIFY(device->name().isEmpty() == true);
	QVERIFY(device->extractName() == false);

	/* Use system timer -- should be present everywhere */
	snd_seq_addr_t addr;
	addr.client = 0;
	addr.port = 0;
	device->setAddress(&addr);
	QVERIFY(device->extractName() == true);
	QVERIFY(device->name().isEmpty() == false);
	QVERIFY(device->name() == "Timer"); // Comment out if test fails

	delete device;
}

void MIDIDevice_Test::cleanupTestCase()
{
	delete m_plugin;
	m_plugin = NULL;
}
