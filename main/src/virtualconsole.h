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

class VCFrame;
class KeyBind;
class Bus;
class VCDockArea;
class FloatingEdit;

const QString KEY_VIRTUAL_CONSOLE_OPEN      (         "VirtualConsoleOpen" );
const QString KEY_VIRTUAL_CONSOLE_GRABKB    ( "VirtualConsoleGrabKeyboard" );
const QString KEY_VIRTUAL_CONSOLE_KEYREPEAT (    "VirtualConsoleKeyRepeat" );
const QString KEY_VIRTUAL_CONSOLE_SNAPGRID  (   "VirtualConsoleSnapToGrid" );
const QString KEY_VIRTUAL_CONSOLE_GRIDX     (        "VirtualConsoleGridX" );
const QString KEY_VIRTUAL_CONSOLE_GRIDY     (        "VirtualConsoleGridY" );

// Menu stuff
const int KVCMenuEvent  ( 2000 );

// Add Menu >>>
const int KVCMenuAddMin           ( 100 );
const int KVCMenuAddMax           ( 199 );
const int KVCMenuAddButton        ( 101 );
const int KVCMenuAddSlider        ( 102 );
const int KVCMenuAddFrame         ( 103 );
const int KVCMenuAddLabel         ( 104 );
// <<< Add Menu

// Tools Menu >>>
const int KVCMenuToolsMin            ( 200 );
const int KVCMenuToolsMax            ( 299 );
const int KVCMenuToolsSettings       ( 200 );
const int KVCMenuToolsDefaultSliders ( 201 );
const int KVCMenuToolsPanic          ( 202 );
// <<< Tools Menu

// Widget Menu >>>
const int KVCMenuWidgetMin              ( 300 );
const int KVCMenuWidgetMax              ( 399 );
const int KVCMenuWidgetProperties       ( 300 );
const int KVCMenuWidgetRename           ( 301 );

const int KVCMenuWidgetFont             ( 302 );
const int KVCMenuWidgetForegroundColor  ( 303 );
const int KVCMenuWidgetForegroundNone   ( 304 );

const int KVCMenuWidgetBackgroundNone   ( 305 );
const int KVCMenuWidgetBackgroundColor  ( 306 );
const int KVCMenuWidgetBackgroundPixmap ( 307 );

const int KVCMenuWidgetRemove           ( 308 );
const int KVCMenuWidgetCopy             ( 309 );

const int KVCMenuWidgetStack            ( 310 );
const int KVCMenuWidgetStackRaise       ( 311 );
const int KVCMenuWidgetStackLower       ( 312 );

const int KVCMenuWidgetFunction         ( 313 );
const int KVCMenuWidgetFunctionAttach   ( 314 );
const int KVCMenuWidgetFunctionDetach   ( 315 );

const int KVCMenuWidgetDrawFrame        ( 316 );
// <<< Widget Menu

class VCMenuEvent : public QCustomEvent
{
 public:
  VCMenuEvent( int item )
    : QCustomEvent( KVCMenuEvent ),
    m_menuItem( item ) 
    {
    }
  
  int menuItem() { return m_menuItem; }

 private:
  int m_menuItem;
};


class VirtualConsole : public QWidget
{
  Q_OBJECT

 public:
  VirtualConsole(QWidget* parent = 0, const char* name = 0);
  ~VirtualConsole();

  void initView();
  void initMenuBar();
  void initDockArea();
  void initDrawArea();

  // Create the vc from list
  void createContents(QPtrList <QString>& file); 

  // Save all widgets and vc data to file
  void saveToFile(QFile& file); 

  // Used to get a correct parent frame for widgets
  VCFrame* getFrame(unsigned int id, VCFrame* widget = NULL);

  bool isGridEnabled() { return m_gridEnabled; }
  int gridX() { return m_gridX; }
  int gridY() { return m_gridY; }

  QWidget* selectedWidget() { return m_selectedWidget; }
  void setSelectedWidget(QWidget*);

  QPopupMenu* widgetMenu() { return m_widgetMenu; }
  QPopupMenu* addMenu() { return m_addMenu; }

 public slots:
  void slotMenuItemActivated(int);
  void slotDockAreaHidden(bool);
  void slotModeChanged();
  void slotRenameReturnPressed();

 signals:
  void closed();

  void keyPressed(QKeyEvent*);
  void keyReleased(QKeyEvent*);

 protected:
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

 private:
  void createVirtualConsole(QPtrList <QString>& list);
  void createWidget(QPtrList <QString> &list);

  void parseAddMenu(int);
  void parseToolsMenu(int);
  void parseWidgetMenu(int);

 private:
  // Virtual console menu bar
  QMenuBar* m_menuBar;
  QPopupMenu* m_widgetMenu;
  QPopupMenu* m_addMenu;

  // Master layout
  QHBoxLayout* m_layout; 

  // Dock area
  VCDockArea* m_dockArea;

  // Draw area
  VCFrame* m_drawArea;

  // Key receiver bind objects
  QPtrList <KeyBind> m_keyReceivers;

  // Grid
  bool m_gridEnabled;
  int m_gridX;
  int m_gridY;

  // Currently selected widget
  QWidget* m_selectedWidget;

  // Rename edit
  FloatingEdit* m_renameEdit;
};

#endif
