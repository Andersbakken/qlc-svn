/*
  Q Light Controller
  mididevice.cpp

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
#include <QSettings>
#include <QVariant>
#include <QObject>
#include <QString>
#include <QDebug>

#include "midiinputevent.h"
#include "midiprotocol.h"
#include "mididevice.h"
#include "midiinput.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

MIDIDevice::MIDIDevice(MIDIInput* parent, MIDIEntityRef entity)
        : QObject(parent),
        m_entity(entity),
        m_source(0),
        m_destination(0),
        m_inPort(0),
        m_outPort(0),
        m_isOK(false),
        m_mode(ControlChange),
        m_midiChannel(0)
{
}

MIDIDevice::~MIDIDevice()
{
    saveSettings();
    close();
}

bool MIDIDevice::extractUID()
{
    OSStatus s;
    SInt32 uid;

    /* Get the UID property */
    s = MIDIObjectGetIntegerProperty(m_entity, kMIDIPropertyUniqueID, &uid);
    if (s == 0)
    {
        m_uid = uid;
        m_isOK = true;
        return true;
    }
    else
    {
        m_uid = -1;
        qWarning() << "Unable to get UID for MIDI entity:" << s;
        return false;
    }
}

bool MIDIDevice::extractName()
{
    CFStringRef str;
    OSStatus s;

    /* Get the name property */
    s = MIDIObjectGetStringProperty(m_entity, kMIDIPropertyModel, &str);
    if (s != 0)
    {
        qWarning() << "Unable to get manufacturer for MIDI entity:"
        << s;
        m_name = QString("Unknown %1").arg(m_uid);
    }
    else
    {
        /* Convert the name into a QString. */
        CFIndex size = CFStringGetLength(str) + 1;
        char* buf = (char*) malloc(size);
        if (CFStringGetCString(str, buf, size,
                               kCFStringEncodingISOLatin1))
        {
            m_name = QString(buf);
        }
        else
        {
            m_name = QString("Unknown %1").arg(m_uid);
        }

        free(buf);
        CFRelease(str);
    }

    return true;
}

void MIDIDevice::loadSettings()
{
    QSettings settings;
    QVariant value;
    QString key;

    /* Attempt to get a MIDI channel from settings */
    key = QString("/midiinput/%1/midichannel").arg(m_name);
    value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);

    /* Attempt to get the mode from settings */
    key = QString("/midiinput/%1/mode").arg(m_name);
    value = settings.value(key);
    if (value.isValid() == true)
        setMode(stringToMode(value.toString()));
    else
        setMode(ControlChange);
}

void MIDIDevice::saveSettings()
{
    QSettings settings;
    QString key;

    /* Store MIDI channel to settings */
    key = QString("/midiinput/%1/midichannel").arg(m_name);
    settings.setValue(key, m_midiChannel);

    /* Store mode to settings */
    key = QString("/midiinput/%1/mode").arg(m_name);
    settings.setValue(key, MIDIDevice::modeToString(m_mode));
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

static void postEvent(MIDIDevice* self, quint32 channelOffset,
                      Byte first, Byte second)
{
    Q_ASSERT(self != NULL);

    quint32 channel = channelOffset + static_cast<quint32> (first);
    uchar value = uchar(SCALE(double(second), double(0), double(127),
                              double(0), double(UCHAR_MAX)));

    MIDIInputEvent* event = new MIDIInputEvent(self, channel, value);
    QApplication::postEvent(self, event);
}

static void MidiInProc(const MIDIPacketList* pktList, void* readProcRefCon,
                       void* srcConnRefCon)
{
    Q_UNUSED(readProcRefCon);

    MIDIDevice* self = static_cast<MIDIDevice*>(srcConnRefCon);
    Q_ASSERT(self != 0);

    for (quint32 i = 0; i < pktList->numPackets; i++)
    {
        MIDIPacket packet = pktList->packet[i];

        // MIDI commands 0xF0 - 0xFF don't contain channel information
        if (packet.data[0] & 0xF0 != 0xF0)
        {
            /* Check that the data came to the correct MIDI channel */
            // @todo Make OMNI available
            if (MIDI_CH(packet.data[0]) != (Byte) self->midiChannel())
                continue;
        }

        for (quint32 j = 0; j < packet.length; j++)
        {
            switch(MIDI_CMD(packet.data[j]))
            {
                case MIDI_NOTE_OFF:
                    if (packet.length < (j + 2))
                        break;

                    postEvent(self, CHANNEL_OFFSET_NOTE, packet.data[j + 1], 0);
                    j += 2;
                    break;

                case MIDI_NOTE_ON:
                    if (packet.length < (j + 2))
                        break;
                    // Can't use (++j, ++j) because execution order is not
                    // defined for all compilers.
                    postEvent(self, CHANNEL_OFFSET_NOTE, packet.data[j + 1],
                              packet.data[j + 2]);
                    j += 2;
                    break;

                case MIDI_NOTE_AFTERTOUCH:
                    if (packet.length < (j + 2))
                        break;
                    // Can't use (++j, ++j) because execution order is not
                    // defined for all compilers.
                    postEvent(self, CHANNEL_OFFSET_NOTE_AFTERTOUCH,
                              packet.data[j + 1], packet.data[j + 2]);
                    j += 2;
                    break;

                case MIDI_CONTROL_CHANGE:
                    if (packet.length < (j + 2))
                        break;
                    // Can't use (++j, ++j) because execution order is not
                    // defined for all compilers.
                    postEvent(self, CHANNEL_OFFSET_CONTROL_CHANGE,
                              packet.data[j + 1], packet.data[j + 2]);
                    j += 2;
                    break;

                case MIDI_PROGRAM_CHANGE:
                    if (packet.length < (j + 1))
                        break;
                    postEvent(self, CHANNEL_OFFSET_PROGRAM_CHANGE, 0,
                              packet.data[++j]);
                    break;

                case MIDI_CHANNEL_AFTERTOUCH:
                    if (packet.length < (j + 1))
                        break;
                    postEvent(self, CHANNEL_OFFSET_CHANNEL_AFTERTOUCH, 0,
                              packet.data[++j]);
                    break;

                case MIDI_PITCH_WHEEL:
                    if (packet.length < (j + 1))
                        break;
                    j++; // Skip LSB
                    postEvent(self, CHANNEL_OFFSET_PITCH_WHEEL, 0,
                              packet.data[++j]);
                    break;

                case MIDI_SYSEX:
                    // Skip all sysex data
                    for (; j < packet.length; j++)
                    {
                        if (packet.data[j] == MIDI_SYSEX_EOX)
                            break;
                    }
                    break;

                case MIDI_TIME_CODE:
                    // Skip time code
                    j += 1;
                    break;

                case MIDI_SONG_POSITION:
                case MIDI_SONG_SELECT:
                    // Skip song position / selection
                    j += 2;
                    break;

                default:
                    qDebug() << "Ignoring MIDI message"
                             << MIDI_CMD(packet.data[j]);
            }
        }
    }
}

bool MIDIDevice::open()
{
    MIDIInput* plugin;
    OSStatus s;

    plugin = qobject_cast<MIDIInput*> (parent());
    Q_ASSERT(plugin != 0);

    /* Don't open twice */
    if (m_inPort != 0)
        return true;

    /* Make an input port */
    s = MIDIInputPortCreate(plugin->client(), CFSTR("QLC Input Port"),
                            MidiInProc, 0, &m_inPort);
    if (s != 0)
    {
        qWarning() << "Unable to make an input port for" << name()
        << ":" << s;
        m_inPort = 0;
        m_isOK = false;
        return false;
    }

    /* Connect the input port to the first source */
    m_source = MIDIEntityGetSource(m_entity, 0);
    s = MIDIPortConnectSource(m_inPort, m_source, this);
    if (s != 0)
    {
        qWarning() << "Unable to connect input port to source for"
        << name() << ":" << s;

        s = MIDIPortDispose(m_inPort);
        if (s != 0)
            qWarning() << "Unable to dispose of port for" << name();

        m_inPort = 0;
        m_source = 0;
        m_isOK = false;

        return false;
    }

    /* If the entity has destinations, use one of them for feedback */
    if (MIDIEntityGetNumberOfDestinations(m_entity) > 0 && m_outPort == 0)
    {
        /* Make an output port */
        s = MIDIOutputPortCreate(plugin->client(),
                                 CFSTR("QLC Input FB Port"),
                                 &m_outPort);
        if (s != 0)
        {
            qWarning() << "Unable to make an output port for"
            << name() << ":" << s;
            m_outPort = 0;
            m_destination = 0;
        }
        else
        {
            /* Use the first destination */
            m_destination = MIDIEntityGetDestination(m_entity, 0);
        }
    }

    m_isOK = true;
    return true;
}

void MIDIDevice::close()
{
    OSStatus s;

    if (m_inPort != 0 && m_source != 0)
    {
        s = MIDIPortDisconnectSource(m_inPort, m_source);
        if (s != 0)
        {
            qWarning() << "Unable to disconnect input port for"
            << name();
        }

        s = MIDIPortDispose(m_inPort);
        if (s != 0)
        {
            qWarning() << "Unable to dispose of input port for"
            << name();
        }
        else
        {
            m_inPort = 0;
            m_source = 0;
        }
    }

    if (m_outPort != 0 && m_destination != 0)
    {
        s = MIDIPortDispose(m_outPort);
        if (s != 0)
        {
            qWarning() << "Unable to dispose of output port for"
            << name();
        }
        else
        {
            m_outPort = 0;
            m_destination = 0;
        }
    }
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText() const
{
    QString info;

    if (m_isOK == true)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += QString("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P>");
        info += QString("<B>MIDI Channel: </B>%1<BR>")
                .arg(midiChannel() + 1);
        info += QString("<B>Mode: </B>%1").arg(modeToString(mode()));
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>Unknown device</B>");
        info += QString("<P>");
        info += QString("MIDI interface is not available.");
        info += QString("</P>");
    }

    return info;
}

/*****************************************************************************
 * Operational mode
 *****************************************************************************/

QString MIDIDevice::modeToString(Mode mode)
{
    switch (mode)
    {
    default:
    case ControlChange:
        return QString("Control Change");
        break;
    case Note:
        return QString("Note Velocity");
        break;
    }
}

MIDIDevice::Mode MIDIDevice::stringToMode(const QString& mode)
{
    if (mode == QString("Note Velocity"))
        return Note;
    else
        return ControlChange;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIDevice::customEvent(QEvent* event)
{
    if (event->type() == MIDIInputEvent::eventType)
    {
        MIDIInputEvent* e = static_cast<MIDIInputEvent*> (event);
        emit valueChanged(e->m_channel, e->m_value);
        event->accept();
    }
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void MIDIDevice::feedBack(quint32 channel, uchar value)
{
    Byte cmd[3];
    quint32 valueByte = 0;

    /* If there's no output port or a destination, the endpoint probably
       doesn't have a MIDI IN port -> no feedback. */
    if (m_outPort == 0 || m_destination == 0)
        return;

    if (/*channel >= CHANNEL_OFFSET_NOTE &&*/ channel <= CHANNEL_OFFSET_NOTE_MAX)
    {
        if (value == 0)
            cmd[0] = MIDI_NOTE_OFF;
        else
            cmd[0] = MIDI_NOTE_ON;
        cmd[1] = static_cast <Byte> (channel);
        valueByte = 2;
    }
    else if (channel >= CHANNEL_OFFSET_CONTROL_CHANGE &&
             channel <= CHANNEL_OFFSET_CONTROL_CHANGE_MAX)
    {
        cmd[0] = MIDI_CONTROL_CHANGE;
        cmd[1] = static_cast <Byte> (channel - CHANNEL_OFFSET_CONTROL_CHANGE);
        valueByte = 2;
    }
    else if (channel >= CHANNEL_OFFSET_NOTE_AFTERTOUCH &&
             channel <= CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX)
    {
        cmd[0] = MIDI_NOTE_AFTERTOUCH;
        cmd[1] = static_cast <Byte> (channel - CHANNEL_OFFSET_NOTE_AFTERTOUCH);
        valueByte = 2;
    }
    else if (channel == CHANNEL_OFFSET_CHANNEL_AFTERTOUCH)
    {
        cmd[0] = MIDI_CHANNEL_AFTERTOUCH;
        valueByte = 1;
    }
    else if (channel == CHANNEL_OFFSET_PROGRAM_CHANGE)
    {
        cmd[0] = MIDI_PROGRAM_CHANGE;
        valueByte = 1;
    }
    else if (channel == CHANNEL_OFFSET_PITCH_WHEEL)
    {
        cmd[0] = MIDI_PITCH_WHEEL;
        valueByte = 1;
    }

    /* Set MIDI channel */
    cmd[0] |= (Byte) midiChannel();

    /* Set input channel (note or cc number) and value */
    cmd[valueByte] = static_cast <Byte> (SCALE(double(value),
                                               double(0), double(UCHAR_MAX),
                                               double(0), double(127)));

    /* Construct a MIDI packet list containing one packet */
    Byte buffer[32]; // Should be enough
    MIDIPacketList* list = (MIDIPacketList*) buffer;
    MIDIPacket* packet = MIDIPacketListInit(list);
    packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0,
                               sizeof(cmd), cmd);
    if (packet == 0)
    {
        qWarning() << "MIDI buffer overflow";
    }
    else
    {
        /* Send the MIDI packet */
        OSStatus s = MIDISend(m_outPort, m_destination, list);
        if (s != 0)
            qWarning() << "Unable to send feedback to" << name();
    }
}
