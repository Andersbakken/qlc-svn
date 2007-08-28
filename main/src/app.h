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
#include <qstring.h>
#include <qptrlist.h>
#include "common/inputplugin.h"
#include "common/outputplugin.h"

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
class QTimer;

class FixtureManager;
class Settings;
class Doc;
class InputPlugin;
class OutputPlugin;
class FunctionConsumer;
class VirtualConsole;
class Plugin;
class DummyInPlugin;
class DummyOutPlugin;
class DeviceClass;
class FunctionManager;
class BusProperties;
class DocumentBrowser;
class PluginManager;
class Monitor;
class QLCFixtureDef;
class QLCWorkspace;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = QString("Version ") + QString(VERSION);

//////////////////////////////////////////////////////////////////
// Class definition

class App : public QMainWindow
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
 public:
	App();
	~App();

 public:
	void init();

	/*********************************************************************
	 * Doc
	 *********************************************************************/
 public:
	Doc* doc() { return m_doc; }
	void newDocument();

 protected slots:
	void slotDocModified(bool state);

 protected:
	void initDoc();

 protected:
	Doc* m_doc;
	
	/*********************************************************************
	 * Workspace
	 *********************************************************************/
 public:
	QLCWorkspace* workspace() { return m_workspace; }

 public slots:
	void slotWindowCascade();
	void slotWindowTile();

 protected slots:
	void slotBackgroundChanged(const QString&);
	void slotWindowMenuCallback(int item);
	void slotRefreshMenus();

 protected:
	void initWorkspace();
	void closeEvent(QCloseEvent*);

 protected:
	QLCWorkspace* m_workspace;
	
	/*********************************************************************
	 * Fixture Manager
	 *********************************************************************/
 public:
	FixtureManager* fixtureManager() { return m_fixtureManager; }

 public slots:
	void slotViewFixtureManager();
	void slotFixtureManagerClosed();

 protected:
	FixtureManager* m_fixtureManager;
	
	/*********************************************************************
	 * Function Manager
	 *********************************************************************/
 public:
	FunctionManager* functionManager() { return m_functionManager; }

 public slots:
	void slotViewFunctionManager();
	void slotFunctionManagerClosed();

 protected:
	FunctionManager* m_functionManager;
	
	/*********************************************************************
	 * Virtual Console
	 *********************************************************************/
 public:
	VirtualConsole* virtualConsole() { return m_virtualConsole; }

 public slots:
	void slotViewVirtualConsole();
	void slotVirtualConsoleClosed();

 protected:
	void initVirtualConsole();

 protected:
	VirtualConsole* m_virtualConsole;

	/*********************************************************************
	 * Function Consumer
	 *********************************************************************/
 public:
	FunctionConsumer* functionConsumer() { return m_functionConsumer; }

 protected:
	void initFunctionConsumer();
  
 protected:
	FunctionConsumer* m_functionConsumer;

	/*********************************************************************
	 * Monitor
	 *********************************************************************/
 public:
	Monitor* monitor() { return m_monitor; }
	void createMonitor();

 public slots:
	void slotViewMonitor();
	void slotMonitorClosed();

 protected:
	Monitor* m_monitor;

	/*********************************************************************
	 * Settings
	 *********************************************************************/
 public:
	Settings* settings() { return m_settings; }
	void saveSettings();

 protected:
	void initSettings();

 protected:
	Settings* m_settings;

	/*********************************************************************
	 * Plugins
	 *********************************************************************/
 public:
	InputPlugin* inputPlugin() { return m_inputPlugin; }
	OutputPlugin* outputPlugin() { return m_outputPlugin; }
	QPtrList <Plugin> *pluginList() { return &m_pluginList; }
	Plugin* searchPlugin(QString name);
	Plugin* searchPlugin(QString name, Plugin::PluginType type);
	Plugin* searchPlugin(const t_plugin_id id);
	void addPlugin(Plugin*);
	void removePlugin(Plugin*);
	void initPlugins();
	bool probePlugin(QString path);

 protected slots:
	void slotChangeInputPlugin(const QString& name);
	void slotChangeOutputPlugin(const QString& name);
	void slotPluginActivated(Plugin* plugin);

 protected:
	PluginManager* m_pluginManager;

	InputPlugin* m_inputPlugin;
	OutputPlugin* m_outputPlugin;
	DummyInPlugin* m_dummyInPlugin;
	DummyOutPlugin* m_dummyOutPlugin;
	QPtrList <Plugin> m_pluginList;
	
	static t_plugin_id NextPluginID;

	/*********************************************************************
	 * Fixture definitions
	 *********************************************************************/
 public:
	/** Load all fixture definitions */
	bool loadFixtureDefinitions();

	/** Get a fixture definition by its manufacturer & model */
	QLCFixtureDef* fixtureDef(const QString& manufacturer, const QString& model);

	/** Get a list of fixture definitions */
	QPtrList <QLCFixtureDef> *fixtureDefList() { return &m_fixtureDefList; }

 protected:
	/** List of fixture definitions */
	QPtrList <QLCFixtureDef> m_fixtureDefList;

	/*********************************************************************
	 * Mode: operate or design
	 *********************************************************************/
 public:
	enum Mode { Operate, Design };
	bool mode() { return m_mode; }

 public slots:
	void slotSetDesignMode();
	void slotSetOperateMode();
	void slotSetMode();

 signals:
	void modeChanged();

 protected:
	/** Main operating mode */
	Mode m_mode;

	/** Mode indicator on the status bar */
	QLabel* m_modeIndicator;

	/*********************************************************************
	 * Value read & write
	 *********************************************************************/
 public:
	void initValues();
	void setValue(t_channel, t_value);
	t_value value(t_channel);
	void valueRange(t_channel address, t_value* values, t_channel num);
	bool isBlackOut() { return m_blackOut; }

 public slots:
	void slotPanic();
	void slotToggleBlackOut();

 protected slots:
	void slotFlashBlackOutIndicator();

 protected:
	/** Blackout status */
	bool m_blackOut;

	/** Flashing indicator on the status bar */
	QLabel* m_blackOutIndicator;

	/** Timer object for the flashing indicator */
	QTimer* m_blackOutIndicatorTimer;

	/*********************************************************************
	 * Submasters
	 *********************************************************************/
 public:
	void initSubmasters();
	bool assignSubmaster(t_channel);
	bool resignSubmaster(t_channel);
	int hasSubmaster(t_channel);
	void resetSubmasters();
	void setSubmasterValue(t_channel, int);
	float submasterValue(t_channel);

	/** DMX value buffer */
	t_value m_values[KChannelMax];

	/** Submaster value buffer */
	float m_submasterValues[KChannelMax];

	/** Submasters */
	int m_submasters[KChannelMax];
	
	/*********************************************************************
	 * Buses
	 *********************************************************************/
 public:
	void slotViewBusProperties();
	void slotBusPropertiesClosed();

 protected:
	BusProperties* m_busProperties;

	/*********************************************************************
	 * Help & About
	 *********************************************************************/	
 public slots:
	void slotHelpIndex();
	void slotDocumentBrowserClosed();
	
	void slotHelpAbout();
	void slotHelpAboutQt();
	void slotHelpTooltips();

 protected:
	DocumentBrowser* m_documentBrowser;

	/*********************************************************************
	 * Menus & toolbars
	 *********************************************************************/	
 protected:
	void initMenuBar();
	void initStatusBar();
	void initToolBar();

 public slots:
	bool slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileSettings();
	void slotFilePlugins();
	void slotPluginManagerClosed();
	void slotFileQuit();
	
 protected:
	QPopupMenu* m_fileMenu;
	QPopupMenu* m_toolsMenu;
	QPopupMenu* m_modeMenu;
	QPopupMenu* m_windowMenu;
	QPopupMenu* m_helpMenu;
	
	QToolBar* m_toolbar;
	QToolButton* m_newTB;
	QToolButton* m_openTB;
	QToolButton* m_saveTB;
	QToolButton* m_dmTB;
	QToolButton* m_vcTB;
	QToolButton* m_ftTB;
	QToolButton* m_panicTB;
	QToolButton* m_modeTB;
	QToolButton* m_blackOutTB;
	QToolButton* m_monitorTB;
};

#endif

