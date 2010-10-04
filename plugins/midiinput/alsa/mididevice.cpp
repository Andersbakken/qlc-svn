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
#include "mididevice.h"
#include "midiinput.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, const snd_seq_addr_t* address)
        : QObject(parent)
{
    m_address = NULL;
    m_mode = ControlChange;
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
    MIDIInput* plugin;
    QString info;

    plugin = static_cast<MIDIInput*> (parent());
    Q_ASSERT(plugin != NULL);

    if (plugin->alsa() != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += QString("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P>");
        info += QString("<B>MIDI Channel: </B>%1<BR>")
                .arg(m_midiChannel + 1);
        info += QString("<B>Mode: </B>%1")
                .arg(modeToString(m_mode));
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>Unknown device</B>");
        info += QString("<P>");
        info += QString("ALSA sequencer interface is not available.");
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

void MIDIDevice::feedBack(quint32 channel, uchar value)
{
    /* MIDI devices can have only 128 notes or controllers */
    if (channel < 128)
    {
        snd_seq_event_t ev;
        MIDIInput* plugin;

        plugin = static_cast<MIDIInput*> (parent());
        Q_ASSERT(plugin != NULL);
        Q_ASSERT(plugin->alsa() != NULL);
        Q_ASSERT(m_address != NULL);

        /* Setup an event structure */
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);

        char scaled = static_cast <char> (SCALE(double(value),
                                                double(0),
                                                double(UCHAR_MAX),
                                                double(0),
                                                double(127)));

        if (m_mode == ControlChange)
        {
            /* Send control change */
            snd_seq_ev_set_controller(&ev, midiChannel(),
                                      channel, scaled);
            snd_seq_event_output(plugin->alsa(), &ev);
            snd_seq_drain_output(plugin->alsa());
        }
        else
        {
            /* Send note on/off */
            if (value == 0)
            {
                snd_seq_ev_set_noteoff(&ev, midiChannel(),
                                       channel, scaled);
            }
            else
            {
                snd_seq_ev_set_noteon(&ev, midiChannel(),
                                      channel, scaled);
            }

            snd_seq_event_output(plugin->alsa(), &ev);
            snd_seq_drain_output(plugin->alsa());
        }
    }
}
