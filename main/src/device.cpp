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
#include "containerview.h"
#include "scene.h"
#include "channelui.h"
#include "settings.h"
#include "sceneeditor.h"
#include "containerview.h"
#include "monitor.h"
#include "deviceproperties.h"
#include "configkeys.h"

#include <unistd.h>
#include <qptrlist.h>
#include <qtooltip.h>
#include <qframe.h>
#include <qbutton.h>
#include <iostream.h>
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
    m_deviceClass (          NULL ),
    m_address     (    KNoChannel ),
    m_id          (         KNoID ),
    m_name        ( QString::null ),
    m_console     (          NULL ),
    m_sceneEditor (          NULL ),
    m_monitor     (          NULL )
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
}


//
// Create a device from a file entry
//
Device* Device::create(QPtrList <QString> &list)
{
  QString name = QString::null;
  QString manufacturer = QString::null;
  QString model = QString::null;

  t_channel address = KNoChannel;
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
      msg = QString("Unable to add device <");
      msg += name;
      msg += QString(">:\n");
      msg += QString("No device class description found for device class\n<");
      msg += manufacturer;
      msg += QString("/");
      msg += model;
      msg += QString(">");
      
      QMessageBox::critical(_app, KApplicationNameShort, msg);

      return NULL;
    }
  else
    {
      Device* d = _app->doc()->newDevice(dc, name, address, id);
      return d;
    }
}


void Device::setID(t_device_id id)
{
  m_id = id;
}


t_device_id Device::id()
{
  return m_id;
}

void Device::setName(QString name)
{
  m_name = name;
  _app->doc()->setModified(true);
}


QString Device::name()
{
  return m_name;
}

void Device::setAddress(t_channel address)
{
  m_address = address;

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
}


t_channel Device::address()
{
  return m_address;
}

void Device::setDeviceClass(DeviceClass* dc)
{
  m_deviceClass = dc;
}


DeviceClass* Device::deviceClass()
{
  return m_deviceClass;
}


QString Device::infoText()
{
  assert(m_deviceClass);

  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Device Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
  str += QString("<TR><TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=5>");
  str += name() + QString("</FONT></TD></TR></TABLE>");
  str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
  str += QString("<TR>\n");
  str += QString("<TD><B>Manufacturer</B></TD>");
  str += QString("<TD>") + m_deviceClass->manufacturer() + QString("</TD>");
  str += QString("</TR>");
  str += QString("<TR>");
  str += QString("<TD><B>Model</B></TD>");
  str += QString("<TD>") + m_deviceClass->model() + QString("</TD>");
  str += QString("</TR>");
  str += QString("<TR>");
  str += QString("<TD><B>Type</B></TD>");
  str += QString("<TD>") + m_deviceClass->type() + QString("</TD>");
  str += QString("</TR>");
  str += QString("<TR>");
  str += QString("<TD><B>Address space</B></TD>");
  t.sprintf("%d - %d",
	    address(), address() + m_deviceClass->channels()->count() - 1);
  str += QString("<TD>") + t + QString("</TD>");
  str += QString("<TR>");
  str += QString("<TD><B>Channels</B></TD>");
  t.setNum(m_deviceClass->channels()->count());
  str += QString("<TD>") + t + QString("</TD>");
  str += QString("</TR>");
  str += QString("</TABLE>");
  
  if (m_deviceClass->imageFileName() != QString::null)
    {
      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + DEVICECLASSPATH + QString("/");

      str += QString("<IMG SRC=\"") +
	dir + m_deviceClass->imageFileName() +
	QString("\">");
    }

  str += QString("</BODY></HTML>");

  return str;
}


void Device::viewConsole()
{
  if (m_console == NULL)
    {
      QString name = m_name + QString(" Console");
      m_console = new ContainerView((QWidget*) _app->workspace());

      // Tell the monitor the dimensions of its widgets
      m_console->setCaption(name);
      
      // the next 2 lines must be in that order
      // drawback: the initial scene is not displayed automagically
      // otherwise, moved sliders cannot emit the "changed"-signal
      m_sceneEditor = new SceneEditor(this, m_console);
      m_sceneEditor->init();
      createChannelUnits();

      m_sceneEditor->update();

      // Set a nice icon
      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + PIXMAPPATH + QString("/");

      m_console->setIcon(QPixmap(dir + QString("console.xpm")));

      connect(m_console, SIGNAL(closed()), this, SLOT(slotConsoleClosed()));

      m_console->show();
      m_console->setMaximumSize(m_console->size().width() 
				+ SceneEditor::width(), ChannelUI::height());
      m_console->resize(m_console->size().width() 
			+ SceneEditor::width(), ChannelUI::height());
    }
  else
    {
      m_console->hide();
      m_console->show();
    }
}


void Device::createChannelUnits()
{
  ChannelUI* unit = NULL;

  for (unsigned int i = 0; i < deviceClass()->channels()->count(); i++)
    {
      unit = new ChannelUI(this, i, m_console);
      unit->init();
      unit->update();
      m_unitList.append(unit);
    }
}


void Device::slotConsoleClosed()
{
  while (!m_unitList.isEmpty())
    {
      delete m_unitList.take(0);
    }

  disconnect(m_sceneEditor);
  delete m_sceneEditor;
  m_sceneEditor = NULL;

  disconnect(m_console);
  delete m_console;
  m_console = NULL;
}


void Device::viewMonitor()
{
  t_channel channels = deviceClass()->channels()->count();
  
  if (m_monitor == NULL)
    {
      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + PIXMAPPATH + QString("/");

      m_monitor = new Monitor(_app->workspace(), address(),
                              address() + channels - 1);
      m_monitor->init();
      m_monitor->setCaption(m_name);
      m_monitor->setIcon(dir + QString("monitor.xpm"));

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
  m_monitor->hide();
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
