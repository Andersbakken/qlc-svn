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

class QWorkspace;
class QApp;
class QMessageBox;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QPopupMenu;
class QPixmap;
class QToolButton;
class QFileDialog;
class QWidgetList;

class DeviceManagerView;
class VirtualConsole;
class Settings;
class SequenceTimer;
class SequenceProvider;
class GlobalFunctionsView;
class Doc;
class OutputPlugin;

///////////////////////////////////////////////////////////////////
// General program defines
#define IDS_APP_NAME_LONG               "Q Light Controller 2"
#define IDS_APP_NAME_SHORT              "QLC2"
#define IDS_APP_VERSION_STR             "Version 2.1.0-1"

#define MIN_LIGHT_VALUE                 0
#define MAX_LIGHT_VALUE                 100

#define MIN_DMX_VALUE                   0
#define MAX_DMX_VALUE                   255

#define OUTPUT_DEVICE_ROOT_ID           0
#define OUTPUT_DEVICE_ID_MIN            1
#define OUTPUT_DEVICE_ID_MAX            999

#define PLUGIN_ROOT_ID                  1000
#define PLUGIN_ID_MIN                   1001
#define PLUGIN_ID_MAX                   1999

#define BUS_ROOT_ID                     2000
#define BUS_ID_MIN                      2001
#define BUS_ID_MAX                      2999

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
  DeviceManagerView* deviceManagerView() { return m_dmView; }
  VirtualConsole* virtualConsole() { return m_virtualConsole; }
  QWorkspace* workspace() { return m_workspace; }
  Settings* settings() { return m_settings; }
  SequenceTimer* sequenceTimer() { return m_sequenceTimer; }
  SequenceProvider* sequenceProvider() { return m_sequenceProvider; }

  Doc* doc(void);

  void setSequenceTimer(SequenceTimer* timer);

 public slots:
  void slotFileNew();
  void slotFileOpen();
  void slotFileSave();
  void slotFileSaveAs();
  void slotFileSettings();
  void slotFileQuit();

  void slotPanic();

  void slotViewDeviceManager();
  void slotViewDeviceClassEditor();
  void slotDeviceManagerViewClosed();
  void slotViewVirtualConsole();
  void slotVirtualConsoleClosed();
  void slotViewToolBar();
  void slotViewStatusBar();
  void slotWindowCascade();
  void slotWindowTile();
  void slotRefreshWindowMenu();

  void slotViewGlobalFunctions();
  void slotGlobalFunctionsViewClosed();

  void slotHelpAbout();
  void slotHelpAboutQt();

 private:
  void initDoc();
  void initSettings();
  void initWorkspace();
  void initDeviceManagerView();
  void initVirtualConsole();
  void initSequenceEngine();

  void initMenuBar();
  void initStatusBar();
  void initToolBar();

 private slots:
  void slotWindowMenuCallback(int item);

 private:
  QPopupMenu* m_fileMenu;
  QPopupMenu* m_toolsMenu;
  QPopupMenu* m_windowMenu;
  QPopupMenu* m_helpMenu;

  QToolBar*   m_toolbar;

  Doc* m_doc;
  Settings* m_settings;
  DeviceManagerView* m_dmView;
  VirtualConsole* m_virtualConsole;
  QWorkspace* m_workspace;
  GlobalFunctionsView* m_globalFunctionsView;

  SequenceProvider* m_sequenceProvider;
  SequenceTimer* m_sequenceTimer;

 protected:
  void closeEvent(QCloseEvent*);
  bool event(QEvent* e);
};

#endif

