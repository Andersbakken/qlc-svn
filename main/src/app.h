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
class DMXMap;

const QString KApplicationNameLong  = "Q Light Controller";
const QString KApplicationNameShort = "QLC";
const QString KApplicationVersion   = QString("Version ") + QString(VERSION);
const int KApplicationDefaultWidth  = 800;
const int KApplicationDefaultHeight = 600;

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
	void createFixtureManager();

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
	 * Output mapping
	 *********************************************************************/
 public:
	DMXMap* dmxMap() const { return m_dmxMap; }

 protected:
	void initDMXMap();

 protected slots:
	void slotViewOutputManager();

 protected:
	DMXMap* m_dmxMap;

	/*********************************************************************
	 * Input mapping
	 *********************************************************************/
 public:

 protected:

 protected slots:
	void slotViewInputManager();

 protected:

	/*********************************************************************
	 * Blackout
	 *********************************************************************/
 public slots:
	/**
	 * Stop all running functions
	 */
	void slotPanic();

	/**
	 * Toggle blackout on/off
	 */
	void slotToggleBlackout();

	/**
	 * Set blackout state
	 *
	 * @param state true to set blackout ON, false for OFF
	 */
	void slotSetBlackout(bool state);

 protected slots:
	void slotFlashBlackoutIndicator();

 protected:
	/** Flashing blackout indicator on the status bar */
	QLabel* m_blackoutIndicator;

	/** Periodic timer object for the flashing indicator */
	QTimer* m_blackoutIndicatorTimer;

	/*********************************************************************
	 * Buses
	 *********************************************************************/
 public slots:
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
	void slotFileQuit();
	
 protected:
	QPopupMenu* m_fileMenu;
	QPopupMenu* m_managerMenu;
	QPopupMenu* m_controlMenu;
	QPopupMenu* m_modeMenu;
	QPopupMenu* m_windowMenu;
	QPopupMenu* m_helpMenu;
	
	QToolBar* m_toolbar;
	QToolButton* m_newToolButton;
	QToolButton* m_openToolButton;
	QToolButton* m_saveToolButton;
	QToolButton* m_fixtureManagerToolButton;
	QToolButton* m_virtualConsoleToolButton;
	QToolButton* m_functionManagerToolButton;
	QToolButton* m_panicToolButton;
	QToolButton* m_modeToolButton;
	QToolButton* m_blackoutToolButton;
	QToolButton* m_monitorToolButton;
};

#endif

