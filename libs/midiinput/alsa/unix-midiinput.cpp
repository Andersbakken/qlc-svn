/*
  Q Light Controller
  unix-midiinput.cpp

  Copyright (C) Heikki Junnila
		Stefan Krumm

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
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include "configuremidiinput.h"
#include "unix-mididevice.h"
#include "unix-midipoller.h"
#include "unix-midiinput.h"
#include "midiinputevent.h"

/*****************************************************************************
 * MIDIInput Initialization
 *****************************************************************************/

MIDIInput::~MIDIInput()
{
	/* Delete the poller. Removes the devices also from the hash table. */
	delete m_poller;

	/* Delete all MIDI devices. */
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();

	/* Close the ALSA sequencer interface */
	if (m_alsa != NULL)
		snd_seq_close(m_alsa);
	m_alsa = NULL;
}

void MIDIInput::init()
{
	m_alsa = NULL;
	m_address = NULL;

	/* Create the poller thread */
	m_poller = new MIDIPoller(this);

	/* Initialize ALSA stuff */
	initALSA();

	/* Find out what devices we have */
	rescanDevices();
}

void MIDIInput::open(t_input input)
{
	MIDIDevice* dev = device(input);
	if (dev != NULL)
		m_poller->addDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

void MIDIInput::close(t_input input)
{
	MIDIDevice* dev = device(input);
	if (dev != NULL)
		m_poller->removeDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

/*****************************************************************************
 * ALSA
 *****************************************************************************/

void MIDIInput::initALSA()
{
	snd_seq_client_info_t* client = NULL;

	/* Destroy the old handle */
	if (m_alsa != NULL)
		snd_seq_close(m_alsa);
	m_alsa = NULL;

	/* Destroy the plugin's own address */
	if (m_address != NULL)
		delete m_address;
	m_address = NULL;

	/* Open the sequencer interface */
	if (snd_seq_open(&m_alsa, "default", SND_SEQ_OPEN_DUPLEX, 0) != 0)
	{
		qWarning() << "Unable to open ALSA interface!";
		m_alsa = NULL;
		return;
	}

	/* Set current client information */
	snd_seq_client_info_alloca(&client);
	snd_seq_set_client_name(m_alsa, name().toAscii());
	snd_seq_get_client_info(m_alsa, client);

	/* Create an application-level port for receiving MIDI data from
	   actual system clients' ports */
	m_address = new snd_seq_addr_t;
	m_address->port = snd_seq_create_simple_port(m_alsa, "__QLC__input",
			SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ |
		   	SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	m_address->client = snd_seq_client_info_get_client(client);
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIInput::rescanDevices()
{
	snd_seq_client_info_t* clientInfo = NULL;
	snd_seq_port_info_t* portInfo = NULL;

	/* Don't do anything if the ALSA sequencer interface is not open */
	if (m_alsa == NULL)
		return;

	/* Copy all device pointers to a destroy list */
	QList <MIDIDevice*> destroyList(m_devices);

	/* Allocate these from stack */
	snd_seq_client_info_alloca(&clientInfo);
	snd_seq_port_info_alloca(&portInfo);

	/* Find out what kinds of clients and ports there are */
	snd_seq_client_info_set_client(clientInfo, 0); // TODO: -1 ?????
	while (snd_seq_query_next_client(m_alsa, clientInfo) == 0)
	{
		int client;

		/* Get the client ID */
		client = snd_seq_client_info_get_client(clientInfo);

		/* Ignore our own client */
		if (m_address->client == client)
			continue;

		/* Go thru all available ports in the client */
		snd_seq_port_info_set_client(portInfo, client);
		snd_seq_port_info_set_port(portInfo, -1);
		while (snd_seq_query_next_port(m_alsa, portInfo) == 0)
		{
			const snd_seq_addr_t* address;
			MIDIDevice* dev;

			address = snd_seq_port_info_get_addr(portInfo);
			dev = device(address);
			if (dev == NULL)
			{
				/* New address. Create a new device for it. */
				dev = new MIDIDevice(this);
				Q_ASSERT(dev != NULL);
				dev->setAddress(address);
				dev->extractName();
				if (dev->name().contains("__QLC__") == false)
					addDevice(dev);
				else
					delete dev;
			}
			else
			{
				/* This device is still alive. Do not destroy
				   it at the end of this function. */
				destroyList.removeAll(dev);
			}
		}
	}

	/* All devices that were not found during rescan are clearly no longer
	   in our presence and must be destroyed. */
	while (destroyList.isEmpty() == false)
		removeDevice(destroyList.takeFirst());
}

MIDIDevice* MIDIInput::device(const snd_seq_addr_t* address)
{
	QListIterator <MIDIDevice*> it(m_devices);

	while (it.hasNext() == true)
	{
		MIDIDevice* dev = it.next();

		Q_ASSERT(dev != NULL);
		Q_ASSERT(dev->address() != NULL);

		if (dev->address()->client == address->client &&
		    dev->address()->port == address->port)
		{
			return dev;
		}
	}

	return NULL;
}

MIDIDevice* MIDIInput::device(unsigned int index)
{
	if (index < static_cast<unsigned int>(m_devices.size()))
		return m_devices.at(index);
	else
		return NULL;
}

void MIDIInput::addDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);

	m_devices.append(device);

	emit deviceAdded(device);
}

void MIDIInput::removeDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);

	removePollDevice(device);
	m_devices.removeAll(device);

	emit deviceRemoved(device);
	delete device;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString MIDIInput::name()
{
	return QString("MIDI Input");
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

QStringList MIDIInput::inputs()
{
	QStringList list;

	QListIterator <MIDIDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next()->name();

	return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void MIDIInput::configure()
{
	ConfigureMIDIInput cmi(NULL, this);
	cmi.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString MIDIInput::infoText(t_input input)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (input == KInputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides input support for ");
		str += QString("various MIDI devices.");
		str += QString("</P>");
	}
	else
	{
		MIDIDevice* dev = device(input);
		if (dev != NULL)
		{
			str += device(input)->infoText();
		}
		else
		{
			str += QString("<P><I>");
			str += QString("Unable to find device. Please go to ");
			str += QString("the configuration dialog and click ");
			str += QString("the refresh button.");
			str += QString("</I></P>");
		}
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Device poller
 *****************************************************************************/

void MIDIInput::addPollDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);
	m_poller->addDevice(device);
}

void MIDIInput::removePollDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);
	m_poller->removeDevice(device);
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIInput::customEvent(QEvent* event)
{
	if (event->type() == MIDIInputEvent::eventType)
	{
		MIDIInputEvent* e = static_cast<MIDIInputEvent*> (event);
		int index;

		Q_ASSERT(event != NULL);
		index = m_devices.indexOf(e->m_device);
		if (index != -1)
		{
			emit valueChanged(this, t_input(index), e->m_channel,
					  e->m_value);
			event->accept();
		}
	}
}

void MIDIInput::connectInputData(QObject* listener)
{
	Q_ASSERT(listener != NULL);

	connect(this, SIGNAL(valueChanged(QLCInPlugin*,t_input,t_input_channel,
					  t_input_value)),
		listener,
		SLOT(slotValueChanged(QLCInPlugin*,t_input,t_input_channel,
				      t_input_value)));
}

void MIDIInput::feedBack(t_input input, t_input_channel channel,
			 t_input_value value)
{
	MIDIDevice* dev;

	dev = device(input);
	if (dev == NULL)
		return;
	else
		dev->feedBack(channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiinput, MIDIInput)
