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
class DeviceClass;
class FunctionTree;
class BusProperties;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = "Version 2.3";

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
  Q_OBJECT
 public:
  App();
  ~App();

 public:
  void init(void);
  DeviceManagerView* deviceManagerView() { return m_dmView; }
  VirtualConsole* virtualConsole() { return m_virtualConsole; }
  QWorkspace* workspace() { return m_workspace; }
  Settings* settings() { return m_settings; }
  FunctionConsumer* functionConsumer() { return m_functionConsumer; }
  Doc* doc() { return m_doc; }

  //
  // Plugin Stuff
  //
  OutputPlugin* outputPlugin() { return m_outputPlugin; }
  QPtrList <Plugin> *pluginList() { return &m_pluginList; }
  Plugin* searchPlugin(QString name);
  Plugin* searchPlugin(QString name, Plugin::PluginType type);
  Plugin* searchPlugin(const t_plugin_id id);
  void addPlugin(Plugin*);
  void removePlugin(Plugin*);
  void initPlugins();
  bool probePlugin(QString path);

  //
  // Device classe stuff
  //
  QPtrList <DeviceClass> *deviceClassList() { return &m_deviceClassList; }
  DeviceClass* searchDeviceClass(const QString &manufacturer,
				 const QString &model);
  DeviceClass* searchDeviceClass(const t_deviceclass_id id);

  //
  // Mode; operate or design
  //
  enum Mode { Operate, Design };
  bool mode() { return m_mode; }
  void setMode(Mode mode);

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
  void slotDeviceManagerViewClosed();

  void slotViewVirtualConsole();
  void slotVirtualConsoleClosed();

  void slotViewFunctionTree();
  void slotFunctionTreeClosed();

  void slotViewBusProperties();
  void slotBusPropertiesClosed();

  void slotWindowCascade();
  void slotWindowTile();

  void slotHelpAbout();
  void slotHelpAboutQt();

  void slotPanic();
  void slotSetMode();

 signals:
  void modeChanged();
  
 private:
  void initSettings();
  void initDeviceClasses();
  void initDoc();
  void initWorkspace();
  void initDeviceManagerView();
  void initVirtualConsole();
  void initFunctionConsumer();

  void initMenuBar();
  void initStatusBar();
  void initToolBar();

  void saveSettings();

  DeviceClass* createDeviceClass(QPtrList <QString> &list);
  void createJoystickContents(QPtrList <QString> &list);

 private slots:
  void slotWindowMenuCallback(int item);
  void slotRefreshMenus();

 private:
  QPopupMenu* m_fileMenu;
  QPopupMenu* m_toolsMenu;
  QPopupMenu* m_modeMenu;
  QPopupMenu* m_windowMenu;
  QPopupMenu* m_helpMenu;

  QToolBar*   m_toolbar;

  Doc* m_doc;
  Settings* m_settings;
  DeviceManagerView* m_dmView;
  VirtualConsole* m_virtualConsole;
  QWorkspace* m_workspace;
  FunctionTree* m_functionTree;
  BusProperties* m_busProperties;

  FunctionConsumer* m_functionConsumer;

  QLabel* m_modeIndicator;
  QToolButton* m_modeButton;

  OutputPlugin* m_outputPlugin;
  DummyOutPlugin* m_dummyOutPlugin;
  QPtrList <Plugin> m_pluginList;

  // Main operating mode
  Mode m_mode;

  // Device classes
  QPtrList <DeviceClass> m_deviceClassList;

 protected:
  void closeEvent(QCloseEvent*);

 private:
  static t_plugin_id NextPluginID;
};

#endif

