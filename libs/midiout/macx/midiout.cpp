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

#include "configuremidiout.h"
#include "mididevice.h"
#include "midiout.h"

/*****************************************************************************
 * MIDIOut Initialization
 *****************************************************************************/

void MIDIOut::init()
{
	OSStatus s;

	m_client = NULL;
	s = MIDIClientCreate(CFSTR("QLC MIDI Output Plugin"), NULL, NULL,
			     &m_client);
	if (s != 0)
		qWarning() << "Unable to create a MIDI Client!";
	else
		rescanDevices();
}

MIDIOut::~MIDIOut()
{
	/* Delete all MIDI devices. */
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();
}

void MIDIOut::open(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		dev->open();
	else
		qWarning() << name() << "has no output number:" << output;
}

void MIDIOut::close(t_output output)
{
	MIDIDevice* dev = device(output);
	if (dev != NULL)
		dev->open();
	else
		qWarning() << name() << "has no output number:" << output;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIOut::rescanDevices()
{
	/* Treat all devices nonexistent and doomed for destruction */
	QList <MIDIDevice*> destroyList(m_devices);

	/* Find out which devices are still present */
	for (ItemCount i = 0; i < MIDIGetNumberOfDevices(); i++)
	{
		MIDIDeviceRef dev = MIDIGetDevice(i);
		for (ItemCount j = 0; j < MIDIDeviceGetNumberOfEntities(dev); j++)
		{
			MIDIEntityRef entity = MIDIDeviceGetEntity(dev, j);
			OSStatus s = 0;
			SInt32 uid = 0;

			/* Check if the entity is able to send data */
			if (MIDIEntityGetNumberOfDestinations(entity) == 0)
				continue;

			/* Extract UID from the entity */
			s = MIDIObjectGetIntegerProperty(entity,
							 kMIDIPropertyUniqueID,
							 &uid);
			if (s != 0)
			{
				qWarning() << "Unable to get entity UID";
				continue;
			}

			MIDIDevice* dev(deviceByUID(uid));
			if (dev != NULL)
			{
				/* Device still exists */
				destroyList.removeAll(dev);
			}
			else
			{
				/* New device */
				dev = new MIDIDevice(this, entity);
				Q_ASSERT(dev != NULL);
				if (dev->extractUID() == true &&
				    dev->extractName() == true)
				{
					addDevice(dev);
				}
				else
				{
					delete dev;
					dev = NULL;
				}
			}
		}
	}

	/* Destroy all devices that were no longer present */
	while (destroyList.isEmpty() == false)
		delete destroyList.takeFirst();
}

MIDIDevice* MIDIOut::deviceByUID(SInt32 uid)
{
	QListIterator <MIDIDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		MIDIDevice* dev(it.next());
		if (dev->uid() == uid)
			return dev;
	}

	return NULL;
}

MIDIDevice* MIDIOut::device(t_output output)
{
	if (output < m_devices.size())
		return m_devices.at(output);
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
		str += QString("This plugin provides DMX output support thru ");
		str += QString("various MIDI devices.");
		str += QString("</P>");
	}
	else
	{
		MIDIDevice* dev = device(output);
		if (dev != NULL)
		{
			str += device(output)->infoText();
			/*str += QString("<H3>%1</H3>").arg(outputs()[output]);
	                str += QString("<P>");
	                str += QString("Device is operating correctly.");
	                str += QString("</P>");*/
		}
		else
		{
			str += QString("<P>");
			str += QString("No MIDI output devices found.");
			str += QString("</P>");
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
