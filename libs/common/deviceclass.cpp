/*
  Q Light Controller
  deviceclass.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#include <qfile.h>
#include <qstring.h>
#include <qdir.h>
#include <qmessagebox.h>

#include "common/deviceclass.h"
#include "common/logicalchannel.h"
#include "common/capability.h"
#include "common/settings.h"

DeviceClass::DeviceClass()
{
	m_manufacturer = QString::null;
	m_model = QString::null;
	m_imageFileName = QString::null;
	m_type = QString("Dimmer");
}

// The copy constructor
DeviceClass::DeviceClass(DeviceClass *dc)
{
	m_manufacturer = QString(dc->m_manufacturer);
	m_model = QString( dc->m_model);
	m_imageFileName = QString(dc->m_imageFileName);
	m_type = QString(dc->m_type);

	for (LogicalChannel* c = dc->channels()->first(); c != NULL;
	     c = dc->channels()->next())
	{
		m_channels.append(new LogicalChannel(c));
	}
}

DeviceClass::~DeviceClass()
{
	while (m_channels.isEmpty() == false)
	{
		LogicalChannel* ch = m_channels.take(0);
		delete ch;
	}
}

void DeviceClass::setManufacturer(const QString mfg)
{
	m_manufacturer = QString(mfg);
}

void DeviceClass::setModel(const QString model)
{
	m_model = QString(model);
}

void DeviceClass::setImageFileName(const QString &fileName)
{
	m_imageFileName = QString(fileName);
}

void DeviceClass::setType(const QString &type)
{
	m_type = QString(type);
}

void DeviceClass::addChannel(LogicalChannel* channel)
{
	ASSERT(channel != NULL);
	m_channels.append(channel);
}

bool DeviceClass::removeChannel(LogicalChannel* channel)
{
	ASSERT(channel != NULL);

	for (LogicalChannel* ch = m_channels.first(); ch != NULL;
	     ch = m_channels.next())
	{
		if (ch == channel)
		{
			delete m_channels.take();
			return true;
		}
	}

	return false;
}


void DeviceClass::createInfo(QPtrList <QString> &list)
{
	QString t;

	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else if (*s == QString("Manufacturer"))
		{
			m_manufacturer = *(list.next());
		}
		else if (*s == QString("Model"))
		{
			m_model = *(list.next());
		}
		else if (*s == QString("Protocol"))
		{
			t = *(list.next());
		}
		else if (*s == QString("Type"))
		{
			m_type = *(list.next());
		}
		else
		{
			// Unknown keyword (at this time)
			list.next();
		}
	}
}

void DeviceClass::createChannel(QPtrList <QString> &list)
{
	QString t;
	QString name;

	LogicalChannel *ch = new LogicalChannel();

	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else if (*s == QString("Number"))
		{
			t = *(list.next());
			ch->setChannel(t.toInt());
		}
		else if (*s == QString("Name"))
		{
			ch->setName(*(list.next()));
		}
		else if (s->at(0).isNumber() == true)
		{
			Capability* cap = new Capability();

			list.prev(); // Reel back once so that the value goes to cap
			cap->createInfo(list);

			ch->capabilities()->append(cap);
		}
		else
		{
			// Unknown keyword
			list.next();
		}
	}

	m_channels.append(ch);
}

int DeviceClass::saveToFile(const QString &fileName)
{
	QFile file(fileName);

	if (file.open(IO_WriteOnly))
	{
		QString t;
		QString s;

		s = QString("#\n# Q Light Controller device class definition file\n#\n");
		file.writeBlock((const char*) s, s.length());

		s = QString("Entry = Device Class\n");
		file.writeBlock((const char*) s, s.length());

		s = QString("Manufacturer = ") + m_manufacturer + QString("\n");
		file.writeBlock((const char*) s, s.length());

		s = QString("Model = ") + m_model + QString("\n");
		file.writeBlock((const char*) s, s.length());

		s = QString("Type = ") + m_type + QString("\n");
		file.writeBlock((const char*) s, s.length());

		s = QString("#\n");
		file.writeBlock((const char*) s, s.length());

		for(LogicalChannel* ch = m_channels.first(); ch != NULL;
		    ch = m_channels.next())
		{
			ch->saveToFile(file);
		}

		file.close();
	}

	return file.status();
}


//
// Create device class from file entry
//
DeviceClass* DeviceClass::createDeviceClass(QPtrList <QString> &list)
{
	QString entry;
	QString manufacturer;
	QString model;
	QString t;

	DeviceClass* dc = new DeviceClass();

	for (QString *s = list.first(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			entry = *(list.next());
			if (entry == QString("Device Class"))
			{
				dc->createInfo(list);
			}
			else if (entry == QString("Channel"))
			{
				dc->createChannel(list);
			}
			else if (entry == QString("Function"))
			{
				list.next();
			}
		}
		else
		{
			// Unknown keyword
			list.next();
		}
	}

	return dc;
}
