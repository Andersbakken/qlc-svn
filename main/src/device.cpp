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

#include "app.h"
#include "doc.h"
#include "device.h"
#include "scene.h"
#include "common/settings.h"
#include "deviceconsole.h"
#include "monitor.h"
#include "deviceproperties.h"
#include "configkeys.h"
#include "deviceclass.h"
#include "logicalchannel.h"

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

extern App* _app;
extern QApplication* _qapp;

Device::Device()
  : QObject(),
    m_deviceClass (            NULL ),
    m_address     ( KChannelInvalid ),
    m_id          (           KNoID ),
    m_name        (   QString::null ),
    m_console     (            NULL ),
    m_monitor     (            NULL )
{
}


Device::~Device()
{
  if (m_console != NULL)
    {
      slotConsoleClosed();
    }

  if (m_monitor != NULL)
    {
      slotMonitorClosed();
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
       QString("Function ID is missing for <") + name + QString(">"));
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

  if (m_monitor)
    m_monitor->setCaption(m_name + " Monitor");
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

  if (m_monitor)
    {
      slotMonitorClosed();
      viewMonitor();
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

  if (m_monitor)
    {
      slotMonitorClosed();
      viewMonitor();
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
  assert(m_deviceClass);

  QString t;
  info = QString::null;

  //
  // General Info
  //
  info += QString("<HTML><HEAD><TITLE>Device Info</TITLE></HEAD><BODY>");
  info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
  info += QString("<TR><TD BGCOLOR=\"darkblue\"><FONT COLOR=\"white\" SIZE=5>");
  info += name() + QString("</FONT></TD></TR></TABLE>");
  info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
  info += QString("<TR>\n");
  info += QString("<TD><B>Manufacturer</B></TD>");
  info += QString("<TD>") + m_deviceClass->manufacturer() + QString("</TD>");
  info += QString("</TR>");
  info += QString("<TR>");
  info += QString("<TD><B>Model</B></TD>");
  info += QString("<TD>") + m_deviceClass->model() + QString("</TD>");
  info += QString("</TR>");
  info += QString("<TR>");
  info += QString("<TD><B>Type</B></TD>");
  info += QString("<TD>") + m_deviceClass->type() + QString("</TD>");
  info += QString("</TR>");
  info += QString("<TR>");
  info += QString("<TD><B>Universe</B></TD>");
  t.sprintf("%d", universe() + 1);
  info += QString("<TD>") + t + QString("</TD>");
  info += QString("</TR>");
  info += QString("<TR>");
  info += QString("<TD><B>Address space</B></TD>");
  t.sprintf("%d - %d",
	    address() + 1, address() + m_deviceClass->channels()->count());
  info += QString("<TD>") + t + QString("</TD>");
  info += QString("</TR>");
  info += QString("</TABLE>");

  //
  // Channels
  //
  info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
  info += QString("<TR>");
  info += QString("<TD BGCOLOR=\"darkblue\">");
  info += QString("<FONT COLOR=\"white\" SIZE=\"3\">Channel</FONT>");
  info += QString("</TD>");
  info += QString("<TD BGCOLOR=\"darkblue\">");
  info += QString("<FONT COLOR=\"white\" SIZE=\"3\">DMX</FONT>");
  info += QString("</TD>");
  info += QString("</TR>");
  info += QString("<TD BGCOLOR=\"darkblue\">");
  info += QString("<FONT COLOR=\"white\" SIZE=\"3\">Name</FONT>");
  info += QString("</TD>");
  info += QString("</TR>");

  for (t_channel ch = 0; ch < (t_channel) m_deviceClass->channels()->count();
       ch++)
    {
      t.setNum(ch + 1);
      info += QString("<TR>");
      info += QString("<TD>" + t + "</TD>");
      t.setNum(address() + ch + 1);
      info += QString("<TD>" + t + "</TD>");
      info += QString("<TD>");
      info += m_deviceClass->channels()->at(ch)->name();
      info += QString("</TD>");
    }

  info += QString("</TR>");
  info += QString("</TABLE>");

  info += QString("</BODY></HTML>");
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
      connect(m_console, SIGNAL(closed()), this, SLOT(slotConsoleClosed()));

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


void Device::viewMonitor()
{
  t_channel channels = deviceClass()->channels()->count();

  if (m_monitor == NULL)
    {
      m_monitor = new Monitor(_app->workspace(), universeAddress(),
                              universeAddress() + channels - 1);
      m_monitor->init();
      m_monitor->setCaption(m_name + " Monitor");
      m_monitor->setIcon(PIXMAPS + QString("monitor.png"));

      connect(m_monitor, SIGNAL(closed()), this, SLOT(slotMonitorClosed()));
      m_monitor->show();
    }
  else
    {
      m_monitor->hide();
      m_monitor->show();
    }
}


void Device::slotMonitorClosed()
{
  delete m_monitor;
  m_monitor = NULL;
}


void Device::viewProperties()
{
  DeviceProperties* dp = new DeviceProperties(this);
  dp->init();
  dp->exec();

  delete dp;
}
