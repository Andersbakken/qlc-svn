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

/****************************************************************************
 * MIDI helper macros
 ****************************************************************************/
/** Extract the MIDI channel part from a MIDI message (0x*0 - 0x*F) */
#define MIDI_CH(x) (x & 0x0F)

/** Extract the MIDI command part from a MIDI message (0x8* - 0xF*) */
#define MIDI_CMD(x) (x & 0xF0)

/****************************************************************************
 * MIDI commands
 ****************************************************************************/
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_NOTE_AFTERTOUCH    0xA0
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_PROGRAM_CHANGE     0xC0
#define MIDI_CHANNEL_AFTERTOUCH 0xD0
#define MIDI_PITCH_WHEEL        0xE0
#define MIDI_SYSEX              0xF0
#define MIDI_SYSEX_EOX          0x7F
#define MIDI_TIME_CODE          0xF1
#define MIDI_SONG_POSITION      0xF2
#define MIDI_SONG_SELECT        0xF3

/****************************************************************************
 * MIDI control/msg -> QLC input channel mappings
 ****************************************************************************/
#define CHANNEL_OFFSET_NOTE                0
#define CHANNEL_OFFSET_NOTE_MAX            127

#define CHANNEL_OFFSET_CONTROL_CHANGE      128
#define CHANNEL_OFFSET_CONTROL_CHANGE_MAX  255

#define CHANNEL_OFFSET_NOTE_AFTERTOUCH     256
#define CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX 383

#define CHANNEL_OFFSET_CHANNEL_AFTERTOUCH  384
#define CHANNEL_OFFSET_PROGRAM_CHANGE      385
#define CHANNEL_OFFSET_PITCH_WHEEL         386

#endif
