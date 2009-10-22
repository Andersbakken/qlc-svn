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
	m_destination(NULL),
	m_outPort(NULL),
	m_uid(0),
	m_mode(ControlChange),
	m_midiChannel(1)
{
	Q_ASSERT(MIDIGetNumberOfDestinations(entity) > 0);
	std::fill(m_values, m_values + MAX_MIDI_DMX_CHANNELS, 0);
}

MIDIDevice::~MIDIDevice()
{
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
        s = MIDIObjectGetStringProperty(m_entity, kMIDIPropertyName, &str);
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
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText() const
{
	MIDIOut* plugin;
	QString info;

	plugin = static_cast<MIDIOut*> (parent());
	Q_ASSERT(plugin != NULL);

	if (plugin->client() != NULL)
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
 * File operations
 *****************************************************************************/
bool MIDIDevice::open()
{
        MIDIOut* plugin;
        OSStatus s;

        plugin = qobject_cast<MIDIOut*> (parent());
        Q_ASSERT(plugin != NULL);

        /* Don't open twice */
        if (m_outPort != NULL)
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
                        qWarning() << "Unable to make an output port for"
                                   << name() << ":" << s;
                        m_outPort = NULL;
                        m_destination = NULL;
                }
                else
                {
                        /* Use the first destination */
                        m_destination = MIDIEntityGetDestination(m_entity, 0);

			/* Set MIDI channel number as a bitmask */
			MIDIObjectSetIntegerProperty(m_destination,
						kMIDIPropertyTransmitChannels,
						1 << midiChannel());
                }

	        return true;
        }
	else
	{
		m_outPort = NULL;
		m_destination = NULL;
		qWarning() << "MIDI entity has no destinations";
		return false;
	}
}

void MIDIDevice::close()
{
        OSStatus s;

	if (m_outPort != NULL && m_destination != NULL)
        {
                s = MIDIPortDispose(m_outPort);
                if (s != 0)
                {
                        qWarning() << "Unable to dispose of output port for"
                                   << name();
                }
                else
                {
                        m_outPort = NULL;
                        m_destination = NULL;
                }
        }
}

void MIDIDevice::setMidiChannel(t_channel channel)
{
	m_midiChannel = channel;

	/* If the destination is already open, change its property */
	if (m_destination != NULL)
	{
		/* Set MIDI channel number as a bitmask */
		MIDIObjectSetIntegerProperty(m_destination,
					     kMIDIPropertyTransmitChannels,
					     1 << midiChannel());
	}
}

/****************************************************************************
 * Write
 ****************************************************************************/

void MIDIDevice::writeRange(t_value* values, t_channel num)
{
	Q_ASSERT(num == 512);
	Q_UNUSED(num);

        /* If there's no output port or a destination, the endpoint probably
           doesn't have a MIDI IN port -> no feedback. */
        if (m_outPort == NULL || m_destination == NULL)
                return;

	Byte buffer[512]; // Should be enough for 128 channels
	MIDIPacketList* list = (MIDIPacketList*) buffer;
	MIDIPacket* packet = MIDIPacketListInit(list);

	/* Since MIDI devices can have only 128 real channels, we don't
	   attempt to write more than that */

	switch (m_mode)
	{
	default:
	case ControlChange:
		/* Use control change numbers as DMX channels and
		   control values as DMX channel values */
		Byte cc[3];
		for (int i = 0; i < MAX_MIDI_DMX_CHANNELS; i++)
		{
			if (m_values[i] == values[i])
				continue;

			m_values[i] = values[i];

			cc[0] = MIDI_CONTROL_CHANGE;
		        cc[1] = static_cast<char> (i);
			cc[2] = static_cast<char> (SCALE(double(values[i]),
							 double(0),
							 double(KInputValueMax),
							 double(0),
							 double(127)));

			packet = MIDIPacketListAdd(list, sizeof(buffer),
						   packet, 0, sizeof(cc), cc);
			if (packet == NULL)
			{
				qWarning() << "MIDIOut buffer overflow";
				break;
			}
		}
		break;

	case Note:
		/* Use note numbers as DMX channels and velocities as
		   DMX channel values. Value 0 is written as note off. */
		Byte note[3];
		for (int i = 0; i < MAX_MIDI_DMX_CHANNELS; i++)
		{
			if (m_values[i] == values[i])
				continue;

			m_values[i] = values[i];

			if (values[i] == 0)
				note[0] = MIDI_NOTE_OFF;
			else
				note[0] = MIDI_NOTE_ON;
		        note[1] = static_cast<char> (i);
			note[2] = static_cast<char> (SCALE(double(values[i]),
							 double(0),
							 double(KInputValueMax),
							 double(0),
							 double(127)));

			packet = MIDIPacketListAdd(list, sizeof(buffer),
						   packet, 0, sizeof(note),
						   note);
			if (packet == NULL)
			{
				qWarning() << "MIDIOut buffer overflow";
				break;
			}
		}
		break;
	}

	/* Send the very peculiar MIDI packet */
	OSStatus s = MIDISend(m_outPort, m_destination, list);
	if (s != 0)
		qWarning() << "Unable to send MIDI data to" << name();
}
