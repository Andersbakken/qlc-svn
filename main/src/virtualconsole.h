/*
  Q Light Controller
  virtualconsole.h
  
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

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <qwidget.h>
#include <qptrlist.h>

class QMenuBar;
class QPopupMenu;
class QToolBar;
class QHBoxLayout;
class QFile;
class QFrame;

class VCWidgetBase;
class VCWidget;
class Bus;
class VCDockArea;

#define ID_VC_ADD                    1100
#define ID_VC_ADD_BUTTON             1110
#define ID_VC_ADD_SLIDER             1120
#define ID_VC_ADD_SPEEDSLIDER        1130
#define ID_VC_ADD_MONITOR            1140
#define ID_VC_ADD_FRAME              1150
#define ID_VC_ADD_LABEL              1160

#define ID_VC_TOOLS                  1200
#define ID_VC_TOOLS_PANIC            1210
#define ID_VC_TOOLS_SLIDERS          1220

class VirtualConsole : public QWidget
{
  Q_OBJECT

 public:
  VirtualConsole(QWidget* parent = 0, const char* name = 0);
  ~VirtualConsole();

  void initView();
  void initDockArea();
  void initDrawArea();

  void registerKeyReceiver(VCWidgetBase* widget);
  void unRegisterKeyReceiver(VCWidgetBase* widget);

  // Create the vc from list
  void createContents(QPtrList <QString>& file); 

  // Save all widgets and vc data to file
  void saveToFile(QFile& file); 

  // Used to get a correct parent frame for widgets
  VCWidget* getFrame(unsigned int id, VCWidget* widget = NULL);

 public slots:
  void slotMenuItemActivated(int);
  void slotDockAreaHidden(bool);
  void slotModeChanged();

 signals:
  void closed();

 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

  VCWidgetBase* searchKeyReceiver(VCWidgetBase* widget);

 private:
  void createVirtualConsole(QPtrList <QString>& list);
  void createWidget(QPtrList <QString> &list);

 private:
  // Virtual console menus
  QMenuBar* m_menuBar;
  QPopupMenu* m_modeMenu;
  QPopupMenu* m_addMenu;
  QPopupMenu* m_toolsMenu;

  // Master layout
  QHBoxLayout* m_layout; 

  // Dock area
  VCDockArea* m_dockArea;

  // Draw area
  VCWidget* m_drawArea;

  // Those widgets that have been registered as keyboard event receivers
  QPtrList <VCWidgetBase> m_keyReceivers;
};

#endif
