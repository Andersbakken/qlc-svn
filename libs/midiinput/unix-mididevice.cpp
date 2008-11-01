/*
  Q Light Controller
  unix-mididevice.cpp

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
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "unix-mididevice.h"
#include "unix-midiinput.h"
#include "midiinputevent.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, t_input input,
		       const snd_seq_addr_t* address) : QObject(parent)
{
	Q_ASSERT(address != NULL);
	m_address = NULL;

	setInput(input);
	setAddress(address);
	extractName();
}

MIDIDevice::~MIDIDevice()
{
	delete m_address;
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
	MIDIInput* plugin;
	int r;
	
	plugin = static_cast<MIDIInput*> (parent());
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
 * Input data
 *****************************************************************************/

void MIDIDevice::feedBack(t_input_channel /*channel*/, t_input_value /*value*/)
{
}

