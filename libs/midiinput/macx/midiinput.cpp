/*
  Q Light Controller
  midiinput.cpp

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

#include <QMessageBox>
#include <QStringList>
#include <QPalette>
#include <QDebug>

#include "mididevice.h"
#include "midiinput.h"

/*****************************************************************************
 * MIDIInput Initialization
 *****************************************************************************/

void MIDIInput::init()
{
	OSStatus s;

	m_client = NULL;
	s = MIDIClientCreate(CFSTR("QLC MIDI Input Plugin"), NULL, NULL,
			     &m_client);
	if (s != 0)
		qWarning() << "Unable to create a MIDI Client!";
	else
		rescanDevices();
}

MIDIInput::~MIDIInput()
{
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();
}

void MIDIInput::open(t_input input)
{
	MIDIDevice* dev = device(input);
	if (dev != NULL)
	{
		connect(dev, SIGNAL(valueChanged(MIDIDevice*,
						 t_input_channel,
						 t_input_value)),
			this, SLOT(slotDeviceValueChanged(MIDIDevice*,
							  t_input_channel,
							  t_input_value)));

		dev->open();
	}
	else
	{
		qWarning() << name() << "has no input number:" << input;
	}
}

void MIDIInput::close(t_input input)
{
	MIDIDevice* dev = device(input);
	if (dev != NULL)
	{
		disconnect(dev, SIGNAL(valueChanged(MIDIDevice*,
						    t_input_channel,
						    t_input_value)),
			this, SLOT(slotDeviceValueChanged(MIDIDevice*,
							  t_input_channel,
							  t_input_value)));

		dev->close();
	}
	else
	{
		qWarning() << name() << "has no input number:" << input;
	}
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIInput::rescanDevices()
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

			/* Check if the entity is able to receive data */
			if (MIDIEntityGetNumberOfSources(entity) == 0)
				continue;

			/* Extract UID from the source */
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
					m_devices.append(dev);
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

MIDIDevice* MIDIInput::deviceByUID(SInt32 uid)
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

MIDIDevice* MIDIInput::device(t_input input)
{
	if (input < m_devices.size())
		return m_devices.at(input);
	else
		return NULL;
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
	int r = QMessageBox::question(NULL, name(),
				tr("Do you wish to re-scan your hardware?"),
				QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
		rescanDevices();
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
	else if (device(input) != NULL)
	{
		str += QString("<H3>%1</H3>").arg(inputs()[input]);
		str += QString("<P>");
		str += QString("Device is operating correctly.");
		str += QString("</P>");
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIInput::slotDeviceValueChanged(MIDIDevice* device,
				       t_input_channel channel,
				       t_input_value value)
{
	t_input input;

	Q_ASSERT(device != NULL);

	input = m_devices.indexOf(device);
	emit valueChanged(this, input, channel, value);
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
	MIDIDevice* dev = device(input);
	if (dev != NULL)
		dev->feedBack(channel, value);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiinput, MIDIInput)
