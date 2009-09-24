/*
  Q Light Controller
  midiinputevent_test.cpp

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

#include "common/qlctypes.h"

#include "midiinputevent_test.h"
#include "midiinputevent.h"
#ifdef Q_WS_WIN
#	include "win32-mididevice.h"
#else
#	include "unix-mididevice.h"
#endif

void MIDIInputEvent_Test::initial()
{
	MIDIInputEvent ev((MIDIDevice*)0xDEADBEEF, 1337, 42);
	QVERIFY(ev.m_device == (MIDIDevice*)0xDEADBEEF);
	QVERIFY(ev.m_channel == 1337);
	QVERIFY(ev.m_value == 42);
}
