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

#include <qapplication.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qdir.h>

#include "hidinput.h"

extern "C" InputPlugin* create()
{
	return new HIDInput;
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

HIDInput::HIDInput() : InputPlugin()
{
	m_version = 0x00010000;
	m_name = QString("HID Input");
	m_type = InputType;
}

HIDInput::~HIDInput()
{
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int HIDInput::open()
{
	HIDDevice* hidDevice = NULL;
	QDir dir("/dev/input/");
	QStringList entries = dir.entryList(QDir::Files | QDir::System);
	QStringList::iterator it;
	QString path;

	close();

	for (it = entries.begin(); it != entries.end(); ++it)
	{
		path = dir.absPath() + QString("/") + *it;
		hidDevice = device(path);
		if (hidDevice == NULL)
		{
			hidDevice = new HIDDevice(this, path);

			if (hidDevice->open() == true)
				m_devices.append(hidDevice);
			else
				delete hidDevice;
			hidDevice = NULL;
		}
	}
}

int HIDInput::close()
{
	HIDDevice* hidDevice = NULL;

	while ((hidDevice = m_devices.take(0)) != NULL)
	{
		hidDevice->close();
		delete hidDevice;
	}
}

HIDDevice* HIDInput::device(const QString& path)
{
	QPtrListIterator<HIDDevice> it(m_devices);

	while (it.current() != NULL)
	{
		if (it.current()->path() == path)
			return it.current();
		++it;
	}

	return NULL;
}

t_input HIDInput::inputs()
{
	return m_devices.count();
}

t_input_channel HIDInput::channels(t_input input)
{
	return 0;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int HIDInput::configure(QWidget* parentWidget)
{
	QMessageBox::information(parentWidget,
				 "Configure HID Input",
				 "This plugin has no configurable options");
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString HIDInput::infoText()
{
	QString info = QString::null;
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
	info += QApplication::palette().active().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().active().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	return info;
}

void HIDInput::feedBack(t_input input, t_input_channel channel,
			t_input_value value)
{
}
