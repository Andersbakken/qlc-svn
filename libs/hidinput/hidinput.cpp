/*
  Q Light Controller
  hidinput.cpp

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
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include "configurehidinput.h"
#include "hideventdevice.h"
#include "hidpoller.h"
#include "hidinput.h"

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

static const QEvent::Type _HIDInputEventType = static_cast<QEvent::Type>
	(QEvent::registerEventType());

HIDInputEvent::HIDInputEvent(HIDDevice* device, t_input input,
			     t_input_channel channel, t_input_value value,
			     bool alive) : QEvent(_HIDInputEventType)
{
	m_device = device;
	m_input = input;
	m_channel = channel;
	m_value = value;
	m_alive = alive;
}

HIDInputEvent::~HIDInputEvent()
{
}

/*****************************************************************************
 * HIDInput Initialization
 *****************************************************************************/

void HIDInput::init()
{
	m_poller = new HIDPoller(this);
	rescanDevices();
}

HIDInput::~HIDInput()
{
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();

	m_poller->stop();
	delete m_poller;
}

void HIDInput::open(t_input input)
{
	HIDDevice* dev = device(input);
	if (dev != NULL)
		addPollDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

void HIDInput::close(t_input input)
{
	HIDDevice* dev = device(input);
	if (dev != NULL)
		removePollDevice(dev);
	else
		qDebug() << name() << "has no input number:" << input;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void HIDInput::rescanDevices()
{
	QDir dir("/dev/input/");
	QStringList nameFilters;
	QStringList entries;
	QStringList::iterator it;
	t_input line = 0;
	QString path;

	nameFilters << "event*";
	entries = dir.entryList(nameFilters, QDir::Files | QDir::System);
	for (it = entries.begin(); it != entries.end(); ++it)
	{
		path = dir.absolutePath() + QDir::separator() + *it;

		if (device(path) == NULL)
			addDevice(new HIDEventDevice(this, line++, path));
	}
}

HIDDevice* HIDInput::device(const QString& path)
{
	QListIterator <HIDDevice*> it(m_devices);

	while (it.hasNext() == true)
	{
		HIDDevice* dev = it.next();
		if (dev->path() == path)
			return dev;
	}

	return NULL;
}

HIDDevice* HIDInput::device(const unsigned int index)
{
	if (index > static_cast<unsigned int> (m_devices.count()))
		return NULL;
	else
		return m_devices.at(index);
}

void HIDInput::addDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);

	m_devices.append(device);

	emit deviceAdded(device);
}

void HIDInput::removeDevice(HIDDevice* device)
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

QString HIDInput::name()
{
	return QString("HID Input");
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

QStringList HIDInput::inputs()
{
	QStringList list;

	QListIterator <HIDDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next()->name();

	return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void HIDInput::configure()
{
	ConfigureHIDInput conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString HIDInput::infoText(t_input input)
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
		str += QString("various HID (Human Interface Device) ");
		str += QString("devices like joysticks, mice and keyboards.");
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

void HIDInput::addPollDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);
	m_poller->addDevice(device);
}

void HIDInput::removePollDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);
	m_poller->removeDevice(device);
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDInput::customEvent(QEvent* event)
{
	if (event->type() == _HIDInputEventType)
	{
		HIDInputEvent* e = static_cast<HIDInputEvent*> (event);

		if (e->m_alive == true)
		{
			emit valueChanged(this, e->m_input, e->m_channel,
					  e->m_value);
		}
		else
		{
			removeDevice(e->m_device);
		}

		event->accept();
	}
}

void HIDInput::connectInputData(QObject* listener)
{
	Q_ASSERT(listener != NULL);

	connect(this, SIGNAL(valueChanged(QLCInPlugin*,t_input,t_input_channel,
					  t_input_value)),
		listener,
		SLOT(slotValueChanged(QLCInPlugin*,t_input,t_input_channel,
				      t_input_value)));
}

void HIDInput::feedBack(t_input /*input*/, t_input_channel /*channel*/,
			t_input_value /*value*/)
{
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(hidinput, HIDInput)
