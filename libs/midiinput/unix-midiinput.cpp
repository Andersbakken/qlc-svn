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

void MIDIInput::init()
{
	m_poller = new MIDIPoller(this);
	rescanDevices();
}

MIDIInput::~MIDIInput()
{
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();

		m_poller->stop();
	delete m_poller;
}

void MIDIInput::open(t_input input)
{
	MIDIDevice* dev;

	dev = device(input);
	if (dev != NULL)
		m_poller->addDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

void MIDIInput::close(t_input input)
{
	MIDIDevice* dev;

	dev = device(input);
	if (dev != NULL)
		m_poller->removeDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void MIDIInput::rescanDevices()
{
	t_input line = 0;

	QDir dir("/dev/", QString("midi*"), QDir::Name, QDir::System);
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QString path(dir.absoluteFilePath(it.next()));

		if (device(path) == NULL)
			addDevice(new MIDIDevice(this, line++, path));
	}
}

MIDIDevice* MIDIInput::device(const QString& path)
{
	QListIterator <MIDIDevice*> it(m_devices);

	while (it.hasNext() == true)
	{
		MIDIDevice* dev = it.next();
		if (dev->path() == path)
			return dev;
	}

	return NULL;
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

		emit valueChanged(this, e->m_input, e->m_channel,
				  e->m_value);
		event->accept();
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

void MIDIInput::feedBack(t_input /*input*/, t_input_channel /*channel*/,
			 t_input_value /*value*/)
{
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(midiinput, MIDIInput)
