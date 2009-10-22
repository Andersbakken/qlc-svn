/*
  Q Light Controller
  midiout.cpp

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
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include <alsa/asoundlib.h>

#include "configuremidiout.h"
#include "mididevice.h"
#include "midiout.h"

/*****************************************************************************
 * MIDIOut Initialization
 *****************************************************************************/

MIDIOut::~MIDIOut()
{
	/* Delete all MIDI devices. */
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();

	/* Close the ALSA sequencer interface */
	snd_seq_close(m_alsa);
	m_alsa = NULL;
}

void MIDIOut::init()
{
	m_alsa = NULL;
	m_address = NULL;

	/* Initialize ALSA stuff */
	initALSA();

	/* Find out what devices we have */
	rescanDevices();
}

void MIDIOut::open(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		subscribeDevice(dev);
	else
		qDebug() << name() << "has no output number:" << output;
}

void MIDIOut::close(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		unsubscribeDevice(dev);
	else
		qDebug() << name() << "has no output number:" << output;
}

void MIDIOut::subscribeDevice(MIDIDevice* device)
{
	snd_seq_port_subscribe_t* sub = NULL;

	Q_ASSERT(device != NULL);
	Q_ASSERT(m_address != NULL);

	snd_seq_port_subscribe_alloca(&sub);
	snd_seq_port_subscribe_set_sender(sub, m_address);
	snd_seq_port_subscribe_set_dest(sub, device->address());
	snd_seq_subscribe_port(m_alsa, sub);
}

void MIDIOut::unsubscribeDevice(MIDIDevice* device)
{
	snd_seq_port_subscribe_t* sub = NULL;

	Q_ASSERT(device != NULL);
	Q_ASSERT(m_address != NULL);

	snd_seq_port_subscribe_alloca(&sub);
	snd_seq_port_subscribe_set_sender(sub, m_address);
	snd_seq_port_subscribe_set_dest(sub, device->address());
	snd_seq_unsubscribe_port(m_alsa, sub);
}
/*****************************************************************************
 * ALSA
 *****************************************************************************/

void MIDIOut::initALSA()
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

	/* Create an application-level port */
	m_address = new snd_seq_addr_t;
	m_address->port = snd_seq_create_simple_port(m_alsa, "__QLC__output",
		   	SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	m_address->client = snd_seq_client_info_get_client(client);
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIOut::rescanDevices()
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
			if (address == NULL)
				continue;

			dev = device(address);
			if (dev == NULL)
			{
				/* New address. Create a new device for it. */
				dev = new MIDIDevice(this, address);
				Q_ASSERT(dev != NULL);

				/* Don't show QLC's internal ALSA ports */
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

MIDIDevice* MIDIOut::device(const snd_seq_addr_t* address)
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

MIDIDevice* MIDIOut::device(unsigned int index)
{
	if (index < static_cast<unsigned int> (m_devices.size()))
		return m_devices.at(index);
	else
		return NULL;
}

void MIDIOut::addDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);

	m_devices.append(device);
	emit deviceAdded(device);
}

void MIDIOut::removeDevice(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);

	m_devices.removeAll(device);
	emit deviceRemoved(device);
	delete device;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString MIDIOut::name()
{
	return QString("MIDI Output");
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

QStringList MIDIOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <MIDIDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->name());

	return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void MIDIOut::configure()
{
	ConfigureMIDIOut cmo(NULL, this);
	cmo.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString MIDIOut::infoText(t_output output)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support ");
		str += QString("through various MIDI devices.");
		str += QString("</P>");
	}
	else
	{
		MIDIDevice* dev = device(output);
		if (dev != NULL)
		{
			str += device(output)->infoText();
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
 * Value read/write methods
 *****************************************************************************/

void MIDIOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void MIDIOut::writeRange(t_output output, t_channel address, t_value* values,
		t_channel num)
{
	Q_UNUSED(address);

	MIDIDevice* dev = device(output);
	if (dev != NULL)
		dev->writeRange(values, num);
}

void MIDIOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void MIDIOut::readRange(t_output output, t_channel address, t_value* values,
	       t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiout, MIDIOut)

