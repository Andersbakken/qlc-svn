/*
  Q Light Controller
  main.cpp

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

#include <QApplication>
#include <QtTest>

#ifdef Q_WS_WIN
#else
#	include "unix-mididevice_test.h"
#	include "unix-midiinput_test.h"
#endif

#include "midiinputevent_test.h"

int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);
	int r;

	MIDIInputEvent_Test event;
	r = QTest::qExec(&event, argc, argv);
	if (r != 0)
		return r;

	MIDIDevice_Test device;
	r = QTest::qExec(&device, argc, argv);
	if (r != 0)
		return r;

	MIDIInput_Test input;
	r = QTest::qExec(&input, argc, argv);
	if (r != 0)
		return r;

	return 0;
}
