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

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include "dmxwidgetbase.h"
#include "keybind.h"
#include "classes.h"

#define ID_VC_MODE                   100
#define ID_VC_MODE_OPERATE           110
#define ID_VC_MODE_DESIGN            120

#define ID_VC_ADD                    1000
#define ID_VC_ADD_BUTTON             1010
#define ID_VC_ADD_SLIDER             1020
#define ID_VC_ADD_SPEEDSLIDER        1030
#define ID_VC_ADD_MONITOR            1040
#define ID_VC_ADD_FRAME              1050

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

  // Used to get a correct parent frame for widgets
  DMXWidget* getFrame(unsigned int id, DMXWidget* widget = NULL);

  bool isDesignMode();
  bool isObjectPointMode();

  SpeedSlider* defaultSpeedSlider() { return m_defaultSpeedSlider; }
  void setDefaultSpeedSlider(SpeedSlider* s);

  void saveToFile(QFile& file);

  void createWidget(QList<QString> &list);

  DMXWidget* drawArea() { return m_drawArea; }

 public slots:
  void slotMenuItemActivated(int item);
  void slotDrawAreaRemoved(DMXWidget* widget);

 signals:
  void closed();
  void modeChange(Mode m);

 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);
  void addBottomFrame();
  DMXWidgetBase* searchKeyReceiver(DMXWidgetBase* widget);

 private:
  QMenuBar* m_menuBar;
  DMXWidget* m_drawArea;
  QVBoxLayout* m_layout;

  SpeedSlider* m_defaultSpeedSlider;

  bool m_designMode;
  bool m_objectPointMode;

  QWidget* m_sourceObject;
  QWidget* m_targetObject;

  QList <DMXWidgetBase> m_keyReceivers;
};

#endif
