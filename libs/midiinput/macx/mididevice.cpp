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
	m_source(NULL),
	m_destination(NULL),
	m_inPort(NULL),
	m_outPort(NULL)
{
	Q_ASSERT(MIDIGetNumberOfSources(entity) > 0);
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

/*****************************************************************************
 * File operations
 *****************************************************************************/

static void MidiInProc(const MIDIPacketList* pktList, void* readProcRefCon,
			void* srcConnRefCon)
{
	t_input_channel channel = 0;
	t_input_value value = 0;
	MIDIDevice* self;

	Q_UNUSED(readProcRefCon);

	self = static_cast<MIDIDevice*>(srcConnRefCon);
	Q_ASSERT(self != NULL);

	for (UInt32 i = 0; i < pktList->numPackets; i++)
	{
		MIDIPacket packet;
		MIDIInputEvent* event;

		packet = pktList->packet[i];

		/* WTF are we gonna do with a packet without data? */
		if (packet.length < 1)
			continue;

		/* TODO: This makes no differentiation on note & CC data:
		   e.g. cc15 == note15, cc1 == note1 etc... Annoying.
		   Then again, there's 127 channels already. Enough? */
		if (packet.data[0] ==MIDI_NOTE_ON ||
		    packet.data[0] == MIDI_NOTE_OFF ||
		    packet.data[0] == MIDI_CONTROL_CHANGE)
		{
			channel = static_cast<t_input_channel> (packet.data[1]);
			value = t_input_value(SCALE(double(packet.data[2]),
						    double(0),
						    double(127),
						    double(0),
						    double(KInputValueMax)));
			event = new MIDIInputEvent(self, channel, value);
			QApplication::postEvent(self, event);
		}
	}
}

bool MIDIDevice::open()
{
	MIDIInput* plugin;
	OSStatus s;

	plugin = qobject_cast<MIDIInput*> (parent());
	Q_ASSERT(plugin != NULL);

	/* Don't open twice */
	if (m_inPort != NULL)
		return true;

	/* Make an input port */
	s = MIDIInputPortCreate(plugin->client(), CFSTR("QLC Input Port"),
				MidiInProc, NULL, &m_inPort);
	if (s != 0)
	{
		qWarning() << "Unable to make an input port for" << name()
			   << ":" << s;
		m_inPort = NULL;
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

		m_inPort = NULL;
		m_source = NULL;

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
			m_outPort = NULL;
			m_destination = NULL;
		}
		else
		{
			/* Use the first destination */
			m_destination = MIDIEntityGetDestination(m_entity, 0);
		}
	}

	return true;
}

void MIDIDevice::close()
{
	OSStatus s;

	if (m_inPort != NULL && m_source != NULL)
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
			m_inPort = NULL;
			m_source = NULL;
		}
	}

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

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIDevice::customEvent(QEvent* event)
{
	if (event->type() == MIDIInputEvent::eventType)
	{
		MIDIInputEvent* e = static_cast<MIDIInputEvent*> (event);
		emit valueChanged(this, e->m_channel, e->m_value);
		event->accept();
	}
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void MIDIDevice::feedBack(t_input_channel channel, t_input_value value)
{
	Byte note[3];
	Byte cc[3];

	/* If there's no output port or a destination, the endpoint probably
	   doesn't have a MIDI IN port -> no feedback. */
	if (m_outPort == NULL || m_destination == NULL)
		return;

	/* MIDI doesn't support more than 127 distinct notes/cc's */
	if (channel > 127)
		return;

	/* Send the value as NOTE as well as CC data */
	if (value == 0)
		note[0] = MIDI_NOTE_OFF;
	else
		note[0] = MIDI_NOTE_ON;
	cc[0] = MIDI_CONTROL_CHANGE;
	cc[1] = note[1] = static_cast<char> (channel);
	cc[2] = note[2] = static_cast<char> (SCALE(double(value),
						   double(0),
						   double(KInputValueMax),
						   double(0),
						   double(127)));

	/* Construct a MIDI packet list (in a very peculiar way, yes..) */
	Byte buffer[32]; // Should be enough
	MIDIPacketList* list = (MIDIPacketList*) buffer;
	MIDIPacket* packet = MIDIPacketListInit(list);
	packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0,
				   sizeof(note), note);
	Q_ASSERT(packet != NULL);
	packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0,
				   sizeof(cc), cc);
	Q_ASSERT(packet != NULL);

	/* Send the very peculiar MIDI packet */
	OSStatus s = MIDISend(m_outPort, m_destination, list);
	if (s != 0)
		qWarning() << "Unable to send feedback data to" << name();
}
