/*
  Q Light Controller
  app.cpp

  Copyright (c) Heikki Junnila,
                Christopher Staite

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
#include <qlayout.h>

#include <X11/Xlib.h>

#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "virtualconsole.h"
#include "functionmanager.h"
#include "fixturemanager.h"
#include "pluginmanager.h"
#include "busproperties.h"
#include "aboutbox.h"
#include "monitor.h"
#include "bus.h"
#include "dmxmap.h"

#include "common/qlcworkspace.h"
#include "common/filehandler.h"
#include "common/documentbrowser.h"
#include "common/qlcfixturedef.h"

static const QString KModeTextOperate ("Operate");
static const QString KModeTextDesign  ("Design");

/* File menu entries */
#define MENU_FILE                       1000
#define MENU_FILE_NEW                 	1010
#define MENU_FILE_OPEN                	1020
#define MENU_FILE_SAVE                	1030
#define MENU_FILE_SAVE_AS             	1040
#define MENU_FILE_SAVE_DEFAULTS         1050
#define MENU_FILE_QUIT                	1090

/* Manager menu entries */
#define MENU_MANAGER                    2000
#define MENU_MANAGER_FIXTURE_MANAGER	2010
#define MENU_MANAGER_FUNCTION_MANAGER   2020
#define MENU_MANAGER_BUS_MANAGER        2030
#define MENU_MANAGER_INPUT_MANAGER      2040
#define MENU_MANAGER_OUTPUT_MANAGER     2050
#define MENU_MANAGER_PLUGIN_MANAGER     2060

/* Control menu entries */
#define MENU_CONTROL                    3000
#define MENU_CONTROL_MODE               3010
#define MENU_CONTROL_MODE_OPERATE       3011
#define MENU_CONTROL_MODE_DESIGN        3012
#define MENU_CONTROL_VIRTUAL_CONSOLE    3020
#define MENU_CONTROL_MONITOR            3030
#define MENU_CONTROL_BLACKOUT           3040
#define MENU_CONTROL_PANIC              3050

/* Window menu entries */
#define MENU_WINDOW                     4000
#define MENU_WINDOW_CASCADE             4010
#define MENU_WINDOW_TILE                4020

/* Help menu entries */
#define MENU_HELP                       5000
#define MENU_HELP_INDEX                 5010
#define MENU_HELP_ABOUT               	5020
#define MENU_HELP_ABOUT_QT              5030

extern QApplication _qapp;
extern App* _app;

/*********************************************************************
 * Initialization
 *********************************************************************/

App::App() : QMainWindow()
{
	m_dmxMap = NULL;
	m_functionConsumer = NULL;
	m_doc = NULL;
	m_workspace = NULL;

	m_pluginManager = NULL;
	m_functionManager = NULL;
	m_busProperties = NULL;
	m_fixtureManager = NULL;
	m_virtualConsole = NULL;
	m_documentBrowser = NULL;
	m_monitor = NULL;

	m_mode = Design;
	m_modeIndicator = NULL;

	m_blackoutIndicator = NULL;
	m_blackoutIndicatorTimer = NULL;
}

App::~App()
{
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
	if (m_blackoutIndicatorTimer)
		delete m_blackoutIndicatorTimer;
	m_blackoutIndicatorTimer = NULL;

	// Delete the blackout indicator
	if (m_blackoutIndicator)
		delete m_blackoutIndicator;
	m_blackoutIndicator = NULL;

	// Delete document browser
	if (m_documentBrowser)
		delete m_documentBrowser;
	m_documentBrowser = NULL;

	// Delete fixture definitions
	while (m_fixtureDefList.isEmpty() == false)
		delete m_fixtureDefList.take(0);
}


/**
 * Main initialization function
 */
void App::init()
{
	/* Initialize engine components first */
	initDMXMap();
	// initInputMap();
	initFunctionConsumer();
	Bus::init();
	loadFixtureDefinitions();

	// The main view
	initWorkspace();
	initMenuBar();
	initStatusBar();
	initToolBar();

	// Document
	initDoc();

	// Virtual Console
	initVirtualConsole();

	// Start up in non-modified state
	m_doc->resetModified();
}

/*****************************************************************************
 * Output mapping
 *****************************************************************************/

void App::initDMXMap()
{
	m_dmxMap = new DMXMap(KUniverseCount);
	Q_ASSERT(m_dmxMap != NULL);
	m_dmxMap->loadDefaults(KApplicationNameLong);

	connect(m_dmxMap, SIGNAL(blackoutChanged(bool)),
		this, SLOT(slotDMXMapBlackoutChanged(bool)));
}

void App::slotDMXMapBlackoutChanged(bool state)
{
	if (state == true)
	{
		m_blackoutIndicator->setText("Blackout");

		connect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			this, SLOT(slotFlashBlackoutIndicator()));
		m_blackoutIndicatorTimer->start(500);
 	}
	else
	{
		m_blackoutIndicator->setText(QString::null);

		m_blackoutIndicatorTimer->stop();
		disconnect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			   this, SLOT(slotFlashBlackoutIndicator()));

		m_blackoutIndicator->unsetPalette();
	}

	m_managerMenu->setItemChecked(MENU_CONTROL_BLACKOUT, state);
	m_blackoutToolButton->setOn(state);
}

/*****************************************************************************
 * Input mapping
 *****************************************************************************/

void App::slotViewInputManager()
{
	QMessageBox::information(workspace(), "TODO", "TODO");
}

/*****************************************************************************
 * Plugin manager
 *****************************************************************************/

void App::slotViewPluginManager()
{
	if (m_pluginManager == NULL)
	{
		m_pluginManager = new PluginManager(workspace());
		m_pluginManager->resize(500, 300);
		connect(m_pluginManager, SIGNAL(closed()),
			this, SLOT(slotPluginManagerClosed()));
	}
	else
	{
		m_pluginManager->hide();
	}

	m_pluginManager->show();
}

void App::slotPluginManagerClosed()
{
	delete m_pluginManager;
	m_pluginManager = NULL;
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

void App::slotToggleBlackout()
{
	Q_ASSERT(m_dmxMap != NULL);
	if (m_dmxMap->blackout() == true)
		m_dmxMap->setBlackout(false);
	else
		m_dmxMap->setBlackout(true);
}

void App::slotFlashBlackoutIndicator()
{
	int mask = 0xff;
	QColor bg(m_blackoutIndicator->backgroundColor());
	bg.setRgb(bg.red() ^ mask, bg.green() ^ mask, bg.blue() ^ mask);
	m_blackoutIndicator->setPaletteBackgroundColor(bg);
	
	QColor fg(m_blackoutIndicator->foregroundColor());
	fg.setRgb(fg.red() ^ mask, fg.green() ^ mask, fg.blue() ^ mask);
	m_blackoutIndicator->setPaletteForegroundColor(fg);
}

/*****************************************************************************
 * Buses
 *****************************************************************************/

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

void App::slotBusPropertiesClosed()
{
	if (m_busProperties)
	{
		disconnect(m_busProperties);
		delete m_busProperties;
		m_busProperties = NULL;
	}
}

/*****************************************************************************
 * Function consumer
 *****************************************************************************/

void App::initFunctionConsumer()
{
	m_functionConsumer = new FunctionConsumer(m_dmxMap);
	Q_ASSERT(m_functionConsumer != NULL);

	/* TODO: Put this into some kind of a settings dialog */
	// m_functionConsumer->setTimerType(FunctionConsumer::RTCTimer);
	m_functionConsumer->setTimerType(FunctionConsumer::NanoSleepTimer);
}

void App::slotPanic()
{
	/* Shut down all running functions */
	m_functionConsumer->purge();
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::initDoc()
{
	// Delete existing document object
	if (m_doc != NULL)
		delete m_doc;
	
	// Create a new document object
	m_doc = new Doc();
	connect(m_doc, SIGNAL(modified(bool)),
		this, SLOT(slotDocModified(bool)));
	
	
	/* Connect fixture list change signals from the new document object */
	if (m_fixtureManager != NULL)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager, SLOT(slotFixtureRemoved(t_fixture_id)));
	}
	
	/* Connect fixture list change signals from the new document object */
	if (m_functionManager != NULL)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureRemoved(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureChanged(t_fixture_id)),
			m_functionManager, SLOT(slotFixtureChanged(t_fixture_id)));
	}
}

void App::slotDocModified(bool state)
{
	QString caption(KApplicationNameLong);

	if (m_doc->fileName() != QString::null)
		caption += QString(" - ") + m_doc->fileName();

	if (state == true)
		setCaption(caption + QString(" *"));
	else
		setCaption(caption);
}

/*****************************************************************************
 * Workspace
 *****************************************************************************/

void App::initWorkspace()
{
	QString config;

	/* Create a workspace object and set it as the central widget */
	m_workspace = new QLCWorkspace(this);
	m_workspace->loadDefaults(KApplicationNameLong);
	m_workspace->setScrollBarsEnabled(true);
	setCentralWidget(m_workspace);

	/* Main application icon */
	setIcon(QString(PIXMAPS) + QString("/qlc.png"));

	/* Resize the whole application to default size */
	resize(KApplicationDefaultWidth, KApplicationDefaultHeight);

	/* Enable all tooltips */
	QToolTip::setGloballyEnabled(true);
}

//
// Refresh menu items' status depending on system mode
// also fills window menu with open window titles
//
void App::slotRefreshMenus()
{
	//
	// Refresh window menu
	//
	QWidget* widget;
	int id = 0;

	QPtrList <QWidget> wl = workspace()->windowList();

	m_windowMenu->clear();
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_sidetree.png")),
				 "Cascade", this, SLOT(slotWindowCascade()),
				 0, MENU_WINDOW_CASCADE);
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_left_right.png")),
				 "Tile", this, SLOT(slotWindowTile()), 0, MENU_WINDOW_TILE);
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

	if (item == MENU_WINDOW_CASCADE || item == MENU_WINDOW_TILE)
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
// Window close button was pressed
//
void App::closeEvent(QCloseEvent* e)
{
	int result = 0;

	if (m_mode == Operate)
	{
		QMessageBox::warning(this,
				     "Cannot exit in Operate mode",
				     "You must switch back to Design mode\n" \
				     "to be able to close the application.");
		e->ignore();
		return;
	}

	if (m_doc->isModified())
	{
		result = QMessageBox::information(
			this, 
			"Close Q Light Controller...",
			"Do you wish to save the current workspace \n"\
			"before closing the application?",
			QMessageBox::Yes,
			QMessageBox::No,
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
}

/*****************************************************************************
 * Fixture Manager
 *****************************************************************************/

void App::createFixtureManager()
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
	}
}

void App::slotViewFixtureManager()
{
	if (m_fixtureManager == NULL)
		createFixtureManager();

	m_managerMenu->setItemChecked(MENU_MANAGER_FIXTURE_MANAGER, true);
	m_fixtureManagerToolButton->setOn(true);

	m_fixtureManager->show();
	m_fixtureManager->setFocus();
}


//
// Fixture manager was closed, uncheck menu item
//
void App::slotFixtureManagerClosed()
{
	m_managerMenu->setItemChecked(MENU_MANAGER_FIXTURE_MANAGER, false);
	m_fixtureManagerToolButton->setOn(false);

	if (m_fixtureManager != NULL)
	{
		delete m_fixtureManager;
		m_fixtureManager = NULL;
	}
}

/*****************************************************************************
 * Function Manager
 *****************************************************************************/

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

	m_functionManagerToolButton->setOn(true);
	m_functionManager->show();
}


//
// Function manager was closed, delete the instance
//
void App::slotFunctionManagerClosed()
{
	if (m_functionManager != NULL)
	{
		disconnect(m_functionManager);
		delete m_functionManager;
		m_functionManager = NULL;
	}

	m_functionManagerToolButton->setOn(false);
}

/*****************************************************************************
 * Virtual Console
 *****************************************************************************/

//
// Create virtual console
//
void App::initVirtualConsole(void)
{
	if (m_virtualConsole != NULL)
		delete m_virtualConsole;

	m_virtualConsole = new VirtualConsole(workspace());
	m_virtualConsole->initView();

	connect(m_virtualConsole, SIGNAL(closed()),
		this, SLOT(slotVirtualConsoleClosed()));

	m_virtualConsole->resize(300, 300);
	m_virtualConsole->hide();
}

//
// View virtual console
//
void App::slotViewVirtualConsole()
{
	m_managerMenu->setItemChecked(MENU_CONTROL_VIRTUAL_CONSOLE, true);
	m_virtualConsoleToolButton->setOn(true);
	m_virtualConsole->show();
	m_virtualConsole->setFocus();
}


//
// Virtual console was closed, uncheck menu item
//
void App::slotVirtualConsoleClosed()
{
	m_managerMenu->setItemChecked(MENU_CONTROL_VIRTUAL_CONSOLE, false);
	m_virtualConsoleToolButton->setOn(false);
}

/*****************************************************************************
 * Monitor
 *****************************************************************************/

//
// Create DMX monitor but don't show it
//
void App::createMonitor()
{
	if (m_monitor == NULL)
	{
		m_monitor = new Monitor(workspace(), m_dmxMap);
		Q_ASSERT(m_monitor != NULL);
		m_monitor->init();
		
		connect(m_monitor, SIGNAL(closed()),
			this, SLOT(slotMonitorClosed()));
	}
}

//
// View DMX monitor
//
void App::slotViewMonitor()
{
	if (m_monitor == NULL)
		createMonitor();

	m_managerMenu->setItemChecked(MENU_CONTROL_MONITOR, true);
	m_monitorToolButton->setOn(true);
	m_monitor->show();
	m_monitor->setFocus();
}


//
// DMX monitor was closed
//
void App::slotMonitorClosed()
{
	m_managerMenu->setItemChecked(MENU_CONTROL_MONITOR, false);
	m_monitorToolButton->setOn(false);
	
	if (m_monitor != NULL)
	{
		delete m_monitor;
		m_monitor = NULL;
	}
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

		qDebug("Loaded fixture definition for %s %s",
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

/*****************************************************************************
 * Main application Mode
 *****************************************************************************/

void App::setMode(Mode mode)
{
	/* Nothing to do if we're already in the desired mode */
	if (m_mode == mode)
		return;

	if (mode == Design)
	{
		if (m_functionConsumer->runningFunctions())
		{
			QString msg;
			msg = "There are running functions. Do you really\n";
			msg += "wish to stop them and switch back to ";
			msg += "Design mode?";
			int result = QMessageBox::warning(
				this,
				"Switch to Design Mode",
				msg,
				QMessageBox::Yes,
				QMessageBox::No);

			if (result == QMessageBox::No)
			{
				return;
			}
			else
			{
				m_functionConsumer->purge();
			}
		}

		/* Stop function consumer */
		m_functionConsumer->stop();

		m_modeIndicator->setText(KModeTextDesign);
		m_modeToolButton->setPixmap(QString(PIXMAPS) +
					    QString("/player_play.png"));
		QToolTip::add(m_modeToolButton, "Switch to Operate mode");

		m_newToolButton->setEnabled(true);
		m_openToolButton->setEnabled(true);
		m_fixtureManagerToolButton->setEnabled(true);
		m_functionManagerToolButton->setEnabled(true);

		m_fileMenu->setItemEnabled(MENU_FILE_NEW, true);
		m_fileMenu->setItemEnabled(MENU_FILE_OPEN, true);
		m_fileMenu->setItemEnabled(MENU_FILE_SAVE_AS, true);
		m_fileMenu->setItemEnabled(MENU_FILE_QUIT, true);

		m_managerMenu->setItemEnabled(MENU_MANAGER_FIXTURE_MANAGER, true);
		m_managerMenu->setItemEnabled(MENU_MANAGER_FUNCTION_MANAGER, true);
		m_managerMenu->setItemEnabled(MENU_MANAGER_BUS_MANAGER, true);
		m_managerMenu->setItemEnabled(MENU_MANAGER_INPUT_MANAGER, true);
		m_managerMenu->setItemEnabled(MENU_MANAGER_OUTPUT_MANAGER, true);
		m_managerMenu->setItemEnabled(MENU_MANAGER_PLUGIN_MANAGER, true);

		m_modeMenu->setItemChecked(MENU_CONTROL_MODE_OPERATE, false);
		m_modeMenu->setItemChecked(MENU_CONTROL_MODE_DESIGN, true);
	}
	else
	{
		m_modeIndicator->setText(KModeTextOperate);
		m_modeToolButton->setPixmap(QString(PIXMAPS) +
					    QString("/player_stop.png"));
		QToolTip::add(m_modeToolButton, "Switch to Design mode");

		m_newToolButton->setEnabled(false);
		m_openToolButton->setEnabled(false);
		m_fixtureManagerToolButton->setEnabled(false);
		m_functionManagerToolButton->setEnabled(false);

		m_fileMenu->setItemEnabled(MENU_FILE_NEW, false);
		m_fileMenu->setItemEnabled(MENU_FILE_OPEN, false);
		m_fileMenu->setItemEnabled(MENU_FILE_SAVE_AS, false);
		m_fileMenu->setItemEnabled(MENU_FILE_QUIT, false);

		m_managerMenu->setItemEnabled(MENU_MANAGER_FIXTURE_MANAGER, false);
		m_managerMenu->setItemEnabled(MENU_MANAGER_FUNCTION_MANAGER, false);
		m_managerMenu->setItemEnabled(MENU_MANAGER_BUS_MANAGER, false);
		m_managerMenu->setItemEnabled(MENU_MANAGER_INPUT_MANAGER, false);
		m_managerMenu->setItemEnabled(MENU_MANAGER_OUTPUT_MANAGER, false);
		m_managerMenu->setItemEnabled(MENU_MANAGER_PLUGIN_MANAGER, false);

		m_modeMenu->setItemChecked(MENU_CONTROL_MODE_OPERATE, true);
		m_modeMenu->setItemChecked(MENU_CONTROL_MODE_DESIGN, false);

		/* Close function manager if it's open */
		slotFunctionManagerClosed();

		/* Close fixture manager if it's open */
		slotFixtureManagerClosed();

		/* Close bus manager if it's open */
		slotBusPropertiesClosed();

		/* Start function consumer */
		m_functionConsumer->start();
	}

	m_mode = mode;

	emit modeChanged(m_mode);
}

void App::slotSetDesignMode()
{
	setMode(Design);
}

void App::slotSetOperateMode()
{
	setMode(Operate);
}

void App::slotToggleMode()
{
	if (m_mode == Design)
		setMode(Operate);
	else
		setMode(Design);
}

/*****************************************************************************
 * Help menu slots
 *****************************************************************************/

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

void App::slotHelpAbout()
{
	AboutBox* ab;
	ab = new AboutBox(this);
	ab->exec();
	delete ab;
}

void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, QString("Q Light Controller"));
}

/*****************************************************************************
 * Menu, toolbar, statusbar
 *****************************************************************************/

void App::initMenuBar()
{
	/* File Menu */
	m_fileMenu = new QPopupMenu(menuBar());
	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/filenew.png")),
		"&New",
		this,
		SLOT(slotFileNew()),
		CTRL+Key_N,
		MENU_FILE_NEW);

	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/fileopen.png")),
		"&Open...",
		this,
		SLOT(slotFileOpen()),
		CTRL+Key_O,
		MENU_FILE_OPEN);

	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/filesave.png")),
		"&Save",
		this,
		SLOT(slotFileSave()),
		CTRL+Key_S,
		MENU_FILE_SAVE);

	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/filesave.png")),
		"Save &As...",
		this,
		SLOT(slotFileSaveAs()),
		0,
		MENU_FILE_SAVE_AS);

	m_fileMenu->insertSeparator();

	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/configure.png")),
		"Save &Defaults...",
		this,
		SLOT(slotFileSaveDefaults()),
		0,
		MENU_FILE_SAVE_DEFAULTS);

	m_fileMenu->insertSeparator();

	m_fileMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/exit.png")),
		"E&xit",
		this,
		SLOT(slotFileQuit()),
		ALT+Key_F4,
		MENU_FILE_QUIT);

	connect(m_fileMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	/* Manager Menu */
	m_managerMenu = new QPopupMenu(menuBar());
	m_managerMenu->setCheckable(true);
	m_managerMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/fixture.png")),
		"&Fixture Manager",
		this,
		SLOT(slotViewFixtureManager()),
		CTRL + Key_D,
		MENU_MANAGER_FIXTURE_MANAGER);

	m_managerMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/function.png")),
		"F&unction Manager",
		this,
		SLOT(slotViewFunctionManager()),
		CTRL + Key_F,
		MENU_MANAGER_FUNCTION_MANAGER);

	m_managerMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/bus.png")),
		"&Bus Manager",
		this,
		SLOT(slotViewBusProperties()),
		CTRL + Key_B,
		MENU_MANAGER_BUS_MANAGER);

	m_managerMenu->insertSeparator();

	m_managerMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/plugin.png")),
		"Input Manager",
		this,
		SLOT(slotViewInputManager()),
		CTRL + Key_I,
		MENU_MANAGER_INPUT_MANAGER);

	m_managerMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/plugin.png")),
		"Plugin Manager",
		this,
		SLOT(slotViewPluginManager()),
		CTRL + Key_P,
		MENU_MANAGER_PLUGIN_MANAGER);

	/* Mode menu */
	m_modeMenu = new QPopupMenu(menuBar());
	m_modeMenu->setCheckable(true);
	m_modeMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/player_stop.png")),
		"Design",
		this,
		SLOT(slotSetDesignMode()),
		CTRL + Key_F11,
		MENU_CONTROL_MODE_DESIGN);

	m_modeMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/player_play.png")),
		"Operate",
		this,
		SLOT(slotSetOperateMode()),
		CTRL + Key_F12,
		MENU_CONTROL_MODE_OPERATE);

	connect(m_modeMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	/* Control Menu */
	m_controlMenu = new QPopupMenu(menuBar());
	m_controlMenu->setCheckable(true);

	m_controlMenu->insertItem("&Mode", m_modeMenu);
	m_controlMenu->insertSeparator();

	m_controlMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/virtualconsole.png")),
		"Virtual Console",
		this,
		SLOT(slotViewVirtualConsole()),
		CTRL + Key_V,
		MENU_CONTROL_VIRTUAL_CONSOLE);
	
	m_controlMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/monitor.png")),
		"Monitor",
		this,
		SLOT(slotViewMonitor()),
		CTRL + Key_M,
		MENU_CONTROL_MONITOR);

	m_controlMenu->insertSeparator();

	m_controlMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/panic.png")),
		"&Stop all functions",
		this,
		SLOT(slotPanic()),
		CTRL + Key_Escape,
		MENU_CONTROL_PANIC);

	m_controlMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/blackout.png")),
		"&Blackout",
		this,
		SLOT(slotToggleBlackout()),
		CTRL + Key_L,
		MENU_CONTROL_BLACKOUT);

	connect(m_managerMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	/* Window Menu. Contents of this menu are created during refresh. */
	m_windowMenu = new QPopupMenu(menuBar());

	connect(m_windowMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshMenus()));

	connect(m_windowMenu, SIGNAL(activated(int)),
		this, SLOT(slotWindowMenuCallback(int)));

	/* Help menu */
	m_helpMenu = new QPopupMenu(menuBar());

	m_helpMenu->setCheckable(true);

	m_helpMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/help.png")),
		"Help Index...",
		this,
		SLOT(slotHelpIndex()),
		SHIFT + Key_F1,
		MENU_HELP_INDEX);

	m_helpMenu->insertSeparator();

	m_helpMenu->insertItem(
		QPixmap(QString(PIXMAPS) + QString("/qlc.png")),
		"About...",
		this,
		SLOT(slotHelpAbout()),
		0,
		MENU_HELP_ABOUT);

	m_helpMenu->insertItem(
		"About Qt...",
		this,
		SLOT(slotHelpAboutQt()),
		0,
		MENU_HELP_ABOUT_QT);

	/* Menubar configuration */
	menuBar()->insertItem("File", m_fileMenu);
	menuBar()->insertItem("Manager", m_managerMenu);
	menuBar()->insertItem("Control", m_controlMenu);
	menuBar()->insertItem("Window", m_windowMenu);
	menuBar()->insertSeparator();
	menuBar()->insertItem("Help", m_helpMenu);
}

void App::initStatusBar()
{
	// Mode Indicator
	m_modeIndicator = new QLabel(KModeTextDesign, statusBar());
	statusBar()->addWidget(m_modeIndicator, 0, true);

	// Blackout Indicator
	m_blackoutIndicatorTimer = new QTimer(this);
	m_blackoutIndicator = new QLabel(QString::null, statusBar());
	statusBar()->addWidget(m_blackoutIndicator, 0, true);
}

void App::initToolBar()
{
	m_toolbar = new QToolBar(this, "Workspace");
	m_toolbar->setMovingEnabled(false);

	m_newToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/filenew.png")),
		"New workspace",
		0,
		this,
		SLOT(slotFileNew()),
		m_toolbar);
	
	m_openToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/fileopen.png")),
		"Open existing workspace",
		0,
		this,
		SLOT(slotFileOpen()),
		m_toolbar);
	
	m_saveToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/filesave.png")),
		"Save current workspace",
		0,
		this,
		SLOT(slotFileSave()),
		m_toolbar);
	
	m_toolbar->addSeparator();
	
	m_fixtureManagerToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/fixture.png")),
		"Fixture manager",
		0,
		this,
		SLOT(slotViewFixtureManager()),
		m_toolbar);
	m_fixtureManagerToolButton->setToggleButton(true);

	m_virtualConsoleToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/virtualconsole.png")),
		"Virtual console",
		0,
		this,
		SLOT(slotViewVirtualConsole()),
		m_toolbar);
	m_virtualConsoleToolButton->setToggleButton(true);
	
	m_functionManagerToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/function.png")),
		"Function manager",
		0,
		this,
		SLOT(slotViewFunctionManager()),
		m_toolbar);
	
	m_toolbar->addSeparator();
	m_functionManagerToolButton->setToggleButton(true);
	
	m_monitorToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/monitor.png")),
		"Monitor",
		0,
		this,
		SLOT(slotViewMonitor()),
		m_toolbar);
	m_monitorToolButton->setToggleButton(true);
	
	m_blackoutToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/blackout.png")),
		"Blackout",
		0,
		this,
		SLOT(slotToggleBlackout()),
		m_toolbar);
	m_blackoutToolButton->setToggleButton(true);

	m_modeToolButton = new QToolButton(
		QPixmap(QString(PIXMAPS) + QString("/player_play.png")),
		"Design Mode; All editing features enabled",
		0,
		this,
		SLOT(slotToggleMode()),
		m_toolbar);
}

bool App::slotFileNew()
{
	bool result = false;

	if (doc()->isModified())
	{
		QString msg;
		msg = "Do you wish to save the current workspace?\n";
		msg += "Changes will be lost if you don't save them.";
		int result = QMessageBox::warning(this, "New Workspace", msg,
						  QMessageBox::Yes,
						  QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			slotFileSave();
			newDocument();
			result = true;
		}
		else if (result == QMessageBox::No)
		{
			newDocument();
			result = true;
		}
		else
		{
			result = false;
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
	doc()->resetModified();
}

void App::slotFileOpen()
{
	QString fileName;

	/* Check that the user is aware of losing previous changes */
	if (doc()->isModified())
	{
		QString msg;
		msg = "Do you wish to save the current workspace?\n";
		msg += "Changes will be lost if you don't save them.";
		int result = QMessageBox::warning(this, "Open Workspace", msg,
						  QMessageBox::Yes,
						  QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
		{
			/* Save first */
			slotFileSave();
		}
		else if (result == QMessageBox::No)
		{
			/* Nah, who cares? */
		}
		else
		{
			/* Whoops, go back! */
			return;
		}
	}

	fileName = QFileDialog::getOpenFileName(m_doc->fileName(), 
						QString("*") + KExtWorkspace,
						this);
	
	if (fileName.isEmpty() == false)
	{
		newDocument();
		
		if (doc()->loadXML(fileName) == false)
		{
			QMessageBox::critical(this,
					      "Unable to open file!",
					      "The file seems to be corrupt.");
		}
	}

	doc()->resetModified();
}

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

void App::slotFileSaveDefaults()
{
	int r = 0;

	r = QMessageBox::question(this, "Save Defaults?",
				  QString("Do you wish to save defaults for:\n")
				  + QString("* Workspace background & theme\n")
				  + QString("* Universe mapping"),
				  QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		m_workspace->saveDefaults(KApplicationNameLong);
		m_dmxMap->saveDefaults(KApplicationNameLong);
	}
}

void App::slotFileQuit()
{
	close();
}
