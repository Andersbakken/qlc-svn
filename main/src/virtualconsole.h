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
class QVBoxLayout;
class QFile;

class VCWidgetBase;
class VCWidget;
class Bus;

#define ID_VC_MODE                   1000
#define ID_VC_MODE_OPERATE           1010
#define ID_VC_MODE_DESIGN            1020

#define ID_VC_ADD                    1100
#define ID_VC_ADD_BUTTON             1110
#define ID_VC_ADD_SLIDER             1120
#define ID_VC_ADD_SPEEDSLIDER        1130
#define ID_VC_ADD_MONITOR            1140
#define ID_VC_ADD_FRAME              1150
#define ID_VC_ADD_LABEL              1160

#define ID_VC_TOOLS                  1200
#define ID_VC_TOOLS_PANIC            1210

class VirtualConsole : public QWidget
{
  Q_OBJECT

 public:
  VirtualConsole(QWidget* parent = 0, const char* name = 0);
  ~VirtualConsole();

  enum Mode { Operate, Design };

  void initView(void);
  void newDocument();

  void registerKeyReceiver(VCWidgetBase* widget);
  void unRegisterKeyReceiver(VCWidgetBase* widget);

  bool isDesignMode();
  void setMode(Mode mode);

  void createContents(QPtrList <QString>& file); // Create the vc from list
  void saveToFile(QFile& file); // Save all widgets and vc data to file

  // Used to get a correct parent frame for widgets
  VCWidget* getFrame(unsigned int id, VCWidget* widget = NULL);
  VCWidget* drawArea() { return m_drawArea; }

  void setDefaultSpeedBus(Bus* bus);
  Bus* defaultSpeedBus() { return m_defaultSpeedBus; }

 public slots:
  void slotMenuItemActivated(int item);
  void slotDefaultSpeedBusDestroyed();

 signals:
  void closed();
  void modeChange(VirtualConsole::Mode);

 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

  VCWidgetBase* searchKeyReceiver(VCWidgetBase* widget);

 private:
  void createVirtualConsole(QPtrList <QString>& list);
  void createWidget(QPtrList <QString> &list);

 private:
  QMenuBar* m_menuBar;
  QPopupMenu* m_modeMenu;
  QPopupMenu* m_addMenu;
  QPopupMenu* m_toolsMenu;

  VCWidget* m_drawArea;
  QVBoxLayout* m_layout;

  Mode m_mode;

  QPtrList <VCWidgetBase> m_keyReceivers;

  Bus* m_defaultSpeedBus;
};

#endif
