/*
  Q Light Controller
  devicemanagerview.cpp
  
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

#include "devicemanagerview.h"
#include "devicelistview.h"
#include "settings.h"
#include "app.h"
#include "doc.h"
#include "configkeys.h"

#include <qwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qsize.h>
#include <qlabel.h>
#include <qdockarea.h>

extern App* _app;

DeviceManagerView::DeviceManagerView(QWidget* parent) : QWidget(parent)
{
  m_layout = NULL;
  m_dockArea = NULL;
  m_toolbar = NULL;
  m_dm = NULL;
}

void DeviceManagerView::initView()
{
  setCaption(QString("Device Manager"));
  resize(300, 200);

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(dir + QString("/device.xpm"));

  m_layout = new QVBoxLayout(this);

  m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
  m_dockArea->setFixedHeight(30);
  m_toolbar = new QToolBar("Device Manager", _app, m_dockArea);
  
  m_dm = new DeviceManager(this);

  m_addOutputDeviceButton = new QToolButton(QIconSet(QPixmap(dir + "/addoutputdevice.xpm")), "Add New Output Device", 0, m_dm, SLOT(slotDLAddOutputDevice()), m_toolbar);

  m_addBusButton = new QToolButton(QIconSet(QPixmap(dir + "/addbus.xpm")), "Add New Bus", 0, m_dm, SLOT(slotDLAddBus()), m_toolbar);

  m_removeButton = new QToolButton(QIconSet(QPixmap(dir + "/remove.xpm")), "Remove Current Selection", 0, m_dm, SLOT(slotDLRemove()), m_toolbar);

  m_toolbar->addSeparator();

  m_propertiesButton = new QToolButton(QIconSet(QPixmap(dir + "/settings.xpm")), "Properties", 0, m_dm, SLOT(slotDLViewProperties()), m_toolbar);

  m_toolbar->addSeparator();

  m_monitorButton = new QToolButton(QIconSet(QPixmap(dir + "/monitor.xpm")), "Monitor Device", 0, m_dm, SLOT(slotDLViewMonitor()), m_toolbar);

  m_consoleButton = new QToolButton(QIconSet(QPixmap(dir + "/console.xpm")), "View Console", 0, m_dm, SLOT(slotDLViewConsole()), m_toolbar);

  m_layout->addWidget(m_dockArea);
  m_layout->addWidget(m_dm);

  connect(_app->virtualConsole(), SIGNAL(modeChange(VirtualConsole::Mode)),
	  this, SLOT(slotModeChanged(VirtualConsole::Mode)));

  connect(m_dm, SIGNAL(selectionChanged(int, int)),
	  this, SLOT(slotSelectionChanged(int, int)));

  slotSelectionChanged(-1, -1);
}

DeviceManagerView::~DeviceManagerView()
{
  if (m_dm != NULL)
    {
      delete m_dm;
    }

  if (m_toolbar != NULL)
    {
      delete m_toolbar;
    }

  if (m_layout != NULL)
    {
      delete m_layout;
    }
}

void DeviceManagerView::resizeEvent(QResizeEvent* e)
{
  QSize size = e->size();
  size.setHeight(size.height() - m_toolbar->height());
  m_dm->resize(size);
}

void DeviceManagerView::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}

void DeviceManagerView::slotModeChanged(VirtualConsole::Mode m)
{
  if (m == VirtualConsole::Operate)
    {
      m_addOutputDeviceButton->setEnabled(false);
      m_addBusButton->setEnabled(false);
      m_removeButton->setEnabled(false);
      m_propertiesButton->setEnabled(false);
    }
  else
    {
      m_addOutputDeviceButton->setEnabled(true);
      m_addBusButton->setEnabled(true);
      m_removeButton->setEnabled(true);
      m_propertiesButton->setEnabled(true);
    }
}

void DeviceManagerView::slotSelectionChanged(int itemId, int itemType)
{
  if (itemType != KDLViewTypeDevice)
    {
      m_consoleButton->setEnabled(false);
      m_monitorButton->setEnabled(false);
    }
  else
    {
      m_consoleButton->setEnabled(true);
      m_monitorButton->setEnabled(true);
    }
}
