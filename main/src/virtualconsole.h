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
#include <qlist.h>

class QMenuBar;
class QPopupMenu;
class QToolBar;
class QVBoxLayout;
class QFile;

class DMXWidgetBase;
class DMXWidget;
class Bus;

#define ID_VC_MODE                   100
#define ID_VC_MODE_OPERATE           110
#define ID_VC_MODE_DESIGN            120

#define ID_VC_ADD                    1000
#define ID_VC_ADD_BUTTON             1010
#define ID_VC_ADD_SLIDER             1020
#define ID_VC_ADD_SPEEDSLIDER        1030
#define ID_VC_ADD_MONITOR            1040
#define ID_VC_ADD_FRAME              1050
#define ID_VC_ADD_LABEL              1060

class VirtualConsole : public QWidget
{
  Q_OBJECT

 public:
  VirtualConsole(QWidget* parent = 0, const char* name = 0);
  ~VirtualConsole();

  enum Mode { Operate, Design };

  void initView(void);
  void newDocument();

  void registerKeyReceiver(DMXWidgetBase* widget);
  void unRegisterKeyReceiver(DMXWidgetBase* widget);

  bool isDesignMode();

  void createContents(QList<QString>& file); // Create the virtual console from list
  void saveToFile(QFile& file); // Save all widgets and vc data to file

  // Used to get a correct parent frame for widgets
  DMXWidget* getFrame(unsigned int id, DMXWidget* widget = NULL);
  DMXWidget* drawArea() { return m_drawArea; }

  void setDefaultSpeedBus(Bus* bus);
  Bus* defaultSpeedBus() { return m_defaultSpeedBus; }

 public slots:
  void slotMenuItemActivated(int item);
  void slotDefaultSpeedBusDestroyed();

 signals:
  void closed();
  void modeChange(Mode m);

 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

  DMXWidgetBase* searchKeyReceiver(DMXWidgetBase* widget);

 private:
  void setMode(Mode mode);

  void createVirtualConsole(QList<QString>& list);
  void createWidget(QList<QString> &list);

 private:
  QMenuBar* m_menuBar;
  QPopupMenu* m_modeMenu;
  QPopupMenu* m_addMenu;
  DMXWidget* m_drawArea;
  QVBoxLayout* m_layout;

  Mode m_mode;

  QList <DMXWidgetBase> m_keyReceivers;

  Bus* m_defaultSpeedBus;
};

#endif
