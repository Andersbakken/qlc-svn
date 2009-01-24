/*
  Q Light Controller
  midiout-win32.cpp

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
#include <QStringList>
#include <Windows.h>
#include <QDebug>

#include "configuremidiout.h"
#include "mididevice-win32.h"
#include "midiout-win32.h"

/*****************************************************************************
 * MIDIOut Initialization
 *****************************************************************************/

MIDIOut::~MIDIOut()
{
	/* Delete all MIDI devices. */
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();
}

void MIDIOut::init()
{
	/* Find out what devices we have */
	rescanDevices();
}

void MIDIOut::open(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		dev->open();
	else
		qDebug() << name() << "has no output number:" << output;
}

void MIDIOut::close(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		dev->open();
	else
		qDebug() << name() << "has no output number:" << output;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIOut::rescanDevices()
{
	UINT deviceCount;

	/* Destroy existing devices in case something has changed */
	while (m_devices.isEmpty() == false)
		removeDevice(m_devices.takeFirst());

	/* Create devices for each valid midi input */
	deviceCount = midiOutGetNumDevs();
	for (UINT id = 0; id < deviceCount; id++)
		addDevice(new MIDIDevice(this, id));
}

MIDIDevice* MIDIOut::device(unsigned int index)
{
	if (index > static_cast<unsigned int> (m_devices.count()))
		return NULL;
	else
		return m_devices.at(index);
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

	device->close();
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

	QListIterator <MIDIDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next()->name();

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
