/*
  Q Light Controller
  devicemanagerview.h
  
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

#ifndef DEVICEMANAGERVIEW_H
#define DEVICEMANAGERVIEW_H

#include "app.h"
#include "devicemanager.h"

class QWidget;
class QToolBar;
class QToolButton;
class QLayout;
class QPixmap;
class QEvent;
class QSize;
class QLabel;
class QDockArea;

class DeviceManagerView : public QWidget
{
  Q_OBJECT

 public:
  DeviceManagerView(QWidget* parent = 0);
  ~DeviceManagerView();

  DeviceManager* deviceManager() { return m_dm; }
  void initView(void);

 private slots:
  void slotModeChanged();
  void slotSelectionChanged(int itemId, int itemType);

 private:
  DeviceManager* m_dm;
  QToolBar* m_toolbar;
  QVBoxLayout* m_layout;
  QDockArea* m_dockArea;

  QToolButton* m_addOutputDeviceButton;
  QToolButton* m_addBusButton;
  QToolButton* m_removeButton;
  QToolButton* m_propertiesButton;
  QToolButton* m_monitorButton;
  QToolButton* m_consoleButton;

 signals:
  void closed();

 protected:
  void resizeEvent(QResizeEvent* e);
  void closeEvent(QCloseEvent*);
};

#endif
