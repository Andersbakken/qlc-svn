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

MIDIDevice::MIDIDevice(MIDIInput* parent, MIDIEndpointRef source)
	: QObject(parent),
	m_source(source),
	m_port(0)
{
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
	s = MIDIObjectGetIntegerProperty(m_source, kMIDIPropertyUniqueID, &uid);
	if (s == 0)
	{
		m_uid = uid;
		return true;
	}
	else
	{
		m_uid = -1;
		qWarning() << "Unable to get UID for MIDI source:" << s;
		return false;
	}
}

bool MIDIDevice::extractName()
{
	CFStringRef str;
	OSStatus s;

	/* Get the name property */
	s = MIDIObjectGetStringProperty(m_source, kMIDIPropertyDisplayName, &str);
	if (s != 0)
	{
		qWarning() << "Unable to get name for MIDI source:" << s;
		return false;
	}

	/* Convert the name into a QString. This call can fail if the buffer
	   is too small, so let's not make the device unusable just because we
	   allocated too little space for it. */
	CFIndex size = CFStringGetLength(str) + 1;
	char* buf = (char*) malloc(size);
	if (CFStringGetCString(str, buf, size, kCFStringEncodingISOLatin1))
		m_name = QString(buf);
	else
		m_name = QString("Unknown");
	free(buf);

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
		   e.g. cc15 == note15, cc1 == note1 etc... Annoying. */
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
	if (m_port != NULL)
		return true;

	/* Make a port */
	s = MIDIInputPortCreate(plugin->client(), CFSTR("QLC Input Port"),
				MidiInProc, NULL, &m_port);
	if (s != 0)
	{
		qWarning() << "Unable to make a port for" << name()
			   << ":" << s;
		m_port = NULL;
		return false;
	}

	/* Connect the port to this device's source */
	s = MIDIPortConnectSource(m_port, m_source, this);
	if (s != 0)
	{
		qWarning() << "Unable to connect port to source for"
			   << name() << ":" << s;
		s = MIDIPortDispose(m_port);
		if (s != 0)
		{
			qWarning() << "Unable to dispose of port for"
				   << name();
		}
		else
		{
			m_port = NULL;
		}

		return false;
	}

	return true;
}

void MIDIDevice::close()
{
	if (m_port != NULL)
	{
		OSStatus s;
		s = MIDIPortDisconnectSource(m_port, m_source);
		if (s != 0)
		{
			qWarning() << "Unable to disconnect port for"
				   << name();
		}

		s = MIDIPortDispose(m_port);
		if (s != 0)
		{
			qWarning() << "Unable to dispose of port for"
				   << name();
		}
		else
		{
			m_port = NULL;
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
	Q_UNUSED(channel);
	Q_UNUSED(value);
}
