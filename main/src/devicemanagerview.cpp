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
#include "settings.h"

#include <qsize.h>

extern App* _app;

DeviceManagerView::DeviceManagerView(QWidget* parent, const char* name) : QWidget(parent, name)
{
  m_layout = NULL;
  m_toolbar = NULL;
  m_dm = NULL;
}

void DeviceManagerView::initView()
{
  setCaption(QString("Device Manager"));
  resize(300, 200);
  setIcon(_app->settings()->getPixmapPath() + QString("device.xpm"));

  m_layout = new QVBoxLayout(this);
  m_toolbar = new QToolBar("Device Manager Toolbar", _app, this);
  m_dm = new DeviceManager(this);

  // QT3 fixes : doj 2001-01-28
#if QT_VERSION >= 300
  new QToolButton(QIconSet(QPixmap(_app->settings()->getPixmapPath() + "add.xpm")), "Add new device", 0, m_dm, SLOT(slotDLAddDevice()), m_toolbar);
  new QToolButton(QIconSet(QPixmap(_app->settings()->getPixmapPath() + "remove.xpm")), "Remove current device", 0, m_dm, SLOT(slotDLRemoveDevice()), m_toolbar);
  new QToolButton(QIconSet(QPixmap(_app->settings()->getPixmapPath() + "monitor.xpm")), "Monitor device channels", 0, m_dm, SLOT(slotDLViewMonitor()), m_toolbar);
  new QToolButton(QIconSet(QPixmap(_app->settings()->getPixmapPath() + "console.xpm")), "Device DMX Console", 0, m_dm, SLOT(slotDLViewConsole()), m_toolbar);
  new QToolButton(QIconSet(QPixmap(_app->settings()->getPixmapPath() + "function.xpm")), "Functions", 0, m_dm, SLOT(slotDLViewFunctions()), m_toolbar);
#else
  new QToolButton(_app->settings()->getPixmapPath() + QString("add.xpm"), "Add new device", 0, m_dm, SLOT(slotDLAddDevice()), m_toolbar);
  new QToolButton(_app->settings()->getPixmapPath() + QString("remove.xpm"), "Remove current device", 0, m_dm, SLOT(slotDLRemoveDevice()), m_toolbar);
  new QToolButton(_app->settings()->getPixmapPath() + QString("monitor.xpm"), "Monitor device channels", 0, m_dm, SLOT(slotDLViewMonitor()), m_toolbar);
  new QToolButton(_app->settings()->getPixmapPath() + QString("console.xpm"), "Device DMX Console", 0, m_dm, SLOT(slotDLViewConsole()), m_toolbar);
  new QToolButton(_app->settings()->getPixmapPath() + QString("function.xpm"), "Functions", 0, m_dm, SLOT(slotDLViewFunctions()), m_toolbar);
#endif

  m_layout->addWidget(m_toolbar);
  m_layout->addWidget(m_dm);
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
