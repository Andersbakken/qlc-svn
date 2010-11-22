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

#include "midiinputevent.h"
#include "midiprotocol.h"
#include "mididevice.h"
#include "midiinput.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, const snd_seq_addr_t* address)
        : QObject(parent)
{
    m_address = NULL;
    m_midiChannel = 0;

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
    key = QString("/midiinput/%1/midichannel").arg(m_name);
    value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);
}

void MIDIDevice::saveSettings()
{
    QSettings settings;
    QString key;

    /* Store MIDI channel to settings */
    key = QString("/midiinput/%1/midichannel").arg(m_name);
    settings.setValue(key, m_midiChannel);
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
    if (m_address != NULL)
        delete m_address;
    m_address = NULL;

    if (address != NULL)
    {
        m_address = new snd_seq_addr_t;
        m_address->client = address->client;
        m_address->port = address->port;
    }
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
    MIDIInput* plugin = static_cast<MIDIInput*> (parent());
    Q_ASSERT(plugin != NULL);

    QString info;
    if (plugin->alsa() != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P>");
        info += QString("<B>%1:</B> ").arg(tr("MIDI Channel"));
        if (midiChannel() < 16)
            info += QString("%1<BR/>").arg(midiChannel() + 1);
        else
            info += QString("%1<BR/>").arg(tr("Any"));
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>%1</B>").arg(tr("Unknown device"));
        info += QString("<P>");
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
    MIDIInput* plugin = static_cast<MIDIInput*> (parent());
    if (plugin == NULL || plugin->alsa() == NULL || m_address == NULL)
        return;

    snd_seq_port_info_t* portInfo = NULL;
    snd_seq_port_info_alloca(&portInfo);
    int r = snd_seq_get_any_port_info(plugin->alsa(), m_address->client,
                                      m_address->port, portInfo);
    if (r == 0)
        m_name = QString(snd_seq_port_info_get_name(portInfo));
    else
        m_name = QString("ERROR");
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIDevice::feedBack(quint32 channel, uchar value)
{
    MIDIInput* plugin = static_cast<MIDIInput*> (parent());
    Q_ASSERT(plugin != NULL);
    Q_ASSERT(plugin->alsa() != NULL);
    Q_ASSERT(m_address != NULL);

    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;
    bool d2v = false;

    if (QLCMIDIProtocol::feedbackToMidi(channel, value, midiChannel(), &cmd,
                                        &data1, &data2, &d2v) == true)
    {
        /* Setup an event structure */
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);

        if (MIDI_CMD(cmd) == MIDI_NOTE_OFF)
        {
            /* Send data as note off command */
            snd_seq_ev_set_noteoff(&ev, midiChannel(), data1, data2);
            snd_seq_event_output(plugin->alsa(), &ev);
            snd_seq_drain_output(plugin->alsa());
        }
        else if (MIDI_CMD(cmd) == MIDI_NOTE_ON)
        {
            /* Send data as note on command */
            snd_seq_ev_set_noteon(&ev, midiChannel(), data1, data2);
            snd_seq_event_output(plugin->alsa(), &ev);
            snd_seq_drain_output(plugin->alsa());
        }
        else if (MIDI_CMD(cmd) == MIDI_CONTROL_CHANGE)
        {
            /* Send data as control change command */
            snd_seq_ev_set_controller(&ev, midiChannel(), data1, data2);
            snd_seq_event_output(plugin->alsa(), &ev);
            snd_seq_drain_output(plugin->alsa());
        }
    }
}
