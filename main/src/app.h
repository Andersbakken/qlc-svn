/*
  Q Light Controller
  app.h
  
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

#ifndef APP_H
#define APP_H

#include <qmainwindow.h>
#include <qworkspace.h>
#include <qapp.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qwidgetlist.h>

#include "classes.h"

///////////////////////////////////////////////////////////////////
// General program defines
#define IDS_APP_NAME_LONG               "Q Light Controller 2"
#define IDS_APP_NAME_SHORT              "QLC2"

#define MIN_LIGHT_VALUE                 0
#define MAX_LIGHT_VALUE                 100

#define MIN_DMX_VALUE                   0
#define MAX_DMX_VALUE                   255

#define OUTPUT_DEVICE_ROOT_ID           0
#define OUTPUT_DEVICE_ID_MIN            1
#define OUTPUT_DEVICE_ID_MAX            999

#define INPUT_DEVICE_ROOT_ID            1000
#define INPUT_DEVICE_ID_MIN             1001
#define INPUT_DEVICE_ID_MAX             1999

#define BUS_ROOT_ID                     2000
#define BUS_ID_MIN                      2001
#define BUS_ID_MAX                      2999

///////////////////////////////////////////////////////////////////
// File menu entries
#define ID_FILE_NEW                 	10020
#define ID_FILE_OPEN                	10030
#define ID_FILE_SAVE                	10050
#define ID_FILE_SAVE_AS             	10060
#define ID_FILE_CLOSE               	10070
#define ID_FILE_PRINT               	10080
#define ID_FILE_SETTINGS                10090
#define ID_FILE_QUIT                	10100

///////////////////////////////////////////////////////////////////
// View menu entries                    
#define ID_VIEW_TOOLBAR        	        12010
#define ID_VIEW_STATUSBAR		12020

#define ID_VIEW_DEVICE_MANAGER		12030
#define ID_VIEW_DEVICE_CLASS_EDITOR	12035
#define ID_VIEW_VIRTUAL_CONSOLE         12040
#define ID_VIEW_SEQUENCE_EDITOR         12050
#define ID_VIEW_DMXADDRESSTOOL          12060
#define ID_VIEW_INPUT_DEVICES           12070

///////////////////////////////////////////////////////////////////
// Functions menu entries
#define ID_VIEW_GLOBAL_FUNCTIONS        13010

///////////////////////////////////////////////////////////////////
// Window menu entries
#define ID_WINDOW_MENU			14000
#define ID_WINDOW_CASCADE		14010
#define ID_WINDOW_TILE			14020

///////////////////////////////////////////////////////////////////
// Help menu entries
#define ID_HELP_ABOUT               	1002

//////////////////////////////////////////////////////////////////
// Status bar messages
#define IDS_STATUS_DEFAULT              "Ready"

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
  Q_OBJECT
 public:
  App();
  ~App();

 public:
  void initView(void);
  DeviceManagerView* deviceManagerView() const { return m_dmView; }
  VirtualConsole* virtualConsole() const { return m_virtualConsole; }
  QWorkspace* workspace() const { return m_workspace; }
  Settings* settings() const { return m_settings; }
  SequenceTimer* sequenceTimer() const { return m_sequenceTimer; }
  SequenceProvider* sequenceProvider() const { return m_sequenceProvider; }
  GlobalFunctionsView* globalFunctionsView() const { return m_globalFunctionsView; }
  InputDeviceView* inputDeviceView() const { return m_inputDeviceView; }

  Doc* doc(void);

 public slots:
  void slotFileNew();
  void slotFileOpen();
  void slotFileSave();
  void slotFileSaveAs();
  void slotFileSettings();
  void slotFileQuit();

  void slotViewDeviceManager();
  void slotViewDeviceClassEditor();
  void slotDeviceManagerViewClosed();
  void slotViewVirtualConsole();
  void slotVirtualConsoleClosed();
  void slotViewSequenceEditor();
  void slotSequenceEditorClosed();
  void slotViewToolBar();
  void slotViewStatusBar();
  void slotViewDMXAddressTool();
  void slotWindowCascade();
  void slotWindowTile();
  void slotRefreshWindowMenu();

  void slotViewGlobalFunctions();
  void slotGlobalFunctionsViewClosed();

  void slotViewInputDevices();
  void slotInputDeviceViewClosed();

  void slotHelpAbout();

 private:
  void initDoc();
  void initSettings();
  void initWorkspace();
  void initDeviceManagerView();
  void initVirtualConsole();
  void initSequenceEditor();
  void initSequenceEngine();

  void initMenuBar();
  void initStatusBar();
  void initToolBar();

 private slots:
  void slotWindowMenuCallback(int item);

 private:
  QPopupMenu* m_fileMenu;
  QPopupMenu* m_viewMenu;
  QPopupMenu* m_functionsMenu;
  QPopupMenu* m_windowMenu;
  QPopupMenu* m_helpMenu;

  QToolBar*   m_toolbar;

  Doc* m_doc;
  Settings* m_settings;
  DeviceManagerView* m_dmView;
  VirtualConsole* m_virtualConsole;
  QWorkspace* m_workspace;
  GlobalFunctionsView* m_globalFunctionsView;
  InputDeviceView* m_inputDeviceView;

  QPixmap m_bgXpm;

  SequenceProvider* m_sequenceProvider;
  SequenceTimer* m_sequenceTimer;

 protected:
  void closeEvent(QCloseEvent*);
  bool event(QEvent* e);
};

#endif

