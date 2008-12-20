/*
  Q Light Controller
  unix-midipoller.cpp

  Copyright (C) Heikki Junnila

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
#include <QEvent>
#include <QDebug>
#include <poll.h>

#include "unix-midipoller.h"
#include "unix-mididevice.h"
#include "unix-midiinput.h"
#include "midiinputevent.h"

#define KPollTimeout 1000

/****************************************************************************
 * Initialization
 ****************************************************************************/

MIDIPoller::MIDIPoller(MIDIInput* parent) : QThread(parent)
{
	Q_ASSERT(parent != NULL);
	m_running = false;
	m_changed = false;
}

MIDIPoller::~MIDIPoller()
{
	m_devices.clear();
	stop();
}

/****************************************************************************
 * Polled devices
 ****************************************************************************/

quint64 MIDIPoller::addressHash(const snd_seq_addr_t* address) const
{
	quint64 hash = 0;

	Q_ASSERT(address != NULL);

	/* Put the client number to the third byte */
	hash = static_cast<quint64> (address->client) << 32;

	/* ...and the port number to the first byte (LSB) */
	hash |= static_cast<quint64> (address->port);

	return hash;
}

bool MIDIPoller::addDevice(MIDIDevice* device)
{
	quint64 hash;

	Q_ASSERT(device != NULL);

	m_mutex.lock();

	/* Check, whether the hash table already contains the device */
	hash = addressHash(device->address());
	if (m_devices.contains(hash) == true)
	{
		m_mutex.unlock();
		return false;
	}

	/* Subscribe the device's events */
	subscribeDevice(device);

	/* Insert the device into the hash map for later retrieval */
	m_devices.insert(hash, device);
	m_changed = true;

	/* Start the poller thread in case it's not running */
	if (m_running == false && isRunning() == false)
	{
		m_running = true;
		start();
	}

	m_mutex.unlock();

	return true;
}

bool MIDIPoller::removeDevice(MIDIDevice* device)
{
	quint64 hash;

	Q_ASSERT(device != NULL);

	m_mutex.lock();

	hash = addressHash(device->address());
	if (m_devices.remove(hash) > 0)
	{
		unsubscribeDevice(device);
		m_changed = true;
	}

	if (m_devices.count() == 0)
	{
		m_mutex.unlock();
		stop();
	}
	else
	{
		m_mutex.unlock();
	}

	return true;
}

void MIDIPoller::subscribeDevice(MIDIDevice* device)
{
	snd_seq_port_subscribe_t* sub = NULL;
	MIDIInput* plugin;

	/* Get the parent plugin pointer */
	plugin = static_cast<MIDIInput*> (parent());
	Q_ASSERT(plugin != NULL);
	Q_ASSERT(plugin->alsa() != NULL);

	/* Subscribe events coming from the the device's MIDI port to get
	   patched to the plugin's own MIDI port */
	snd_seq_port_subscribe_alloca(&sub);
	snd_seq_port_subscribe_set_sender(sub, device->address());
	snd_seq_port_subscribe_set_dest(sub, plugin->address());
	snd_seq_subscribe_port(plugin->alsa(), sub);
}

void MIDIPoller::unsubscribeDevice(MIDIDevice* device)
{
	snd_seq_port_subscribe_t* sub = NULL;
	MIDIInput* plugin = NULL;

	Q_ASSERT(device != NULL);

	/* Get the parent plugin pointer */
	plugin = static_cast<MIDIInput*> (parent());
	Q_ASSERT(plugin != NULL);
	Q_ASSERT(plugin->alsa() != NULL);

	/* Unsubscribe events from the device */
	snd_seq_port_subscribe_alloca(&sub);
	snd_seq_port_subscribe_set_sender(sub, device->address());
	snd_seq_port_subscribe_set_dest(sub, plugin->address());
	snd_seq_unsubscribe_port(plugin->alsa(), sub);
}

/*****************************************************************************
 * Poller thread
 *****************************************************************************/

void MIDIPoller::stop()
{
	m_mutex.lock();
	m_running = false;
	m_mutex.unlock();

	wait();
}

void MIDIPoller::run()
{
	MIDIInput* plugin = NULL;
	struct pollfd *pfd = 0;
	int npfd = 0;

	/* Get the parent plugin pointer */
	plugin = static_cast<MIDIInput*> (parent());
	Q_ASSERT(plugin != NULL);

	m_mutex.lock();

	while (m_running == true)
	{
		Q_ASSERT(plugin->alsa() != NULL);

		if (m_changed == true)
		{
			/* Poll descriptors must be re-evaluated When the
			   polled devices hash contents change */
			npfd = snd_seq_poll_descriptors_count(plugin->alsa(),
							      POLLIN);
			pfd = (struct pollfd*)
					alloca(npfd * sizeof(struct pollfd));
			snd_seq_poll_descriptors(plugin->alsa(), pfd, npfd,
						 POLLIN);
			m_changed = false;
		}

		m_mutex.unlock();
		/* Poll for MIDI events from the polled descriptors */
		if (poll(pfd, npfd, KPollTimeout) > 0)
			readEvent(plugin->alsa());
		m_mutex.lock();
	}

	m_mutex.unlock();
}

void MIDIPoller::readEvent(snd_seq_t* alsa)
{
	m_mutex.lock();

	/* Wait for input data */
	do
	{
		snd_seq_event_t* ev = NULL;
		MIDIDevice* device;
		MIDIInputEvent* e;
		quint64 hash;
		int r;

		/* Receive an event */
		r = snd_seq_event_input(alsa, &ev);

		/* Find a device matching the event's address. If one isn't
		   found, skip this event, since we're not interested in it */
		hash = addressHash(&ev->source);
		if (m_devices.contains(hash) == true)
			device = m_devices[hash];
		else
			continue;

		/* Parse the MIDI event into an internal MIDIInputEvent */
		if (ev->type == SND_SEQ_EVENT_CONTROLLER ||
		    ev->type == SND_SEQ_EVENT_NONREGPARAM ||
		    ev->type == SND_SEQ_EVENT_REGPARAM ||
		    ev->type == SND_SEQ_EVENT_KEYPRESS ||
		    ev->type == SND_SEQ_EVENT_CHANPRESS)
		{
			/* "<< 1" == "times two", but 255 is never reached */
			e = new MIDIInputEvent(device, device->input(),
					       ev->data.control.param,
					       ev->data.control.value << 1);
			QApplication::postEvent(parent(), e);
		}
		else if (ev->type == SND_SEQ_EVENT_NOTEON)
		{
			/* "<< 1" == "times two", but 255 is never reached */
			e = new MIDIInputEvent(device, device->input(),
					       ev->data.note.note,
					       ev->data.note.velocity << 1);
			QApplication::postEvent(parent(), e);
		}
		else if (ev->type == SND_SEQ_EVENT_NOTEOFF)
		{
			/* "<< 1" == "times two", but 255 is never reached */
			e = new MIDIInputEvent(device, device->input(),
					       ev->data.note.note,
					       ev->data.note.velocity << 1);
			QApplication::postEvent(parent(), e);
		}
	}
	while (snd_seq_event_input_pending(alsa, 0) > 0);

	m_mutex.unlock();
}
