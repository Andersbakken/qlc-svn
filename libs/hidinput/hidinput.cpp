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
#include "hidjsdevice.h"
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
	t_input line = 0;

	/* Copy the pointers from our devices list into a list of devices
	   to destroy in case some of them have disappeared. */
	QList <HIDDevice*> destroyList(m_devices);

	/* Check all files matching filter "/dev/input/js*" */
	QDir dir("/dev/input/", QString("js*"), QDir::Name, QDir::System);
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		/* Construct an absolute path for the file */
		QString path(dir.absoluteFilePath(it.next()));

		/* Check that we can at least read from the device. Otherwise
		   deem it to ge destroyed. */
		if (QFile::permissions(path) & QFile::ReadOther)
		{
			HIDDevice* dev = device(path);
			if (dev == NULL)
			{
				/* This device is unknown to us. Add it. */
				dev = new HIDJsDevice(this, line++, path);
				addDevice(dev);
			}
			else
			{
				/* Remove the device from our destroy list,
				   since it is still available */
				destroyList.removeAll(dev);
			}
		}
		else
		{
			/* The file is not readable. If we have an entry for
			   it, it must be destroyed. */
			HIDDevice* dev = device(path);
			if (dev != NULL)
				removeDevice(dev);
		}
	}

	/* Destroy all devices that were not found during rescan */
	while (destroyList.isEmpty() == false)
		removeDevice(destroyList.takeFirst());
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

	emit configurationChanged();
}

void HIDInput::removeDevice(HIDDevice* device)
{
	Q_ASSERT(device != NULL);

	removePollDevice(device);
	m_devices.removeAll(device);

	emit deviceRemoved(device);
	delete device;

	emit configurationChanged();
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

	str += QString("<H3>%1</H3>").arg(name());

	if (input == KInputInvalid)
	{
		/* Plugin or just an invalid input selected. Display generic
		   information. */
		str += QString("<P>This plugin provides input support for ");
		str += QString("various Human Interface Devices like ");
		str += QString("joysticks, mice and keyboards.</P>");
	}
	else
	{
		/* A specific input line selected. Display its information if
		   available. */
		HIDDevice* dev = device(input);

		if (dev != NULL)
			str += dev->infoText();
		else
			str += tr("<P>%1: Device not found.</P>").arg(input+1);
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

	connect(this, SIGNAL(configurationChanged()),
		listener, SLOT(slotConfigurationChanged()));
}

void HIDInput::feedBack(t_input /*input*/, t_input_channel /*channel*/,
			t_input_value /*value*/)
{
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(hidinput, HIDInput)
