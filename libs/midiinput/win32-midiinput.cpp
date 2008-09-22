/*
  Q Light Controller
  win32-midiinput.cpp
  
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
#include <QPalette>
#include <QDebug>

#include "win32-mididevice.h"
#include "win32-midiinput.h"

/*****************************************************************************
 * MIDIInput Initialization
 *****************************************************************************/

void MIDIInput::init()
{
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
		qDebug() << name() << "has no input number:" << input;
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
		qDebug() << name() << "has no input number:" << input;
	}
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIInput::rescanDevices()
{
	UINT deviceCount;
	
	deviceCount = midiInGetNumDevs();
	for (UINT id = 0; id < deviceCount; id++)
		addDevice(new MIDIDevice(this, id));
}

MIDIDevice* MIDIInput::device(unsigned int index)
{
	if (index > static_cast<unsigned int> (m_devices.count()))
		return NULL;
	else
		return m_devices.at(index);
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

	device->close();
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
	//ConfigureMIDIInput cmi(NULL, this);
	//cmi.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString MIDIInput::infoText()
{
	QString info;
	QString t;

	/* HTML Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\" COLSPAN=\"3\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");

	/* Device title */
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"4\">");
	info += QString("Device");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Name title */
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"4\">");
	info += QString("Name");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Mode title */
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"4\">");
	info += QString("Channels");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	/* Devices */
	if (m_devices.count() == 0)
	{
		info += QString("<TR>");
		info += QString("<TD COLSPAN=\"3\">");
		info += QString("Unable to find any MIDI inputs.");
		info += QString("</TD>");
		info += QString("</TR>");
	}
	else
	{
		QListIterator <MIDIDevice*> it(m_devices);
		while (it.hasNext() == true)
			info += it.next()->infoText();
	}

	info += QString("</TABLE>");

	return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIInput::slotDeviceValueChanged(MIDIDevice* device,
				       t_input_channel channel,
				       t_input_value value)
{
	qDebug() << QString("%1: C:%2 V:%3").arg(device->name())
			.arg(channel).arg(value);

	emit valueChanged(this, device->line(), channel, value);
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

void MIDIInput::feedBack(t_input /*input*/, t_input_channel /*channel*/,
			 t_input_value /*value*/)
{
	/* TODO */
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiinput, MIDIInput)
