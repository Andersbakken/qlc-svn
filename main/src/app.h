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
#include "virtualconsole.h"

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
class QLabel;
class FunctionTree;

class DeviceManagerView;
class Settings;
class Doc;
class OutputPlugin;
class FunctionConsumer;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = "Version 2.2.1";

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
  Q_OBJECT
 public:
  App(Settings* settings);
  ~App();

 public:
  void initView(void);
  DeviceManagerView* deviceManagerView() { return m_dmView; }
  VirtualConsole* virtualConsole() { return m_virtualConsole; }
  QWorkspace* workspace() { return m_workspace; }
  Settings* settings() { return m_settings; }
  FunctionConsumer* functionConsumer() { return m_functionConsumer; }

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
  void slotViewFunctionTree();
  void slotFunctionTreeClosed();

  void slotWindowCascade();
  void slotWindowTile();

  void slotRefreshWindowMenu();
  void slotRefreshToolsMenu();

  void slotHelpAbout();
  void slotHelpAboutQt();

  void slotPanic();
  void slotModeButtonClicked();

  void slotSetModeIndicator(VirtualConsole::Mode mode);

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
  FunctionTree* m_functionTree;

  FunctionConsumer* m_functionConsumer;

  QLabel* m_modeIndicator;
  QToolButton* m_modeButton;

 protected:
  void closeEvent(QCloseEvent*);
  bool event(QEvent* e);
};

#endif

