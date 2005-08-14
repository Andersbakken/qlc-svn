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

#include "device.h"
#include "function.h"

#include <qwidget.h>

class QToolBar;
class QToolButton;
class QVBoxLayout;
class QCloseEvent;
class QDockArea;
class QSplitter;
class QListView;
class QListViewItem;
class QTextView;

class DeviceManagerView : public QWidget
{
  Q_OBJECT

 public:
  DeviceManagerView(QWidget* parent, const char* name = 0);
  ~DeviceManagerView();

  void initView();

 public slots:
  void slotUpdate();

 private:
  void initTitle();
  void initToolBar();
  void initDataView();
  
  void copyFunction(Function* function, Device* device);

 private slots:
  void slotAdd();
  void slotRemove();
  void slotClone();
  void slotDoubleClicked(QListViewItem*);
  void slotProperties();
  void slotMonitor();
  void slotConsole();
  void slotAutoFunction();

  void slotModeChanged();
  void slotSelectionChanged(QListViewItem*);
  void slotRightButtonClicked(QListViewItem*, const QPoint&, int);
  void slotMenuCallBack(int);

 private:
  QVBoxLayout* m_layout;
  QToolBar* m_toolbar;
  QDockArea* m_dockArea;
  QSplitter* m_splitter;
  QListView* m_listView;
  QTextView* m_textView;

  QToolButton* m_addButton;
  QToolButton* m_cloneButton;
  QToolButton* m_removeButton;
  QToolButton* m_propertiesButton;
  QToolButton* m_consoleButton;
  QToolButton* m_monitorButton;

 signals:
  void closed();

 protected:
  void closeEvent(QCloseEvent*);
};

#endif
