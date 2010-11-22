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
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "midiprotocol.h"
#include "mididevice.h"
#include "midiout.h"

MIDIDevice::MIDIDevice(MIDIOut* parent, MIDIEntityRef entity)
        : QObject(parent),
        m_entity(entity),
        m_destination(0),
        m_outPort(0),
        m_uid(0),
        m_mode(ControlChange),
        m_midiChannel(1)
{
    std::fill(m_values, m_values + MAX_MIDI_DMX_CHANNELS, 0);
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
        qWarning() << "Unable to get manufacturer for MIDI entity:" << s;
        m_name = tr("Unknown %1").arg(m_uid);
    }
    else
    {
        /* Convert the name into a QString. */
        CFIndex size = CFStringGetLength(str) + 1;
        char* buf = (char*) malloc(size);
        if (CFStringGetCString(str, buf, size, kCFStringEncodingISOLatin1))
            m_name = QString(buf);
        else
            m_name = tr("Unknown %1").arg(m_uid);

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
    key = QString("/midiout/%1/midichannel").arg(m_uid);
    value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);

    /* Attempt to get the mode from settings */
    key = QString("/midiout/%1/mode").arg(m_uid);
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
    key = QString("/midiout/%1/midichannel").arg(m_uid);
    settings.setValue(key, m_midiChannel);

    /* Store mode to settings */
    key = QString("/midiout/%1/mode").arg(m_uid);
    settings.setValue(key, MIDIDevice::modeToString(m_mode));
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText() const
{
    MIDIOut* plugin;
    QString info;

    plugin = static_cast<MIDIOut*> (parent());
    Q_ASSERT(plugin != 0);

    if (plugin->client() != 0)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P><B>");
        info += tr("MIDI Channel: %1").arg(m_midiChannel + 1);
        info += QString("</B><BR><B>");
        info += tr("Mode: %1").arg(modeToString(m_mode));
        info += QString("</B></P>");
    }
    else
    {
        info += QString("<B>");
        info += tr("Unknown device");
        info += QString("</B>");
        info += QString("<P>");
        info += tr("MIDI interface is not available.");
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
 * File operations
 *****************************************************************************/
bool MIDIDevice::open()
{
    MIDIOut* plugin;
    OSStatus s;

    plugin = qobject_cast<MIDIOut*> (parent());
    Q_ASSERT(plugin != 0);

    /* Don't open twice */
    if (m_outPort != 0)
        return true;

    /* Use the first destination */
    if (MIDIEntityGetNumberOfDestinations(m_entity) > 0)
    {
        /* Make an output port */
        s = MIDIOutputPortCreate(plugin->client(),
                                 CFSTR("QLC Output Port"),
                                 &m_outPort);
        if (s != 0)
        {
            qWarning() << "Unable to make an output port for" << name() << s;
            m_outPort = 0;
            m_destination = 0;
        }
        else
        {
            /* Use the first destination */
            m_destination = MIDIEntityGetDestination(m_entity, 0);
        }

        return true;
    }
    else
    {
        m_outPort = 0;
        m_destination = 0;
        qWarning() << "MIDI entity has no destinations";
        return false;
    }
}

void MIDIDevice::close()
{
    OSStatus s;

    if (m_outPort != 0 && m_destination != 0)
    {
        s = MIDIPortDispose(m_outPort);
        if (s != 0)
        {
            qWarning() << "Unable to dispose of output port for" << name();
        }
        else
        {
            m_outPort = 0;
            m_destination = 0;
        }
    }
}

/****************************************************************************
 * Write
 ****************************************************************************/

void MIDIDevice::outputDMX(const QByteArray& universe)
{
    /* If there's no output port or a destination, the endpoint probably
       doesn't have a MIDI OUT port. */
    if (m_outPort == 0 || m_destination == 0)
        return;

    Byte buffer[512]; // Should be enough for 128 channels
    MIDIPacketList* list = (MIDIPacketList*) buffer;
    MIDIPacket* packet = MIDIPacketListInit(list);

    /* Since MIDI devices can have only 128 real channels, we don't
       attempt to write more than that */
    for (Byte channel = 0; channel < MAX_MIDI_DMX_CHANNELS; channel++)
    {
        Byte cmd[3];

        cmd[1] = channel;
        cmd[2] = DMX2MIDI(universe[channel]);

        /* Since MIDI is so slow, we only send values that are
           actually changed. */
        if (m_values[channel] == cmd[2])
            continue;

        /* Store the changed MIDI value. */
        m_values[channel] = cmd[2];

        if (m_mode == Note)
        {
            if (cmd[2] == 0)
            {
                /* Zero is sent as a note off command */
                cmd[0] = MIDI_NOTE_OFF;
            }
            else
            {
                /* 1-127 is sent as note on command */
                cmd[0] = MIDI_NOTE_ON;
            }
        }
        else
        {
            /* Control change */
            cmd[0] = MIDI_CONTROL_CHANGE;
        }

        /* Encode MIDI channel to the command */
        cmd[0] |= (Byte) midiChannel();

        /* Add the MIDI command to the packet list */
        packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0,
                                   sizeof(cmd), cmd);
        if (packet == 0)
        {
            qWarning() << "MIDIOut buffer overflow";
            break;
        }
    }

    /* Send the MIDI packet list */
    OSStatus s = MIDISend(m_outPort, m_destination, list);
    if (s != 0)
        qWarning() << "Unable to send MIDI data to" << name();
}
