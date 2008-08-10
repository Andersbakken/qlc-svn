/*
  Q Light Controller
  app.h

  Copyright (c) Heikki Junnila

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

#include <QMainWindow>
#include <QString>
#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlcoutplugin.h"

class QMessageBox;
class QToolButton;
class QFileDialog;
class QStatusBar;
class QMenuBar;
class QToolBar;
class QPixmap;
class QAction;
class QLabel;
class QTimer;
class QMenu;

class FunctionConsumer;
class FunctionManager;
class FixtureManager;
class VirtualConsole;
class DummyOutPlugin;
class DummyInPlugin;
class PluginManager;
class QLCDocBrowser;
class BusProperties;
class QLCFixtureDef;
class QLCInPlugin;
class QLCPlugin;
class InputMap;
class Monitor;
class DMXMap;
class Doc;
class App;

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

protected:
	void init();
	void closeEvent(QCloseEvent*);

	/*********************************************************************
	 * Output mapping
	 *********************************************************************/
public:
	DMXMap* dmxMap() const { return m_dmxMap; }

protected:
	void initDMXMap();

protected slots:
	void slotDMXMapBlackoutChanged(bool state);
	void slotFlashBlackoutIndicator();

protected:
	DMXMap* m_dmxMap;

	/*********************************************************************
	 * Input mapping
	 *********************************************************************/
public:
	InputMap* inputMap() { return m_inputMap; }

protected:
	void initInputMap();

protected:
	InputMap* m_inputMap;

	/*********************************************************************
	 * Function Consumer
	 *********************************************************************/
public:
	/** Get a pointer to the function runner object */
	FunctionConsumer* functionConsumer() { return m_functionConsumer; }

protected:
	/** Initialize the function runner object */
	void initFunctionConsumer();

protected:
	/** The function runner object */
	FunctionConsumer* m_functionConsumer;

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
	 * Fixture definitions
	 *********************************************************************/
public:
	/** Load all fixture definitions */
	bool initFixtureDefinitions();

	/** Get a fixture definition by its manufacturer & model */
	QLCFixtureDef* fixtureDef(const QString& manufacturer,
				  const QString& model);

	/** Get a list of fixture definitions */
	QList <QLCFixtureDef*> *fixtureDefList() { return &m_fixtureDefList; }

protected:
	/** List of fixture definitions */
	QList <QLCFixtureDef*> m_fixtureDefList;

	/*********************************************************************
	 * Mode: operate or design
	 *********************************************************************/
public:
	enum Mode { Operate, Design };
	Mode mode() { return m_mode; }

signals:
	void modeChanged(App::Mode mode);

public slots:
	void slotModeOperate();
	void slotModeDesign();

protected:
	/** Main operating mode */
	Mode m_mode;

	/*********************************************************************
	 * Fixture Manager
	 *********************************************************************/
public:
	FixtureManager* fixtureManager() const { return m_fixtureManager; }

protected:
	FixtureManager* m_fixtureManager;
	
	/*********************************************************************
	 * Function Manager
	 *********************************************************************/
public:
	FunctionManager* functionManager() const { return m_functionManager; }

protected:
	FunctionManager* m_functionManager;

	/*********************************************************************
	 * Bus Manager
	 *********************************************************************/
public:
	BusProperties* busManager() const { return m_busManager; }
 
protected:
	BusProperties* m_busManager;

	/*********************************************************************
	 * Plugin Manager
	 *********************************************************************/
public:
	PluginManager* pluginManager() const { return m_pluginManager; }

protected:
	PluginManager* m_pluginManager;

	/*********************************************************************
	 * Virtual Console
	 *********************************************************************/
public:
	VirtualConsole* virtualConsole() const { return m_virtualConsole; }

protected:
	void initVirtualConsole();

protected:
	VirtualConsole* m_virtualConsole;

	/*********************************************************************
	 * DMX Monitor
	 *********************************************************************/
public:
	Monitor* monitor() const { return m_monitor; }

protected:
	Monitor* m_monitor;

	/*********************************************************************
	 * Help browser
	 *********************************************************************/	
public:
	QLCDocBrowser* docBrowser() const { return m_docBrowser; }

protected:
	QLCDocBrowser* m_docBrowser;

	/*********************************************************************
	 * Status bar
	 *********************************************************************/
protected:
	/** Flashing blackout indicator on the status bar */
	QLabel* m_blackoutIndicator;

	/** Periodic timer object for the flashing indicator */
	QTimer* m_blackoutIndicatorTimer;

	/** Mode indicator on the status bar */
	QLabel* m_modeIndicator;

	/*********************************************************************
	 * Menus & toolbars
	 *********************************************************************/	
protected:
	void initActions();
	void initMenuBar();
	void initToolBar();
	void initStatusBar();

public slots:
	bool slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveAs();
	void slotFileDirectories();
	void slotFileQuit();

	void slotFixtureManager();
	void slotFixtureManagerDestroyed(QObject* object);
	void slotFunctionManager();
	void slotFunctionManagerDestroyed(QObject* object);
	void slotBusManager();
	void slotBusManagerDestroyed(QObject* object);
	void slotPluginManager();
	void slotPluginManagerDestroyed(QObject* object);

	void slotControlVirtualConsole();
	void slotVirtualConsoleClosed();
	void slotControlMonitor();
	void slotMonitorDestroyed(QObject* object);
	void slotControlBlackout();
	void slotControlPanic();

	void slotHelpIndex();
	void slotDocBrowserDestroyed(QObject* object);	
	void slotHelpAbout();
	void slotHelpAboutQt();

	void slotCustomContextMenuRequested(const QPoint&);

protected:
	QAction* m_fileNewAction;
	QAction* m_fileOpenAction;
	QAction* m_fileSaveAction;
	QAction* m_fileSaveAsAction;
	QAction* m_fileDirectoriesAction;
	QAction* m_fileSaveDefaultsAction;
	QAction* m_fileQuitAction;

	QAction* m_fixtureManagerAction;
	QAction* m_functionManagerAction;
	QAction* m_busManagerAction;
	QAction* m_pluginManagerAction;

	QAction* m_modeOperateAction;
	QAction* m_modeDesignAction;

	QAction* m_controlVCAction;
	QAction* m_controlMonitorAction;
	QAction* m_controlBlackoutAction;
	QAction* m_controlPanicAction;

	QAction* m_helpIndexAction;
	QAction* m_helpAboutAction;
	QAction* m_helpAboutQtAction;

protected:
	QMenu* m_fileMenu;
	QMenu* m_managerMenu;
	QMenu* m_controlMenu;
	QMenu* m_modeMenu;
	QMenu* m_helpMenu;
	
	QToolBar* m_toolbar;

	/*********************************************************************
	 * Workspace background
	 *********************************************************************/	
public:
	void setBackgroundImage(QString path);

public slots:
	void slotSetBackgroundImage();
	void slotClearBackgroundImage();

protected:
	QString m_backgroundImage;
};

#endif

