/*
  Q Light Controller
  device.cpp

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

#include <unistd.h>
#include <qptrlist.h>
#include <qtooltip.h>
#include <qframe.h>
#include <qbutton.h>
#include <qfile.h>
#include <qlayout.h>
#include <qthread.h>
#include <qpixmap.h>
#include <qworkspace.h>
#include <qmessagebox.h>
#include <assert.h>
#include <qdom.h>

#include "common/deviceclass.h"
#include "common/logicalchannel.h"
#include "common/settings.h"
#include "app.h"
#include "doc.h"
#include "device.h"
#include "scene.h"
#include "deviceconsole.h"
#include "deviceproperties.h"
#include "configkeys.h"

extern App* _app;
extern QApplication* _qapp;

const QString KXMLFixtureNode         (      "Fixture" );
const QString KXMLFixtureName         (         "Name" );
const QString KXMLFixtureManufacturer ( "Manufacturer" );
const QString KXMLFixtureModel        (        "Model" );
const QString KXMLFixtureID           (           "ID" );
const QString KXMLFixtureUniverse     (     "Universe" );
const QString KXMLFixtureAddress      (      "Address" );

Device::Device() : QObject(),
	m_deviceClass ( NULL ),
	m_address     ( KChannelInvalid ),
	m_id          ( KNoID ),
	m_name        ( QString::null ),
	m_console     ( NULL )
{
}


Device::~Device()
{
	if (m_console != NULL)
	{
		slotConsoleClosed();
	}
}


void Device::saveToFile(QFile &file)
{
	QString s;
	QString t;
	
	// Comment
	s = QString("# Device Entry\n");
	file.writeBlock((const char*) s, s.length());
	
	// Entry type
	s = QString("Entry = Device") + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// Name
	s = QString("Name = ") + name() + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// Manufacturer
	s = QString("Manufacturer = ") + deviceClass()->manufacturer()
		+ QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// Model
	s = QString("Model = ") + deviceClass()->model() + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// ID
	t.setNum(m_id);
	s = QString("ID = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// Address
	t.setNum(address());
	s = QString("Address = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());
	
	// Universe
	t.setNum(universe());
	s = QString("Universe = ") + t + QString("\n");
	file.writeBlock((const char*) s, s.length());
}


/**
 * Save the device's properties to an XML document
 *
 * @param doc The QDomDocument to save the information to
 */
void Device::saveXML(QDomDocument* doc)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	assert(doc);

	/* Device entry */
	root = doc->createElement(KXMLFixtureNode);
	doc->appendChild(root);

	/* Name */
	tag = doc->createElement(KXMLFixtureName);
	root.appendChild(tag);
	text = doc->createTextNode(name());
	tag.appendChild(text);

	/* Manufacturer */
	tag = doc->createElement(KXMLFixtureManufacturer);
	root.appendChild(tag);
	text = doc->createTextNode(deviceClass()->manufacturer());
	tag.appendChild(text);

	/* Model */
	tag = doc->createElement(KXMLFixtureModel);
	root.appendChild(tag);
	text = doc->createTextNode(deviceClass()->model());
	tag.appendChild(text);

	/* ID */
	tag = doc->createElement(KXMLFixtureID);
	root.appendChild(tag);
	str.setNum(id());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Universe */
	tag = doc->createElement(KXMLFixtureUniverse);
	root.appendChild(tag);
	str.setNum(universe());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Address */
	tag = doc->createElement(KXMLFixtureAddress);
	root.appendChild(tag);
	str.setNum(address());
	text = doc->createTextNode(str);
	tag.appendChild(text);
}

//
// Create a device from a file entry
//
Device* Device::create(QPtrList <QString> &list)
{
	QString name = QString::null;
	QString manufacturer = QString::null;
	QString model = QString::null;
	
	t_channel address = KChannelInvalid;
	t_channel universe = 0;
	t_device_id id = KNoID;
	
	for (QString* s = list.next(); s != NULL; s = list.next())
	{
		if (*s == QString("Entry"))
		{
			s = list.prev();
			break;
		}
		else if (*s == QString("Name"))
		{
			name = *(list.next());
		}
		else if (*s == QString("Manufacturer"))
		{
			manufacturer = *(list.next());
		}
		else if (*s == QString("Model"))
		{
			model = *(list.next());
		}
		else if (*s == QString("ID"))
		{
			id = list.next()->toInt();
		}
		else if (*s == QString("Address"))
		{
			address = list.next()->toInt();
		}
		else if (*s == QString("Universe"))
		{
			universe = list.next()->toInt();
		}
		else
		{
			// Unknown keyword
			list.next();
		}
	}
	
	if (id == KNoID)
	{
		QMessageBox::warning(_app, KApplicationNameShort,
				     QString("Function ID is missing for <")
				     + name + QString(">"));
	}
	
	DeviceClass* dc = _app->searchDeviceClass(manufacturer, model);
	if (dc == NULL)
	{
		QString msg;
		msg = QString("Unable to create device!\n");
		msg += QString("Name: ") + name + QString("\n");
		msg += QString("Manufacturer: ") + manufacturer + QString("\n");
		msg += QString("Model: ") + model + QString("\n");
		msg += QString("No such device class");
		
		QMessageBox::critical(_app, KApplicationNameShort, msg);
		
		return NULL;
	}
	else
	{
		Device* d = _app->doc()->newDevice(dc, name, address, universe, id);
		return d;
	}
}


void Device::setID(t_device_id id)
{
	m_id = id;
	
	emit changed(m_id);
}


t_device_id Device::id()
{
	return m_id;
}

void Device::setName(QString name)
{
	m_name = name;
	
	_app->doc()->setModified(true);
	
	emit changed(m_id);
	
	if (m_console)
		m_console->setCaption(m_name + " Console");
}


QString Device::name()
{
	return m_name;
}

/**
 * Set the device address
 *
 * @param address The DMX address (0-511)
 */
void Device::setAddress(t_channel address)
{
	/* The address part is stored in the lowest 9 bits */
	m_address = (m_address & 0xFE00) | (address & 0x01FF);
	
	if (m_console)
	{
		slotConsoleClosed();
		viewConsole();
	}
	
	_app->doc()->setModified(true);
	
	emit changed(m_id);
}

/**
 * Get the device's DMX address (0-511)
 *
 */
t_channel Device::address()
{
	/* The address part is stored in the lowest 9 bits */
	return (m_address & 0x01FF);
}


/**
 * Set the device's universe
 *
 * @param universe The DMX universe number (0-127)
 */
void Device::setUniverse(t_channel universe)
{
	/* The universe part is stored in the highest 7 bits */
	m_address = (m_address & 0x01FF) | (universe << 9);
	
	if (m_console)
	{
		slotConsoleClosed();
		viewConsole();
	}
	
	_app->doc()->setModified(true);
	
	emit changed(m_id);
}


/**
 * Get the device's universe
 *
 * @return DMX universe number
 */
t_channel Device::universe()
{
	/* The universe part is stored in the highest 7 bits */
	return (m_address >> 9);
}


/**
 * Get the complete address, including universe and DMX address
 */
t_channel Device::universeAddress()
{
	return m_address;
}

/**
 * Return the number of channels taken by this device
 */
t_channel Device::channels()
{
	if (m_deviceClass == NULL)
		return 0;
	else
		return m_deviceClass->channels()->count();
}

void Device::setDeviceClass(DeviceClass* dc)
{
	m_deviceClass = dc;
	
	emit changed(m_id);
}


DeviceClass* Device::deviceClass()
{
	return m_deviceClass;
}


void Device::infoText(QString &info)
{
	QString t;

	assert(m_deviceClass);

	info = QString::null;

	// HTML header
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Device Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	//
	// Device info
	//
	
	// Device title
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");
	
	// Manufacturer
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Manufacturer</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += m_deviceClass->manufacturer();
	info += QString("</TD>");
	info += QString("</TR>");

	// Model
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Model</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += m_deviceClass->model();
	info += QString("</TD>");
	info += QString("</TR>");
	
	// Type
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Type</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += m_deviceClass->type();
	info += QString("</TD>");
	info += QString("</TR>");

	// Universe
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Universe</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += t.sprintf("%d", universe() + 1);
	info += QString("</TD>");
	info += QString("</TR>");
	
	// Address
	info += QString("<TR>");
	info += QString("<TD>");
	info += QString("<B>Address space</B>");
	info += QString("</TD>");
	info += QString("<TD>");
	info += t.sprintf("%d - %d", address() + 1, address() + channels());
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");
	
	//
	// Channels
	//
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");
	
	// Relative channel column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("Channel");
	info += QString("</FONT>");
	info += QString("</TD>");
	
	// DMX channel column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("DMX");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	// Channel name column title
	info += QString("<TD BGCOLOR=\"");
	info += _app->colorGroup().highlight().name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += _app->colorGroup().highlightedText().name();
	info += QString("\" SIZE=\"3\">");
	info += QString("Name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	
	// Fill table with device's channels
	for (t_channel ch = 0; ch < channels();	ch++)
	{
		info += QString("<TR>");

		// Relative channel
		info += QString("<TD>");
		info += t.setNum(ch + 1);
		info += QString("</TD>");

		// DMX channel
		info += QString("<TD>");
		info += t.setNum(address() + ch + 1);
		info += QString("</TD>");
		
		// Channel name
		info += QString("<TD>");
		info += m_deviceClass->channels()->at(ch)->name();
		info += QString("</TD>");
	}

	info += QString("</TR>");
	info += QString("</TABLE>");
	info += QString("</BODY>");
	info += QString("</HTML>");
}


void Device::viewConsole()
{
	if (m_console == NULL)
	{
		m_console = new DeviceConsole(_app->workspace());
		m_console->setDevice(m_id);
		
		// Set window title
		m_console->setCaption(m_name + " Console");
		
		// Catch close event
		connect(m_console, SIGNAL(closed()),
			this, SLOT(slotConsoleClosed()));
		
		m_console->show();
		
		m_console->resize(m_console->width() + 100, 300);
	}
	else
	{
		m_console->hide();
		m_console->show();
	}
}


void Device::slotConsoleClosed()
{
	disconnect(m_console);
	delete m_console;
	m_console = NULL;
}


void Device::viewProperties()
{
	DeviceProperties* dp = new DeviceProperties(this);
	dp->init();
	dp->exec();
	
	delete dp;
}
