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
#include "../../libs/common/outputplugin.h"

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
class VirtualConsole;
class Plugin;
class DummyOutPlugin;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = "Version 2.3";

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

  Doc* doc() { return m_doc; }

  //
  // General Plugin Stuff
  //
  QPtrList <Plugin> *pluginList() { return &m_pluginList; }
  Plugin* searchPlugin(QString name);
  Plugin* searchPlugin(QString name, Plugin::PluginType type);
  Plugin* searchPlugin(const t_plugin_id id);
  void addPlugin(Plugin*);
  void removePlugin(Plugin*);
  void initPlugins();
  bool probePlugin(QString path);
  
  //
  // The Output Plugin
  //
  OutputPlugin* outputPlugin() { return m_outputPlugin; }

 private slots:
  void slotChangeOutputPlugin(const QString& name);
  void slotPluginActivated(Plugin* plugin);

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

  void slotSetModeIndicator();

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

  void createJoystickContents(QPtrList <QString> &list);

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

  OutputPlugin* m_outputPlugin;
  DummyOutPlugin* m_dummyOutPlugin;
  QPtrList <Plugin> m_pluginList;

 protected:
  void closeEvent(QCloseEvent*);
  bool event(QEvent* e);

 private:
  static t_plugin_id NextPluginID;
};

#endif

