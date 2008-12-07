/*
  Q Light Controller
  mididevice-unix.cpp

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

#include <alsa/asoundlib.h>

#include "mididevice-unix.h"
#include "midiout-unix.h"

#define MAX_MIDI_DMX_CHANNELS 128

MIDIDevice::MIDIDevice(MIDIOut* parent, t_output output,
		       const snd_seq_addr_t* address) : QObject(parent)
{
	QSettings settings;
	QVariant value;
	QString key;

	Q_ASSERT(address != NULL);
	m_address = NULL;

	setOutput(output);
	setAddress(address);
	extractName();

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

	/* Start with all values zeroed */
	memset(m_values, 0, sizeof(m_values));
}

MIDIDevice::~MIDIDevice()
{
	delete m_address;
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
	key = QString("/midiout/%1/midichannel").arg(device->name());
	settings.setValue(key, device->midiChannel());

	/* Store mode to settings */
	key = QString("/midiout/%1/mode").arg(device->name());
	settings.setValue(key, MIDIDevice::modeToString(device->mode()));
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

/****************************************************************************
 * Read & write
 ****************************************************************************/

void MIDIDevice::write(t_channel channel, t_value value)
{
	Q_ASSERT(channel < 512);
	m_values[channel] = value;

	/* Since MIDI devices can have only 128 real channels, we don't
	   actually attempt to output more than that, although a full 512
	   channel universe exists within each MIDIDevice object. */
	if (channel < MAX_MIDI_DMX_CHANNELS)
	{
		snd_seq_event_t ev;
		MIDIOut* plugin;
	
		plugin = static_cast<MIDIOut*> (parent());
		Q_ASSERT(plugin != NULL);
		Q_ASSERT(plugin->alsa() != NULL);
		Q_ASSERT(m_address != NULL);

		/* Setup an event structure */
		snd_seq_ev_clear(&ev);
		snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
		snd_seq_ev_set_subs(&ev);
		snd_seq_ev_set_direct(&ev);

		switch (m_mode)
		{
		default:
		case ControlChange:
			/* Use control change numbers as DMX channels and
			   control values as DMX channel values */
			snd_seq_ev_set_controller(&ev, m_midiChannel,
						  channel, value >> 1);
			break;

		case Note:
			/* Use note numbers as DMX channels and velocities as
			   DMX channel values. 0 is written as note off */
			if (value == 0)
				snd_seq_ev_set_noteoff(&ev, m_midiChannel,
						       channel, 0);
			else
				snd_seq_ev_set_noteon(&ev, m_midiChannel,
						      channel, value >> 1);
			break;
		}

		snd_seq_event_output(plugin->alsa(), &ev);
		snd_seq_drain_output(plugin->alsa());
	}
}

void MIDIDevice::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num < 512);
	memcpy(m_values + address, values, num);

	/* Since MIDI devices can have only 128 real channels, we don't
	   actually attempt to output more than that, although a full 512
	   channel universe exists within each MIDIDevice object. */
	if (address < MAX_MIDI_DMX_CHANNELS)
	{
		snd_seq_event_t ev;
		MIDIOut* plugin;
		int i;
	
		plugin = static_cast<MIDIOut*> (parent());
		Q_ASSERT(plugin != NULL);
		Q_ASSERT(plugin->alsa() != NULL);
		Q_ASSERT(m_address != NULL);

		/* Setup a common event structure for all values */
		snd_seq_ev_clear(&ev);
		snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
		snd_seq_ev_set_subs(&ev);
		snd_seq_ev_set_direct(&ev);
	
		switch (m_mode)
		{
		default:
		case ControlChange:
			/* Use control change numbers as DMX channels and
			   control values as DMX channel values */
			for (i = 0; i < num; i++)
			{
				snd_seq_ev_set_controller(&ev, m_midiChannel,
						address + i, values[i] >> 1);
				snd_seq_event_output(plugin->alsa(), &ev);
			}
			break;

		case Note:
			/* Use note numbers as DMX channels and velocities as
			   DMX channel values. 0 is written as note off */
			for (i = 0; i < num; i++)
			{
				if (values[i] == 0)
				{
					snd_seq_ev_set_noteoff(&ev,
						m_midiChannel, address + i, 0);
				}
				else
				{
					snd_seq_ev_set_noteon(&ev,
						m_midiChannel, address + i,
						values[i] >> 1);
				}
			}
			break;
		}

		snd_seq_drain_output(plugin->alsa());
	}
}

void MIDIDevice::read(t_channel channel, t_value* value)
{
	Q_ASSERT(channel < 512);
	*value = m_values[channel];
}

void MIDIDevice::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num < 512);
	memcpy(values, m_values + address, num);
}

