/*
  Q Light Controller
  midiprotocol.h

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

#ifndef MIDIPROTOCOL_H
#define MIDIPROTOCOL_H

#define MIDI_NOTE_ON        0x90
#define MIDI_NOTE_OFF       0x80
#define MIDI_CONTROL_CHANGE 0xB0

#define DMX2MIDI(x) static_cast <char> (SCALE(qreal(static_cast <uchar> (x)), \
			qreal(0), qreal(UCHAR_MAX), qreal(0), qreal(127)))

#endif
