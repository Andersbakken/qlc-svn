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

HIDInputEvent::HIDInputEvent(t_input input, t_input_channel channel,
			     t_input_value value) : QEvent(_HIDInputEventType)
{
	m_input = input;
	m_channel = channel;
	m_value = value;
}

HIDInputEvent::~HIDInputEvent()
{
}

/*****************************************************************************
 * HIDInput Initialization
 *****************************************************************************/

void HIDInput::init()
{
	QDir dir("/dev/input/");
	QStringList nameFilters;
	QStringList entries;
	QStringList::iterator it;
	HIDDevice* device;
	QString path;
	t_input line = 0;

	nameFilters << "event*";
	entries = dir.entryList(nameFilters, QDir::Files | QDir::System);
	for (it = entries.begin(); it != entries.end(); ++it)
	{
		path = dir.absolutePath() + QDir::separator() + *it;
		device = new HIDEventDevice(this, line++, path);
		m_devices.append(device);
	}

	m_poller = new HIDPoller(this);
}

HIDInput::~HIDInput()
{
	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();

	m_poller->stop();
	delete m_poller;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

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

t_input HIDInput::inputs()
{
	return m_devices.count();
}

t_input_channel HIDInput::channels(t_input input)
{
	if (input >= m_devices.count())
		return NULL;
	else
		return m_devices.at(input)->channels();
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

QString HIDInput::infoText()
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
		info += QString("No HID devices found under /dev/input/");
		info += QString("</TD>");
		info += QString("</TR>");
	}
	else
	{
		QListIterator <HIDDevice*> it(m_devices);
		while (it.hasNext() == true)
			info += it.next()->infoText();
	}

	info += QString("</TABLE>");

	return info;
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
		emit valueChanged(this, e->m_input, e->m_channel, e->m_value);
		event->accept();
	}
}

void HIDInput::connectInputData(QObject* listener)
{
	Q_ASSERT(listener != NULL);

	qDebug() << "Connect" << listener;

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
