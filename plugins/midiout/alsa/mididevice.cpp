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

#include <QSettings>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include <alsa/asoundlib.h>

#include "midiprotocol.h"
#include "mididevice.h"
#include "midiout.h"

MIDIDevice::MIDIDevice(MIDIOut* parent, const snd_seq_addr_t* address)
        : QObject(parent)
{
    Q_ASSERT(address != NULL);
    m_address = NULL;
    m_mode = ControlChange;
    m_midiChannel = 0;

    std::fill(m_values, m_values + MAX_MIDI_DMX_CHANNELS, 0);

    setAddress(address);
    extractName();
    loadSettings();
}

MIDIDevice::~MIDIDevice()
{
    saveSettings();
    delete m_address;
    m_address = NULL;
}

void MIDIDevice::loadSettings()
{
    QSettings settings;
    QVariant value;
    QString key;

    /* Attempt to get a MIDI channel from settings */
    key = QString("/midiout/%1/midichannel").arg(m_name);
    value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);

    /* Attempt to get the mode from settings */
    key = QString("/midiout/%1/mode").arg(m_name);
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
    key = QString("/midiout/%1/midichannel").arg(m_name);
    settings.setValue(key, m_midiChannel);

    /* Store mode to settings */
    key = QString("/midiout/%1/mode").arg(m_name);
    settings.setValue(key, MIDIDevice::modeToString(m_mode));
}

/*****************************************************************************
 * ALSA address
 *****************************************************************************/

const snd_seq_addr_t* MIDIDevice::address() const
{
    return m_address;
}

void MIDIDevice::setAddress(const snd_seq_addr_t* address)
{
    Q_ASSERT(address != NULL);

    if (m_address != NULL)
        delete m_address;

    m_address = new snd_seq_addr_t;
    m_address->client = address->client;
    m_address->port = address->port;
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
    MIDIOut* plugin;
    QString info;

    plugin = static_cast<MIDIOut*> (parent());
    Q_ASSERT(plugin != NULL);

    if (plugin->alsa() != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P><B>");
        info += tr("MIDI Channel: %1").arg(m_midiChannel + 1);
        info += QString("</B><BR><B>");
        info += tr("Mode: %1").arg(modeToString(m_mode));
        info += QString("</B>");
        info += QString("</P>");
    }
    else
    {
        info += QString("<P><B>");
        info += tr("Unknown device");
        info += QString("</B>");
        info += tr("ALSA sequencer interface is not available.");
        info += QString("</P>");
    }

    return info;
}

QString MIDIDevice::name() const
{
    return m_name;
}

void MIDIDevice::extractName()
{
    snd_seq_port_info_t* portInfo = NULL;
    MIDIOut* plugin;
    int r;

    plugin = static_cast<MIDIOut*> (parent());
    Q_ASSERT(plugin != NULL);
    Q_ASSERT(plugin->alsa() != NULL);
    Q_ASSERT(m_address != NULL);

    snd_seq_port_info_alloca(&portInfo);
    r = snd_seq_get_any_port_info(plugin->alsa(), m_address->client,
                                  m_address->port, portInfo);
    if (r == 0)
        m_name = QString(snd_seq_port_info_get_name(portInfo));
    else
        m_name = tr("Unknown");
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

/****************************************************************************
 * Write
 ****************************************************************************/

void MIDIDevice::outputDMX(const QByteArray& universe)
{
    MIDIOut* plugin = static_cast<MIDIOut*> (parent());
    Q_ASSERT(plugin != NULL);
    Q_ASSERT(plugin->alsa() != NULL);
    Q_ASSERT(m_address != NULL);

    /* Setup a common event structure for all values */
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    /* Since MIDI devices can have only 128 real channels, we don't
       attempt to write more than that */

    for (unsigned char channel = 0; channel < MAX_MIDI_DMX_CHANNELS;
            channel++)
    {
        /* Scale 0-255 to 0-127 */
        char scaled = DMX2MIDI(universe[channel]);

        /* Since MIDI is so slow, we only send values that are
           	   actually changed. */
        if (m_values[channel] == scaled)
            continue;

        /* Store the changed MIDI value */
        m_values[channel] = scaled;

        if (mode() == Note)
        {
            if (scaled == 0)
            {
                /* 0 is sent as a note off command */
                snd_seq_ev_set_noteoff(&ev, midiChannel(),
                                       channel, scaled);
            }
            else
            {
                /* 1-127 is sent as note on command */
                snd_seq_ev_set_noteon(&ev, midiChannel(),
                                      channel, scaled);
            }

            snd_seq_event_output(plugin->alsa(), &ev);
        }
        else
        {
            /* Control change */
            snd_seq_ev_set_controller(&ev, midiChannel(),
                                      channel, scaled);
            snd_seq_event_output_buffer(plugin->alsa(), &ev);
        }
    }

    /* Make sure that all values go to the MIDI endpoint */
    snd_seq_drain_output(plugin->alsa());
}
