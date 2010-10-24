/*
  Q Light Controller
  midiprotocol_test.cpp

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

#include "midiprotocol_test.h"
#include "midiprotocol.h"
#include "qlctypes.h"

using namespace QLCMIDIProtocol;

void MIDIProtocol_Test::macros()
{
    uchar cmd;
    for (cmd = 0x00; cmd < 0x80; cmd++)
        QVERIFY(MIDI_IS_CMD(cmd) == false);

    for (cmd = 0x80; cmd <= 0x0F; cmd += 0x10)
    {
        for (uchar ch = 0x00; ch <= 0xF; ch++)
        {
            uchar cmdch = cmd | ch;
            QCOMPARE(uchar(MIDI_CH(cmdch)), ch);
            QCOMPARE(uchar(MIDI_CMD(cmdch)), cmd);
            QVERIFY(MIDI_IS_CMD(cmdch) == true);
        }
    }

    QCOMPARE(uchar(MIDI_CH(0xFF)), uchar(0x0F));
    QCOMPARE(uchar(MIDI_CMD(0xFF)), uchar(0xF0));
    QVERIFY(MIDI_IS_CMD(0xFF) == true);
}

void MIDIProtocol_Test::noteToInput()
{
    quint32 ch = 0;
    uchar val = 0;

    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar note = 0; note < 128; note++)
        {
            for (uchar velocity = 0; velocity < 128; velocity++)
            {
                uchar cmd = 0;
                if (velocity == 0)
                    cmd = MIDI_NOTE_OFF;
                else
                    cmd = MIDI_NOTE_ON;
                QCOMPARE(midiToInput(cmd | midich, note, velocity,
                                     midich, &ch, &val), true);
                QCOMPARE(ch, quint32(CHANNEL_OFFSET_NOTE + note));
                QCOMPARE(val, MIDI2DMX(velocity));
            }
        }
    }

    // Check that a non-command (<0x80) doesn't succeed
    QCOMPARE(midiToInput(0x60, 15, 127, 0, &ch, &val), false);
}

void MIDIProtocol_Test::noteAftertouchToInput()
{
    quint32 ch = 0;
    uchar val = 0;

    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar note = 0; note < 128; note++)
        {
            for (uchar velocity = 0; velocity < 128; velocity++)
            {
                QCOMPARE(midiToInput(MIDI_NOTE_AFTERTOUCH | midich, note,
                                     velocity, midich, &ch, &val), true);
                QCOMPARE(ch, quint32(CHANNEL_OFFSET_NOTE_AFTERTOUCH + note));
                QCOMPARE(val, MIDI2DMX(velocity));
            }
        }
    }
}

void MIDIProtocol_Test::noteToInputAnyMidiChannel()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar note = 0; note < 128; note++)
        {
            for (uchar velocity = 0; velocity < 128; velocity++)
            {
                quint32 ch = 0;
                uchar val = 0;
                QCOMPARE(midiToInput(MIDI_NOTE_ON | midich, note, velocity,
                                     16, &ch, &val), true);
                QCOMPARE(ch, quint32(CHANNEL_OFFSET_NOTE + note));
                QCOMPARE(val, MIDI2DMX(velocity));
            }
        }
    }
}

void MIDIProtocol_Test::noteToInputWrongMidiChannel()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar note = 0; note < 128; note++)
        {
            for (uchar velocity = 0; velocity < 128; velocity++)
            {
                quint32 ch = 0;
                uchar val = 0;
                QCOMPARE(midiToInput(MIDI_NOTE_ON | (15 - midich), note,
                                     velocity, midich, &ch, &val), false);
            }
        }
    }
}

void MIDIProtocol_Test::singleChannelCommandsToInput()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar data1 = 0; data1 < 128; data1++)
        {
            quint32 ch = 0;
            uchar val = 0;
            QCOMPARE(midiToInput(MIDI_PROGRAM_CHANGE | midich, data1, 0x00,
                                 midich, &ch, &val), true);
            QCOMPARE(ch, quint32(CHANNEL_OFFSET_PROGRAM_CHANGE));
            QCOMPARE(val, uchar(MIDI2DMX(data1)));

            ch = val = 0;

            QCOMPARE(midiToInput(MIDI_CHANNEL_AFTERTOUCH | midich, data1, 0x00,
                                 midich, &ch, &val), true);
            QCOMPARE(ch, quint32(CHANNEL_OFFSET_CHANNEL_AFTERTOUCH));
            QCOMPARE(val, uchar(MIDI2DMX(data1)));

            ch = val = 0;

            QCOMPARE(midiToInput(MIDI_PITCH_WHEEL | midich, 0x00, data1,
                                 midich, &ch, &val), true);
            QCOMPARE(ch, quint32(CHANNEL_OFFSET_PITCH_WHEEL));
            QCOMPARE(val, uchar(MIDI2DMX(data1)));
        }
    }
}

void MIDIProtocol_Test::ccToInput()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar cc = 0; cc < 128; cc++)
        {
            for (uchar data = 0; data < 128; data++)
            {
                quint32 ch = 0;
                uchar val = 0;
                QCOMPARE(midiToInput(MIDI_CONTROL_CHANGE | midich, cc, data,
                                     midich, &ch, &val), true);
                QCOMPARE(ch, quint32(CHANNEL_OFFSET_CONTROL_CHANGE + cc));
                QCOMPARE(val, MIDI2DMX(data));
            }
        }
    }
}

void MIDIProtocol_Test::ccToInputAnyMidiChannel()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar cc = 0; cc < 128; cc++)
        {
            for (uchar data = 0; data < 128; data++)
            {
                quint32 ch = 0;
                uchar val = 0;
                QCOMPARE(midiToInput(MIDI_CONTROL_CHANGE | midich, cc, data,
                                     16, &ch, &val), true);
                QCOMPARE(ch, quint32(CHANNEL_OFFSET_CONTROL_CHANGE + cc));
                QCOMPARE(val, MIDI2DMX(data));
            }
        }
    }
}

void MIDIProtocol_Test::ccToInputWrongMidiChannel()
{
    for (uchar midich = 0; midich < 16; midich++)
    {
        for (uchar cc = 0; cc < 128; cc++)
        {
            for (uchar data = 0; data < 128; data++)
            {
                quint32 ch = 0;
                uchar val = 0;
                QCOMPARE(midiToInput(MIDI_CONTROL_CHANGE | (15 - midich), cc,
                                     data, midich, &ch, &val), false);
            }
        }
    }
}

void MIDIProtocol_Test::inputToNote()
{
    for (quint32 ch = CHANNEL_OFFSET_NOTE; ch < CHANNEL_OFFSET_NOTE_MAX; ch++)
    {
        for (uchar val = 0; val < 255; val++)
        {
            for (uchar midich = 0; midich < 16; midich++)
            {
                uchar cmd = 0;
                uchar data1 = 0;
                uchar data2 = 0;
                bool d2v = false;

                QCOMPARE(feedbackToMidi(ch, val, midich, &cmd, &data1, &data2,
                                        &d2v), true);
                if (val == 0)
                    QCOMPARE(cmd, uchar(MIDI_NOTE_OFF | midich));
                else
                    QCOMPARE(cmd, uchar(MIDI_NOTE_ON | midich));
                QCOMPARE(data1, uchar(ch - CHANNEL_OFFSET_NOTE));
                QCOMPARE(data2, DMX2MIDI(val));
                QCOMPARE(d2v, true);
            }
        }
    }
}

void MIDIProtocol_Test::inputToControl()
{
    for (quint32 ch = CHANNEL_OFFSET_CONTROL_CHANGE;
         ch < CHANNEL_OFFSET_CONTROL_CHANGE_MAX; ch++)
    {
        for (uchar val = 0; val < 255; val++)
        {
            for (uchar midich = 0; midich < 16; midich++)
            {
                uchar cmd = 0;
                uchar data1 = 0;
                uchar data2 = 0;
                bool d2v = false;

                QCOMPARE(feedbackToMidi(ch, val, midich, &cmd, &data1, &data2,
                                        &d2v), true);
                QCOMPARE(cmd, uchar(MIDI_CONTROL_CHANGE | midich));
                QCOMPARE(data1, uchar(ch - CHANNEL_OFFSET_CONTROL_CHANGE));
                QCOMPARE(data2, DMX2MIDI(val));
                QCOMPARE(d2v, true);
            }
        }
    }
}

void MIDIProtocol_Test::inputToNoteAftertouch()
{
    for (quint32 ch = CHANNEL_OFFSET_NOTE_AFTERTOUCH;
         ch < CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX; ch++)
    {
        for (uchar val = 0; val < 255; val++)
        {
            for (uchar midich = 0; midich < 16; midich++)
            {
                uchar cmd = 0;
                uchar data1 = 0;
                uchar data2 = 0;
                bool d2v = false;

                QCOMPARE(feedbackToMidi(ch, val, midich, &cmd, &data1, &data2,
                                        &d2v), true);
                QCOMPARE(cmd, uchar(MIDI_NOTE_AFTERTOUCH | midich));
                QCOMPARE(data1, uchar(ch - CHANNEL_OFFSET_NOTE_AFTERTOUCH));
                QCOMPARE(data2, DMX2MIDI(val));
                QCOMPARE(d2v, true);
            }
        }
    }
}

void MIDIProtocol_Test::unknownInputCh()
{
    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;
    bool d2v = false;

    QCOMPARE(feedbackToMidi(1000, 10, 0, &cmd, &data1, &data2, &d2v), false);
}

void MIDIProtocol_Test::inputToSingleChannelCommands()
{
    for (uchar val = 0; val < 127; val++)
    {
        for (uchar midich = 0; midich < 16; midich++)
        {
            uchar cmd = 0;
            uchar data1 = 0;
            uchar data2 = 0;
            bool d2v = false;

            QCOMPARE(feedbackToMidi(CHANNEL_OFFSET_CHANNEL_AFTERTOUCH, val,
                                    midich, &cmd, &data1, &data2, &d2v), true);
            QCOMPARE(cmd, uchar(MIDI_CHANNEL_AFTERTOUCH | midich));
            QCOMPARE(data1, uchar(DMX2MIDI(val)));
            QCOMPARE(d2v, false);

            cmd = data1 = data2 = 0;

            QCOMPARE(feedbackToMidi(CHANNEL_OFFSET_PROGRAM_CHANGE, val,
                                    midich, &cmd, &data1, &data2, &d2v), true);
            QCOMPARE(cmd, uchar(MIDI_PROGRAM_CHANGE | midich));
            QCOMPARE(data1, uchar(DMX2MIDI(val)));
            QCOMPARE(d2v, false);

            cmd = data1 = data2 = 0;

            QCOMPARE(feedbackToMidi(CHANNEL_OFFSET_PITCH_WHEEL, val,
                                    midich, &cmd, &data1, &data2, &d2v), true);
            QCOMPARE(cmd, uchar(MIDI_PITCH_WHEEL | midich));
            QCOMPARE(data1, uchar(DMX2MIDI(val)));
            QCOMPARE(d2v, false);
        }
    }
}
