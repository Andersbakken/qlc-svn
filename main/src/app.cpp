/*
  Q Light Controller
  app.cpp

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

#include <qapplication.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qwidgetlist.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qtooltip.h>
#include <qstylefactory.h>
#include <qrect.h>
#include <qtimer.h>
#include <qfile.h>

#include <unistd.h>
#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <X11/Xlib.h>

#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "settingsui.h"
#include "configkeys.h"
#include "virtualconsole.h"
#include "dmxaddresstool.h"
#include "functionmanager.h"
#include "fixturemanager.h"
#include "busproperties.h"
#include "aboutbox.h"
#include "dummyoutplugin.h"
#include "dummyinplugin.h"
#include "pluginmanager.h"
#include "monitor.h"
#include "bus.h"

#include "common/qlcworkspace.h"
#include "common/settings.h"
#include "common/inputplugin.h"
#include "common/outputplugin.h"
#include "common/plugin.h"
#include "common/filehandler.h"
#include "common/documentbrowser.h"
#include "common/qlcfixturedef.h"

static const QString KModeTextOperate ("Operate");
static const QString KModeTextDesign  ("Design");

static const QString KBlackOutText    ("Blackout");
static const int KBlackOutIndicatorFlashInterval (500);
static const int KColorMask (0xff);

static const QColor KModeColorOperate (255, 0, 0);
static const QColor KModeColorDesign (0, 255, 0);

t_plugin_id App::NextPluginID (KPluginIDMin);

///////////////////////////////////////////////////////////////////
// File menu entries
#define ID_FILE                         10000
#define ID_FILE_NEW                 	10010
#define ID_FILE_OPEN                	10020
#define ID_FILE_SAVE                	10030
#define ID_FILE_SAVE_AS             	10040
#define ID_FILE_CLOSE               	10050
#define ID_FILE_PRINT               	10060
#define ID_FILE_SETTINGS                10070
#define ID_FILE_PLUGINS                 10080
#define ID_FILE_QUIT                	10090

///////////////////////////////////////////////////////////////////
// Tools menu entries
#define ID_VIEW_TOOLBAR       	        12010
#define ID_VIEW_STATUSBAR		12020

#define ID_VIEW_FIXTURE_MANAGER		12030
#define ID_VIEW_VIRTUAL_CONSOLE         12040
#define ID_VIEW_MONITOR                 12050
#define ID_VIEW_DMXADDRESSTOOL          12060
#define ID_VIEW_FUNCTION_TREE           12070
#define ID_VIEW_BUS_PROPERTIES          12080

///////////////////////////////////////////////////////////////////
// Functions menu entries
#define ID_FUNCTIONS                    13000
#define ID_FUNCTIONS_PANIC              13010
#define ID_FUNCTIONS_MODE_OPERATE       13020
#define ID_FUNCTIONS_MODE_DESIGN        13030

///////////////////////////////////////////////////////////////////
// Window menu entries
#define ID_WINDOW_MENU			14000
#define ID_WINDOW_CASCADE		14010
#define ID_WINDOW_TILE			14020

///////////////////////////////////////////////////////////////////
// Help menu entries
#define ID_HELP                         1000
#define ID_HELP_INDEX                   1010
#define ID_HELP_ABOUT               	1020
#define ID_HELP_ABOUT_QT                1030
#define ID_HELP_TOOLTIPS                1040

//////////////////////////////////////////////////////////////////
// Status bar messages
#define IDS_STATUS_DEFAULT              "Ready"

extern QApplication _qapp;
extern App* _app;

//
// DMX writer functions
//
// Generic writer function pointer
void (*writer) (t_channel, t_value);

// Used when blackout is active
void blackOutWriter(t_channel, t_value);

// Used when blackout is not active
void normalWriter(t_channel, t_value);

App::App() : QMainWindow(),
	     m_functionManager  ( NULL ),
	     m_busProperties    ( NULL ),
	     m_fixtureManager   ( NULL ),
	     m_virtualConsole   ( NULL ),
	     m_doc              ( NULL ),
	     m_workspace        ( NULL ),
	     m_functionConsumer ( NULL ),
	     m_inputPlugin      ( NULL ),
	     m_outputPlugin     ( NULL ),
	     m_dummyInPlugin    ( NULL ),
	     m_dummyOutPlugin   ( NULL ),
	     m_settings         ( NULL ),
	     m_mode             ( Design ),
	     m_blackOut         ( false ),
	     m_modeIndicator    ( NULL ),
	     m_blackOutIndicator ( NULL ),
	     m_blackOutIndicatorTimer ( NULL ),
	     m_documentBrowser  ( NULL ),
	     m_pluginManager    ( NULL ),
	     m_monitor          ( NULL )
{
	writer = normalWriter;
}

App::~App()
{
	// Delete plugin manager
	if (m_pluginManager)
	        delete m_pluginManager;
	m_pluginManager = NULL;

	// Delete function tree
	if (m_functionManager)
		delete m_functionManager;
	m_functionManager = NULL;

	// Delete bus properties
	if (m_busProperties)
		delete m_busProperties;
	m_busProperties = NULL;

	// Delete monitor
	if (m_monitor)
		delete m_monitor;
	m_monitor = NULL;
	
	// Delete fixture manager view
	if (m_fixtureManager)
		delete m_fixtureManager;
	m_fixtureManager = NULL;

	// Delete virtual console
	if (m_virtualConsole)
		delete m_virtualConsole;
	m_virtualConsole = NULL;

	// Delete doc
	if (m_doc)
		delete m_doc;
	m_doc = NULL;

	// Delete workspace
	if (m_workspace)
		delete m_workspace;
	m_workspace = NULL;

	// Delete function consumer
	if (m_functionConsumer)
		delete m_functionConsumer;
	m_functionConsumer = NULL;

	// Delete mode indicator
	if (m_modeIndicator)
		delete m_modeIndicator;
	m_modeIndicator = NULL;

	// Delete blackout indicator's timer
	if (m_blackOutIndicatorTimer)
		delete m_blackOutIndicatorTimer;
	m_blackOutIndicatorTimer = NULL;

	// Delete the blackout indicator
	if (m_blackOutIndicator)
		delete m_blackOutIndicator;
	m_blackOutIndicator = NULL;

	// Delete document browser
	if (m_documentBrowser)
		delete m_documentBrowser;
	m_documentBrowser = NULL;

	// Delete fixture definitions
	while (m_fixtureDefList.isEmpty() == false)
	{
		delete m_fixtureDefList.take(0);
	}

	// Delete plugins
	Plugin* plugin = NULL;
	void* handle = NULL;
	while (!m_pluginList.isEmpty())
	{
		plugin = m_pluginList.take(0);

		if (plugin->name() != DummyOutPlugin::PluginName
		    && plugin->name() != DummyInPlugin::PluginName)
		{
			handle = plugin->handle();
			delete plugin;

			// Close all real plugins
			dlclose(handle);
		}
	}

	// Delete dummy input/output "plugin"
	if (m_dummyOutPlugin)
		delete m_dummyOutPlugin;
	m_dummyOutPlugin = NULL;

	if (m_dummyInPlugin)
		delete m_dummyInPlugin;
	m_dummyInPlugin = NULL;

	// Delete settings
	if (m_settings)
		delete m_settings;
	m_settings = NULL;
}


//////////////////////////////////////////////////////////////////////
// Main initialization function                                     //
//                                                                  //
// This creates all items that are not saved in workspace files     //
//////////////////////////////////////////////////////////////////////
void App::init()
{
	QString msg;
	QString config;

	//
	// Settings has to be first
	//
	initSettings();

	//
	// Menus, toolbar, statusbar
	//
	initMenuBar();
	initStatusBar();
	initToolBar();

	//
	// The main view
	//
	initWorkspace();

	//
	// Fixture definitions
	//
	loadFixtureDefinitions();

	//
	// Submasters & values
	//
	initSubmasters();
	initValues();

	//
	// Function consumer
	//
	initFunctionConsumer();

	//
	// Buses
	//
	Bus::init();

	//
	// Document
	//
	initDoc();

	//
	// Virtual Console
	//
	initVirtualConsole();

	//
	// Plugins
	//
	initPlugins();

	//
	// Load the previous workspace
	//
	settings()->get(KEY_OPEN_LAST_WORKSPACE, config);
	if (config == Settings::trueValue())
	{
		config = QString::null;

		settings()->get(KEY_LAST_WORKSPACE_NAME, config);
		if (config.length() > 0)
		{
			if (doc()->loadXML(config) == false)
			{
				newDocument();
			}
		}
	}

	//
	// Check if FixtureManager should be open
	//
	_app->settings()->get(KEY_FIXTURE_MANAGER_OPEN, config);
	if (config == Settings::trueValue())
	{
		_app->slotViewFixtureManager();
	}
}


//
// Create the settings object and load its contents
//
void App::initSettings()
{
	m_settings = new Settings();
	m_settings->load();
}


//
// Create the main workspace and load related settings
//
void App::initWorkspace()
{
	QString config;
	int x, y, w, h, wmax, hmax;
	Display* display = NULL;

	//
	// Create workspace
	//
	m_workspace = new QLCWorkspace(this);
	setCentralWidget(m_workspace);
	m_workspace->setScrollBarsEnabled(true);

	//
	// Set background picture
	//
	if (settings()->get(KEY_APP_BACKGROUND, config) != -1
	    && config.length() > 0)
	{
		m_workspace->setBackground(config);
	}

	connect(m_workspace, SIGNAL(backgroundChanged(const QString&)),
		this, SLOT(slotBackgroundChanged(const QString&)));

	//
	// Set the maximum values for width & height
	//
	display = XOpenDisplay(NULL);
	if (display != NULL)
	{
		wmax = XDisplayWidth(display, XDefaultScreen(display));
		hmax = XDisplayHeight(display, XDefaultScreen(display));
	}
	else
	{
		// VGA resolution should be a safe fallback
		wmax = 640;
		hmax = 480;
	}

	//
	// Check that the window isn't positioned out of screen
	//
	if (settings()->get(KEY_APP_X, config) == -1
	    || config.toInt() < 0 || config.toInt() >= wmax)
	{
		x = 0;
	}
	else
	{
		x = config.toInt();
	}

	if (settings()->get(KEY_APP_Y, config) == -1
	    || config.toInt() < 0 || config.toInt() >= hmax)
	{
		y = 0;
	}
	else
	{
		y = config.toInt();
	}

	//
	// Check that the window isn't going to be too big
	//
	if (settings()->get(KEY_APP_W, config) == -1
	    || config.toInt() <= 0 || config.toInt() > wmax)
	{
		w = wmax - 100; // Heretic
	}
	else
	{
		w = config.toInt();
	}

	if (settings()->get(KEY_APP_H, config) == -1
	    || config.toInt() <= 0 || config.toInt() > hmax)
	{
		h = hmax - 100; // Heretic
	}
	else
	{
		h = config.toInt();
	}
	
	XCloseDisplay(display);

	//
	// Set geometry
	//
	setGeometry(x, y, w, h);

	//
	// Set maximized
	//
	if (settings()->get(KEY_APP_MAXIMIZED, config) != -1
	    && config == Settings::trueValue())
	{
		showMaximized();
	}

	//
	// Tooltips
	//
	if (settings()->get(KEY_APP_SHOW_TOOLTIPS, config) != -1
	    && config == Settings::falseValue())
	{
		QToolTip::setGloballyEnabled(false);
		m_helpMenu->setItemChecked(ID_HELP_TOOLTIPS, false);
	}
	else
	{
		QToolTip::setGloballyEnabled(true);
		m_helpMenu->setItemChecked(ID_HELP_TOOLTIPS, true);
	}

	//
	// Main application icon
	//
	setIcon(QString(PIXMAPS) + QString("/qlc.png"));

	//
	// Get the widget style from settings
	//
	if (m_settings->get(KEY_WIDGET_STYLE, config) != -1
	    && config != "")
	{
		QApplication::setStyle(config);
	}
}


//
// Menu bar
//
void App::initMenuBar()
{
	///////////////////////////////////////////////////////////////////
	// File Menu
	m_fileMenu = new QPopupMenu();
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/filenew.png")),
			       "&New", this, SLOT(slotFileNew()),
			       CTRL+Key_N, ID_FILE_NEW);
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fileopen.png")),
			       "&Open...", this, SLOT(slotFileOpen()),
			       CTRL+Key_O, ID_FILE_OPEN);
	m_fileMenu->insertSeparator();
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/filesave.png")),
			       "&Save", this, SLOT(slotFileSave()),
			       CTRL+Key_S, ID_FILE_SAVE);
	m_fileMenu->insertItem("Save &As...", this, SLOT(slotFileSaveAs()),
			       0, ID_FILE_SAVE_AS);
	m_fileMenu->insertSeparator();
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			       "Se&ttings...", this, SLOT(slotFileSettings()),
			       0, ID_FILE_SETTINGS);
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/plugin.png")),
			       "&Plugins...", this, SLOT(slotFilePlugins()),
			       0, ID_FILE_PLUGINS);
	m_fileMenu->insertSeparator();
	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/exit.png")),
			       "E&xit", this, SLOT(slotFileQuit()),
			       CTRL+Key_Q, ID_FILE_QUIT);

	connect(m_fileMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	///////////////////////////////////////////////////////////////////
	// Tools Menu
	m_toolsMenu = new QPopupMenu();
	m_toolsMenu->setCheckable(true);
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fixture.png")),
				"Fixture Manager", this,
				SLOT(slotViewFixtureManager()),
				CTRL + Key_D, ID_VIEW_FIXTURE_MANAGER);
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/virtualconsole.png")),
				"Virtual Console", this,
				SLOT(slotViewVirtualConsole()),
				CTRL + Key_V, ID_VIEW_VIRTUAL_CONSOLE);
	m_toolsMenu->insertSeparator();
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/function.png")),
				"Function Manager", this,
				SLOT(slotViewFunctionManager()),
				CTRL + Key_F, ID_VIEW_FUNCTION_TREE);
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/bus.png")),
				"Bus Properties", this,
				SLOT(slotViewBusProperties()), CTRL + Key_B,
				ID_VIEW_BUS_PROPERTIES);
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/monitor.png")),
				"Monitor", this,
				SLOT(slotViewMonitor()),
				CTRL + Key_M, ID_VIEW_MONITOR);
	m_toolsMenu->insertSeparator();
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/panic.png")),
				"Panic!", this, SLOT(slotPanic()), CTRL + Key_P,
				ID_FUNCTIONS_PANIC);

	connect(m_toolsMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	////////////////////////////////////////////////////////////////////
	// Mode menu
	m_modeMenu = new QPopupMenu();
	m_modeMenu->setCheckable(true);
	m_modeMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/player_stop.png")),
			       "Design", this,
			       SLOT(slotSetDesignMode()), CTRL + Key_D,
			       ID_FUNCTIONS_MODE_DESIGN);

	m_modeMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/player_play.png")),
			       "Operate", this,
			       SLOT(slotSetOperateMode()), CTRL + Key_R,
			       ID_FUNCTIONS_MODE_OPERATE);

	connect(m_modeMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	///////////////////////////////////////////////////////////////////
	// Window Menu
	m_windowMenu = new QPopupMenu();
	connect(m_windowMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	connect(m_windowMenu, SIGNAL(activated(int)),
		this, SLOT(slotWindowMenuCallback(int)));

	///////////////////////////////////////////////////////////////////
	// Help menu
	m_helpMenu = new QPopupMenu();
	m_helpMenu->setCheckable(true);
	m_helpMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/help.png")),
			       "Index...", this, SLOT(slotHelpIndex()),
			       SHIFT + Key_F1, ID_HELP_INDEX);
	m_helpMenu->insertSeparator();
	m_helpMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/qlc.png")),
			       "About...", this, SLOT(slotHelpAbout()),
			       0, ID_HELP_ABOUT);
	m_helpMenu->insertItem("About Qt...", this, SLOT(slotHelpAboutQt()),
			       0, ID_HELP_ABOUT_QT);
	m_helpMenu->insertSeparator();
	m_helpMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("")),
			       "Show Tooltips", this, SLOT(slotHelpTooltips()),
			       0, ID_HELP_TOOLTIPS);

	///////////////////////////////////////////////////////////////////
	// Menubar configuration
	menuBar()->insertItem("&File", m_fileMenu);
	menuBar()->insertItem("&Tools", m_toolsMenu);
	m_toolsMenu->insertItem("&Mode", m_modeMenu);
	menuBar()->insertItem("&Window", m_windowMenu);
	menuBar()->insertSeparator();
	menuBar()->insertItem("&Help", m_helpMenu);

	menuBar()->setSeparator(QMenuBar::InWindowsStyle);
}


//
// Status bar
//
void App::initStatusBar()
{
	//
	// Mode Indicator
	//
	m_modeIndicator = new QLabel(KModeTextDesign, statusBar());
	statusBar()->addWidget(m_modeIndicator, 0, true);

	//
	// Blackout Indicator
	//
	m_blackOutIndicatorTimer = new QTimer(this);
	m_blackOutIndicator = new QLabel(QString::null, statusBar());
	statusBar()->addWidget(m_blackOutIndicator, 0, true);
}

#include <qlayout.h>

//
// Create & fill toolbar
//
void App::initToolBar()
{
	m_toolbar = new QToolBar(this, "Workspace");
	m_toolbar->setMovingEnabled(false);

	m_newTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					  QString("/filenew.png")),
				  "New workspace",
				  0,
				  this,
				  SLOT(slotFileNew()),
				  m_toolbar);

	m_openTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					   QString("/fileopen.png")),
				   "Open existing workspace",
				   0,
				   this,
				   SLOT(slotFileOpen()),
				   m_toolbar);

	m_saveTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					   QString("/filesave.png")),
				   "Save current workspace",
				   0,
				   this,
				   SLOT(slotFileSave()),
				   m_toolbar);

	m_toolbar->addSeparator();

	m_dmTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					 QString("/fixture.png")),
				 "Fixture manager",
				 0,
				 this,
				 SLOT(slotViewFixtureManager()),
				 m_toolbar);

	m_vcTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					 QString("/virtualconsole.png")),
				 "Virtual console",
				 0,
				 this,
				 SLOT(slotViewVirtualConsole()),
				 m_toolbar);

	m_ftTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					 QString("/function.png")),
				 "Function manager",
				 0,
				 this,
				 SLOT(slotViewFunctionManager()),
				 m_toolbar);

	m_toolbar->addSeparator();

	m_monitorTB = new QToolButton(QPixmap(QString(PIXMAPS) +
					      QString("/monitor.png")),
				      "Monitor",
				      0,
				      this,
				      SLOT(slotViewMonitor()),
				      m_toolbar);
					
	m_blackOutTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					       QString("/fileclose.png")),
				       "Blackout",
				       0,
				       this,
				       SLOT(slotToggleBlackOut()),
				       m_toolbar);

	m_modeTB = new QToolButton(QPixmap(QString(PIXMAPS) + 
					   QString("/player_play.png")),
				   "Design Mode; All editing features enabled",
				   0,
				   this,
				   SLOT(slotSetMode()),
				   m_toolbar);
}


//
// Initialize the function consumer
//
void App::initFunctionConsumer()
{
	m_functionConsumer = new FunctionConsumer();
	assert(m_functionConsumer);

	m_functionConsumer->init();
	m_functionConsumer->start();
}

//
// Doc contains all workspace-related _variable_ data
//
void App::initDoc()
{
	// Delete existing document object
	if (m_doc)
	{
		delete m_doc;
	}
	
	// Create a new document object
	m_doc = new Doc();
	connect(m_doc, SIGNAL(modified(bool)),
		this, SLOT(slotDocModified(bool)));
	
	
	// Connect the fixture list change signal from the new document object
	if (m_fixtureManager)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureRemoved(t_fixture_id)));
	}
	
	if (m_functionManager)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureRemoved(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureChanged(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureChanged(t_fixture_id)));
	}
}


//
// Create virtual console
//
void App::initVirtualConsole(void)
{
	if (m_virtualConsole)
	{
		delete m_virtualConsole;
	}

	m_virtualConsole = new VirtualConsole(workspace());
	m_virtualConsole->initView();

	connect(m_virtualConsole, SIGNAL(closed()),
		this, SLOT(slotVirtualConsoleClosed()));

	m_virtualConsole->resize(300, 300);
}

//////////////////////////////////////////////////////////
// Operational control functions                        //
//////////////////////////////////////////////////////////
//
// New document; destroy everything and start anew
//
bool App::slotFileNew()
{
	bool result = false;

	if (doc()->isModified())
	{
		QString msg;
		msg = "Are you sure you want to clear the current workspace?\n";
		msg += "There are unsaved changes.";
		if (QMessageBox::warning(this, KApplicationNameShort, msg,
					 QMessageBox::Yes, QMessageBox::No)
		    == QMessageBox::Yes)
		{
			newDocument();
			result = true;
		}
	}
	else
	{
		newDocument();
		result = true;
	}

	return result;
}


void App::newDocument()
{
	setCaption(KApplicationNameLong);

	initDoc();
	initVirtualConsole();
	m_inputPlugin->setEventReceiver(_app->virtualConsole());

	//
	// Set the last workspace name
	//
	m_settings->set(KEY_LAST_WORKSPACE_NAME, QString::null);
}


//
// Open an existing document
//
void App::slotFileOpen()
{
	QString fileName;

	/* Check that the user is aware of losing previous changes */
	if (doc()->isModified())
	{
		QString msg;
		msg = "Are you sure you want to clear the current workspace?\n";
		msg += "There are unsaved changes.";
		if (QMessageBox::warning(this, KApplicationNameShort, msg,
					 QMessageBox::Yes, QMessageBox::No)
		    == QMessageBox::No)
		{
			return;
		}
	}

	fileName = QFileDialog::getOpenFileName(m_doc->fileName(), 
				QString("*") + KExtWorkspace, this);
	
	if (fileName.isEmpty() == false)
	{
		newDocument();
		
		if (doc()->loadXML(fileName) == false)
		{
			QMessageBox::critical(this, KApplicationNameShort,
				      "Errors occurred while reading file.");
		}
	}
}


//
// Save current workspace
//
void App::slotFileSave()
{
	if (m_doc->fileName().isEmpty())
	{
		slotFileSaveAs();
	}
	else
	{
		if (m_doc->saveXML(m_doc->fileName()) == true)
		{
			setCaption(KApplicationNameLong + QString(" - ") +
				   doc()->fileName());
		}
	}
}

//
// Save current workspace, ask for a file name
//
void App::slotFileSaveAs()
{
	QString fileName = QFileDialog::getSaveFileName(
		m_doc->fileName(), QString("*") + KExtWorkspace, this);

	if (fileName.isEmpty() == false)
	{
		// Use the suffix ".qxw" always
		if (fileName.right(4) != KExtWorkspace)
		{
			fileName += KExtWorkspace;
		}

		if (m_doc->saveXML(fileName) == true)
		{
			setCaption(KApplicationNameLong + QString(" - ") +
				   doc()->fileName());
		}
	}
}


//
// Open settings UI
//
void App::slotFileSettings()
{
	SettingsUI* sui = new SettingsUI(this);
	sui->init();

	if (sui->exec() == QDialog::Accepted)
	{
		// Save settings if OK
		settings()->save();
	}

	delete sui;
}

//
// Open plugin manager
//
void App::slotFilePlugins()
{
	if (m_pluginManager == NULL)
	{
		m_pluginManager = new PluginManager(workspace());
		m_pluginManager->initView();
	}

	connect(m_pluginManager, SIGNAL(closed()),
		this, SLOT(slotPluginManagerClosed()));

	m_pluginManager->show();
	m_pluginManager->setGeometry(0, 0, 600, 400);
}

//
// Plugin manager was closed
//
void App::slotPluginManagerClosed()
{
	delete m_pluginManager;
	m_pluginManager = NULL;
}

//
// Quit the program
//
void App::slotFileQuit()
{
	close();
}


//
// View fixture manager
//
void App::slotViewFixtureManager()
{
	if (m_fixtureManager == NULL)
	{
		// Create fixture manager view
		m_fixtureManager = new FixtureManager(workspace());
		assert(m_fixtureManager);
		m_fixtureManager->initView();
      
		connect(m_fixtureManager, SIGNAL(closed()),
			this, SLOT(slotFixtureManagerClosed()));
      
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureAdded(t_fixture_id)));

		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureRemoved(t_fixture_id)));

		connect(_app, SIGNAL(modeChanged()),
			m_fixtureManager, SLOT(slotModeChanged()));
	}

	m_toolsMenu->setItemChecked(ID_VIEW_FIXTURE_MANAGER, true);
	m_fixtureManager->show();
	m_fixtureManager->setFocus();
}


//
// Fixture manager was closed, uncheck menu item
//
void App::slotFixtureManagerClosed()
{
	m_toolsMenu->setItemChecked(ID_VIEW_FIXTURE_MANAGER, false);

	if (m_fixtureManager != NULL)
	{
		delete m_fixtureManager;
		m_fixtureManager = NULL;
	}
}


//
// View virtual console
//
void App::slotViewVirtualConsole()
{
	m_toolsMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, true);
	m_virtualConsole->show();
	m_virtualConsole->setFocus();
}


//
// Virtual console was closed, uncheck menu item
//
void App::slotVirtualConsoleClosed()
{
	m_toolsMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, false);
}


//
// View function manager
//
void App::slotViewFunctionManager()
{
	if (m_functionManager == NULL)
	{
		m_functionManager = new FunctionManager(workspace());
		m_functionManager->init();

		connect(m_functionManager, SIGNAL(closed()),
			this, SLOT(slotFunctionManagerClosed()));

		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureRemoved(t_fixture_id)));

		connect(m_doc, SIGNAL(fixtureChanged(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureChanged(t_fixture_id)));

		connect(m_doc, SIGNAL(functionAdded(t_function_id)),
			m_functionManager, SLOT(slotFunctionAdded(t_function_id)));

		connect(m_doc, SIGNAL(functionRemoved(t_function_id)),
			m_functionManager, SLOT(slotFunctionRemoved(t_function_id)));

		connect(m_doc, SIGNAL(functionChanged(t_function_id)),
			m_functionManager, SLOT(slotFunctionChanged(t_function_id)));
	}
	else
	{
		// Hide the window first so that it pops on top
		m_functionManager->hide();
	}

	m_functionManager->show();
}


//
// Function manager was closed, delete the instance
//
void App::slotFunctionManagerClosed()
{
	if (m_functionManager)
	{
		disconnect(m_functionManager);
		delete m_functionManager;
		m_functionManager = NULL;
	}
}


//
// View Bus Properties
//
void App::slotViewBusProperties()
{
	if (m_busProperties == NULL)
	{
		m_busProperties = new BusProperties(workspace());
		m_busProperties->init();
		connect(m_busProperties, SIGNAL(closed()),
			this, SLOT(slotBusPropertiesClosed()));
	}
	else
	{
		m_busProperties->hide();
	}

	m_busProperties->show();
}


//
// Bus Properties Closed
//
void App::slotBusPropertiesClosed()
{
	if (m_busProperties)
	{
		disconnect(m_busProperties);
		delete m_busProperties;
		m_busProperties = NULL;
	}
}


//
// View DMX monitor
//
void App::slotViewMonitor()
{
	if (m_monitor == NULL)
	{
		m_monitor = new Monitor(workspace());
		assert(m_monitor);
		m_monitor->init();
		
		connect(m_monitor, SIGNAL(closed()),
			this, SLOT(slotMonitorClosed()));
	}

	m_toolsMenu->setItemChecked(ID_VIEW_MONITOR, true);
	m_monitor->show();
	m_monitor->setFocus();
}


//
// DMX monitor was closed
//
void App::slotMonitorClosed()
{
	m_toolsMenu->setItemChecked(ID_VIEW_MONITOR, false);
	
	if (m_monitor != NULL)
	{
		delete m_monitor;
		m_monitor = NULL;
	}
}


//
// Refresh menu items' status depending on system mode
// also fills window menu with open window titles
//
void App::slotRefreshMenus()
{
	//
	// Refresh tools & mode menus
	//
	if (m_mode == Operate)
	{
		m_fileMenu->setItemEnabled(ID_FILE_NEW, false);
		m_fileMenu->setItemEnabled(ID_FILE_OPEN, false);
		m_fileMenu->setItemEnabled(ID_FILE_SAVE_AS, false);
		m_fileMenu->setItemEnabled(ID_FILE_SETTINGS, false);
		m_fileMenu->setItemEnabled(ID_FILE_QUIT, false);

		m_toolsMenu->setItemEnabled(ID_VIEW_FUNCTION_TREE, false);

		m_modeMenu->setItemChecked(ID_FUNCTIONS_MODE_OPERATE, true);
		m_modeMenu->setItemChecked(ID_FUNCTIONS_MODE_DESIGN, false);
	}
	else
	{
		m_fileMenu->setItemEnabled(ID_FILE_NEW, true);
		m_fileMenu->setItemEnabled(ID_FILE_OPEN, true);
		m_fileMenu->setItemEnabled(ID_FILE_SAVE_AS, true);
		m_fileMenu->setItemEnabled(ID_FILE_SETTINGS, true);
		m_fileMenu->setItemEnabled(ID_FILE_QUIT, true);

		m_toolsMenu->setItemEnabled(ID_VIEW_FUNCTION_TREE, true);

		m_modeMenu->setItemChecked(ID_FUNCTIONS_MODE_OPERATE, false);
		m_modeMenu->setItemChecked(ID_FUNCTIONS_MODE_DESIGN, true);
	}

	//
	// Refresh window menu
	//
	QWidget* widget;
	int id = 0;

	QPtrList <QWidget> wl = workspace()->windowList();

	m_windowMenu->clear();
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_sidetree.png")),
				 "Cascade", this, SLOT(slotWindowCascade()),
				 0, ID_WINDOW_CASCADE);
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_left_right.png")),
				 "Tile", this, SLOT(slotWindowTile()), 0, ID_WINDOW_TILE);
	m_windowMenu->insertSeparator();

	for (widget = wl.first(); widget != NULL; widget = wl.next())
	{
		m_windowMenu->insertItem(widget->caption(), id);
		if (widget->isVisible() == true)
		{
			m_windowMenu->setItemChecked(id, true);
		}
		id++;
	}
}


//
// A window title was selected from window menu, show that window
//
void App::slotWindowMenuCallback(int item)
{
	QPtrList <QWidget> wl = workspace()->windowList();

	if (item == ID_WINDOW_CASCADE || item == ID_WINDOW_TILE)
	{
		return;
	}

	if (wl.count())
	{
		QWidget* widget;

		widget = wl.at(item);
		if (widget != NULL)
		{
			widget->show();
			widget->setFocus();
		}
		else
		{
			assert(false);
		}

		disconnect(m_windowMenu);
	}
}


//
// Windows should be organized in a cascaded order.
//
void App::slotWindowCascade()
{
	workspace()->cascade();
}


//
// Windows should be organized as a tile view.
//
void App::slotWindowTile()
{
	workspace()->tile();
}


//
// Panic button pressed; shut down all running functions
//
void App::slotPanic()
{
	/* Shut down all running functions */
	m_functionConsumer->purge();
}


//
// Help -> Index
//
void App::slotHelpIndex()
{
	if (m_documentBrowser == NULL)
	{
		m_documentBrowser = new DocumentBrowser(this);
		m_documentBrowser->init();
		connect(m_documentBrowser, SIGNAL(closed()),
			this, SLOT(slotDocumentBrowserClosed()));
	}
	else
	{
		m_documentBrowser->hide();
	}

	m_documentBrowser->show();
}


void App::slotDocumentBrowserClosed()
{
	if (m_documentBrowser)
	{
		disconnect(m_documentBrowser);
		delete m_documentBrowser;
		m_documentBrowser = NULL;
	}
}

//
// Help -> About QLC
//
void App::slotHelpAbout()
{
	AboutBox* ab;
	ab = new AboutBox(this);
	ab->exec();
	delete ab;
}


//
// Help -> About QT version
//
void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, QString("Q Light Controller"));
}

//
// Help -> Toggle tooltips
//
void App::slotHelpTooltips()
{
	if (QToolTip::isGloballyEnabled())
	{
		QToolTip::setGloballyEnabled(false);
		m_helpMenu->setItemChecked(ID_HELP_TOOLTIPS, false);
		m_settings->set(KEY_APP_SHOW_TOOLTIPS, Settings::falseValue());
	}
	else
	{
		QToolTip::setGloballyEnabled(true);
		m_helpMenu->setItemChecked(ID_HELP_TOOLTIPS, true);
		m_settings->set(KEY_APP_SHOW_TOOLTIPS, Settings::trueValue());
	}
}

//
// Window close button was pressed
//
void App::closeEvent(QCloseEvent* e)
{
	int result = 0;

	if (m_mode == Operate)
	{
		QString msg;
		msg = "Cannot close while in operate mode.\n";
		msg += "Switch back to Design mode to close the application.";
		QMessageBox::warning(this, KApplicationNameShort, msg);
		e->ignore();
		return;
	}

	if (m_doc->isModified())
	{
		result = QMessageBox::information(this, "Exit...",
						  "Do you want to save before exit?",
						  QMessageBox::Yes, QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			slotFileSave();
			e->accept();
		}
		else if (result == QMessageBox::No)
		{
			e->accept();
		}
		else if (result == QMessageBox::Cancel)
		{
			e->ignore();
		}
	}
	else
	{
		e->accept();
	}

	saveSettings();
}


//
// Save some App-related global settings
//
void App::saveSettings()
{
	//
	// Main window geometry
	//
	m_settings->set(KEY_APP_X, x());
	m_settings->set(KEY_APP_Y, y());
	m_settings->set(KEY_APP_W, width());
	m_settings->set(KEY_APP_H, height());

	m_settings->set(KEY_APP_MAXIMIZED, (isMaximized()) ? Settings::trueValue() :
			Settings::falseValue());

	settings()->save();
}


void App::slotSetDesignMode()
{
	if (m_mode == Operate)
	{
		slotSetMode();
	}
}


void App::slotSetOperateMode()
{
	if (m_mode == Design)
	{
		slotSetMode();
	}
}


//
// The main operating mode has been changed, signal it to everyone
//
void App::slotSetMode()
{
	if (m_mode == Operate)
	{
		if (m_functionConsumer->runningFunctions())
		{
			QString msg;
			msg = "There are running functions. Do you really wish to stop\n";
			msg += "them and change back to Design Mode?";
			int result = QMessageBox::warning(this, KApplicationNameShort, msg,
							  QMessageBox::Yes, QMessageBox::No);
			if (result == QMessageBox::No)
			{
				return;
			}
			else
			{
				m_functionConsumer->purge();
			}
		}

		m_modeIndicator->setText(KModeTextDesign);
		m_modeTB->setPixmap(QString(PIXMAPS) + QString("/player_play.png"));
		QToolTip::add(m_modeTB, "Switch to Operate mode");

		m_newTB->setEnabled(true);
		m_openTB->setEnabled(true);
		m_ftTB->setEnabled(true);

		m_mode = Design;
	}
	else
	{
		m_modeIndicator->setText(KModeTextOperate);
		m_modeTB->setPixmap(QString(PIXMAPS) + QString("/player_stop.png"));
		QToolTip::add(m_modeTB, "Switch back to Design mode");

		m_newTB->setEnabled(false);
		m_openTB->setEnabled(false);
		m_ftTB->setEnabled(false);

		// Close function tree if it's open
		slotFunctionManagerClosed();

		// Close plugin manager if it's open
		slotPluginManagerClosed();

		m_mode = Operate;
	}

	emit modeChanged();
}

void App::slotToggleBlackOut()
{
	if (m_blackOut)
	{
		m_blackOut = false;
		m_blackOutIndicator->setText(QString::null);
		m_blackOutIndicatorTimer->stop();
		disconnect(m_blackOutIndicatorTimer, SIGNAL(timeout()),
			   this, SLOT(slotFlashBlackOutIndicator()));

		m_outputPlugin->writeRange(0, m_values, KChannelMax);
		//
		// Set the channel writer function to blackout writer
		// to prevent channel value updates to DMX hardware
		//
		writer = normalWriter;
	}
	else
	{
		m_blackOut = true;
		m_blackOutIndicator->setText(KBlackOutText);
		connect(m_blackOutIndicatorTimer, SIGNAL(timeout()),
			this, SLOT(slotFlashBlackOutIndicator()));
		m_blackOutIndicatorTimer->start(KBlackOutIndicatorFlashInterval);
 
		t_value tmpValues[KChannelMax] = { 0 };
		m_outputPlugin->writeRange(0, tmpValues, KChannelMax);

		// old stuff that was too slow for the USB2DMX code
		//      for (t_channel ch = 0; ch < KChannelMax; ch++)
		//	{
		//	  m_outputPlugin->writeChannel(ch, 0);
		//	}

		//
		// Set the channel writer function to blackout writer
		// to prevent channel value updates to DMX hardware
		//
		writer = blackOutWriter;
	}
}

void App::slotFlashBlackOutIndicator()
{
	QColor c(m_blackOutIndicator->backgroundColor());
	c.setRgb(c.red() ^ KColorMask,
		 c.green() ^ KColorMask,
		 c.blue() ^ KColorMask);
	m_blackOutIndicator->setPaletteBackgroundColor(c);
}

void App::slotBackgroundChanged(const QString& path)
{
	m_settings->set(KEY_APP_BACKGROUND, path);
}

void App::slotDocModified(bool state)
{
	QString caption(KApplicationNameLong);

	if (m_doc->fileName() != QString::null)
	{
		caption += QString(" - ") + m_doc->fileName();
	}

	if (state == true)
	{
		setCaption(caption + QString(" *"));
	}
	else
	{
		setCaption(caption);
	}
}

//////////////////
// Plugin stuff //
//////////////////

//
// Search and load plugins
//
void App::initPlugins()
{
	QString path;

	// First of all, add the dummy output plugin
	m_dummyOutPlugin = new DummyOutPlugin(NextPluginID++);
	connect(m_dummyOutPlugin, SIGNAL(activated(Plugin*)),
		this, SLOT(slotPluginActivated(Plugin*)));
	addPlugin(m_dummyOutPlugin);
	
	m_dummyInPlugin = new DummyInPlugin(NextPluginID++);
	connect(m_dummyInPlugin, SIGNAL(activated(Plugin*)),
		this, SLOT(slotPluginActivated(Plugin*)));
	addPlugin(m_dummyInPlugin);

	QDir d(PLUGINS);
	d.setFilter(QDir::Files);
	if (d.exists() == false || d.isReadable() == false)
	{
		QString msg;
		msg = "Unable to access plugin directory:\n";
		msg += PLUGINS;
		QMessageBox::warning(this, KApplicationNameShort, msg);
	}

	QStringList dirlist(d.entryList());
	QStringList::Iterator it;

	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		// Ignore everything else than .so files
		if ((*it).right(2) != QString("so"))
		{
			continue;
		}

		path = QString(PLUGINS) + QString("/") + *it;

		probePlugin(path);
	}

	//
	// Use the output plugin that user has selected previously
	//
	QString config;
	if (settings()->get(KEY_OUTPUT_PLUGIN, config) != -1
	    && config != "")
	{
		Plugin* plugin = searchPlugin(config, Plugin::OutputType);
		if (plugin != NULL)
		{
			m_outputPlugin = static_cast<OutputPlugin*> (plugin);
		}
		else
		{
			m_outputPlugin = m_dummyOutPlugin;
		}
	}
	if (settings()->get(KEY_INPUT_PLUGIN, config) != -1
	    && config != "")
    	{
      	  	Plugin* plugin = searchPlugin(config, Plugin::InputType);
      	  	if (plugin != NULL)
		{
	  		m_inputPlugin = static_cast<InputPlugin*> (plugin);
                        m_inputPlugin->setEventReceiver(_app->virtualConsole());
		}
      	  	else
        	{
	  		m_inputPlugin = m_dummyInPlugin;
		}
    	}
	else
	{
		m_outputPlugin = m_dummyOutPlugin;
		m_inputPlugin = m_dummyInPlugin;
	}

	slotPluginActivated(m_outputPlugin);
	slotPluginActivated(m_inputPlugin);
}


//
// Try if the given file is a plugin
//
bool App::probePlugin(QString path)
{
	void* handle = NULL;

	handle = ::dlopen((const char*) path, RTLD_LAZY);
	if (handle == NULL)
	{
		fprintf(stderr, "dlopen: %s\n", dlerror());
	}
	else
	{
		typedef Plugin* create_t(int);
		typedef void destroy_t(Plugin*);

		create_t* create = (create_t*) ::dlsym(handle, "create");
		destroy_t* destroy = (destroy_t*) ::dlsym(handle, "destroy");

		if (create == NULL || destroy == NULL)
		{
			fprintf(stderr, "dlsym(init): %s\n", dlerror());
			return false;
		}
		else
		{
			Plugin* plugin = create(App::NextPluginID++);
			assert(plugin != NULL);
			plugin->setHandle(handle);

			plugin->setConfigDirectory(QString(getenv("HOME")) +
						   QString("/") + QString(QLCUSERDIR) +
						   QString("/"));
			plugin->loadSettings();

			connect(plugin, SIGNAL(activated(Plugin*)),
				this, SLOT(slotPluginActivated(Plugin*)));
			addPlugin(plugin);

			qDebug(QString("Found ") + plugin->name() + " plugin");
		}
	}

	return true;
}


//
// Add a plugin to list
//
void App::addPlugin(Plugin* plugin)
{
	assert(plugin != NULL);
	m_pluginList.append(plugin);
}


//
// Remove a plugin from list
//
void App::removePlugin(Plugin* plugin)
{
	assert(plugin != NULL);
	m_pluginList.remove(plugin);
}


//
// Search for a plugin by its name
//
Plugin* App::searchPlugin(QString name)
{
	Plugin* plugin = NULL;

	for (unsigned int i = 0; i < m_pluginList.count(); i++)
	{
		plugin = m_pluginList.at(i);

		if (plugin->name() == name)
		{
			return plugin;
		}
	}

	return NULL;
}


//
// Search for a plugin by its name & type
//
Plugin* App::searchPlugin(QString name, Plugin::PluginType type)
{
	Plugin* plugin = NULL;

	for (unsigned int i = 0; i < m_pluginList.count(); i++)
	{
		plugin = m_pluginList.at(i);

		if (plugin->name() == name && plugin->type() == type)
		{
			return plugin;
		}
	}

	return NULL;
}


//
// Search plugin by its id
//
Plugin* App::searchPlugin(const t_plugin_id id)
{
	Plugin* plugin = NULL;

	for (t_plugin_id i = 0; i < m_pluginList.count(); i++)
	{
		plugin = m_pluginList.at(i);

		if (plugin->id() == id)
		{
			return plugin;
		}
	}

	return NULL;
}


//
// A plugin has been activated
//
void App::slotPluginActivated(Plugin* plugin)
{
	if (plugin && plugin->type() == Plugin::OutputType)
	{
		slotChangeOutputPlugin(plugin->name());
		settings()->set("OutputPlugin", plugin->name());
		settings()->save();
	}
	if (plugin && plugin->type() == Plugin::InputType)
	{
		slotChangeInputPlugin(plugin->name());
		settings()->set("InputPlugin", plugin->name());
		settings()->save();
	}
}


//
// Search for an output plugin and set it. If not found,
// use Dummy Output by default.
//
void App::slotChangeOutputPlugin(const QString& name)
{
	if (m_outputPlugin != NULL)
	{
		m_outputPlugin->close();
	}

	m_outputPlugin = (OutputPlugin*) searchPlugin(name, Plugin::OutputType);
	if (m_outputPlugin == NULL)
	{
		// If an output plugin cannot be found, use the dummy plugin
		m_outputPlugin = m_dummyOutPlugin;
	}

	m_outputPlugin->open();
}

void App::slotChangeInputPlugin(const QString& name)
{
	if (m_inputPlugin != NULL)
	{
		m_inputPlugin->close();
	}

	m_inputPlugin = (InputPlugin*) searchPlugin(name, Plugin::InputType);
	if (m_inputPlugin == NULL)
	{
		// If an input plugin cannot be found, use the dummy plugin
		m_inputPlugin = m_dummyInPlugin;
	}
	//m_inputPlugin->setParentApp(this);
	m_inputPlugin->open();
}

/*********************************************************************
 * Fixture definitions
 *********************************************************************/
bool App::loadFixtureDefinitions()
{
	QLCFixtureDef* fixtureDef = NULL;
	QString path = QString::null;

	QDir dir(QString(FIXTURES));

	dir.setFilter(QDir::Files);
	dir.setNameFilter("*.qxf");

	if (dir.exists() == false)
	{
		QMessageBox::critical(this, 
				      "Unable to load fixture definitions", 
				      QString(FIXTURES) + 
				      QString(" does not exist!"));
		return false;
	}
	else if (dir.isReadable() == false)
	{
		QMessageBox::critical(this, 
				      "Unable to load fixture definitions", 
				      QString(FIXTURES) + 
				      QString(" cannot be read by user!"));
		return false;
	}

	QStringList dirlist(dir.entryList());
	QStringList::Iterator it;

	// Go thru all files
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		path = QString(FIXTURES) + QString("/") + *it;
		fixtureDef = new QLCFixtureDef(path);

		qDebug("Loaded fixture definition from %s: %s-%s",
		       (const char*) path, 
		       (const char*) fixtureDef->manufacturer(),
		       (const char*) fixtureDef->model());

		if (fixtureDef != NULL)
		{
			m_fixtureDefList.append(fixtureDef);
		}
		else
		{
			qDebug("Fixture definition loading failed: %s", 
			       (const char*) path);
		}
	}

	return true;
}

QLCFixtureDef* App::fixtureDef(const QString& manufacturer, const QString& model)
{
	QLCFixtureDef* fixtureDef = NULL;
	QPtrListIterator <QLCFixtureDef> it(m_fixtureDefList);

	while ( (fixtureDef = *it) != NULL )
	{
		if (fixtureDef->manufacturer() == manufacturer && 
		    fixtureDef->model() == model)
		{
			return fixtureDef;
		}

		++it;
	}
	
	return NULL;
}

/*********************************************************************
 * Submasters & values
 *********************************************************************/

void App::initSubmasters()
{
	for (t_channel i = 0; i < KChannelMax; i++)
	{
		m_submasters[i] = 0;
		m_submasterValues[i] = 1;
	}
}

void App::initValues()
{
	if (m_outputPlugin)
		m_outputPlugin->readRange(0, m_values, KChannelMax);
}

//
// Set a DMX value
//
void App::setValue(t_channel ch, t_value value)
{
	if (ch < KChannelMax)
	{
		m_values[ch] = value;

		//
		// This is either normalWriter or blackOutWriter, depending
		// on whether blackout is enabled or disabled
		//
		writer(ch, value);
	}
}


void blackOutWriter(t_channel, t_value)
{
	// Don't do anything
}

void normalWriter(t_channel ch, t_value value)
{
	_app->outputPlugin()->writeChannel(ch, static_cast<t_value>
					   (((float) value) * _app->submasterValue(ch)));
}

t_value App::value(t_channel ch)
{
	return m_values[ch];
}

void App::valueRange(t_channel address, t_value* values, t_channel num)
{
	memcpy(values, m_values + address, num);
}

//
// Indicate that someone is setting submaster value to a channel
//
bool App::assignSubmaster(t_channel ch)
{
	m_submasters[ch]++;
	return true;
}


//
// Indicate that someone is no longer setting submaster value to a channel
//
bool App::resignSubmaster(t_channel ch)
{
	m_submasters[ch]--;

	assert(m_submasters[ch] >= 0);

	return true;
}


//
// Check if a channel has a submaster
//
int App::hasSubmaster(t_channel ch)
{
	return m_submasters[ch];
}


//
// After assign/resign, reset all non-assigned submaster channel values to 100%
//
void App::resetSubmasters()
{
	for (t_channel i = 0; i < KChannelMax; i++)
	{
		if (hasSubmaster(i) == 0)
		{
			setSubmasterValue(i, 100);
		}
	}
}


//
// Set the submaster value to a channel
//
void App::setSubmasterValue(t_channel ch, int percent)
{
	m_submasterValues[ch] = (float) percent / (float) 100;
	setValue(ch, m_values[ch]);
}


//
// Get the submaster value for a channel
//
float App::submasterValue(t_channel ch)
{
	return m_submasterValues[ch];
}
