/*
  Q Light Controller
  app.cpp
  
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
#include <qlabel.h>
#include <qcolor.h>
#include <qtooltip.h>
#include <qstylefactory.h>
#include <qrect.h>

#include <unistd.h>
#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "app.h"
#include "doc.h"

#include "functionconsumer.h"

#include "settings.h"
#include "settingsui.h"
#include "configkeys.h"

#include "devicemanagerview.h"

#include "virtualconsole.h"
#include "dmxaddresstool.h"
#include "functiontree.h"
#include "busproperties.h"

#include "documentbrowser.h"
#include "aboutbox.h"

#include "dummyoutplugin.h"
#include "../../libs/common/outputplugin.h"
#include "../../libs/common/plugin.h"

#include "../../libs/common/filehandler.h"

static const QString KModeTextOperate = QString("Operate");
static const QString KModeTextDesign = QString("Design");

static const QColor KModeColorOperate = QColor(255, 0, 0);
static const QColor KModeColorDesign = QColor(0, 255, 0);

t_plugin_id App::NextPluginID = KPluginIDMin;

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
#define ID_FILE_QUIT                	10080

///////////////////////////////////////////////////////////////////
// Tools menu entries                    
#define ID_VIEW_TOOLBAR       	        12010
#define ID_VIEW_STATUSBAR		12020

#define ID_VIEW_DEVICE_MANAGER		12030
#define ID_VIEW_VIRTUAL_CONSOLE         12040
#define ID_VIEW_SEQUENCE_EDITOR         12050
#define ID_VIEW_DMXADDRESSTOOL          12060
#define ID_VIEW_INPUT_DEVICES           12070
#define ID_VIEW_FUNCTION_TREE           12080
#define ID_VIEW_BUS_PROPERTIES          12090

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

App::App() : QMainWindow()
{
  m_functionTree = NULL;
  m_busProperties = NULL;
  m_dmView = NULL;
  m_modeIndicator = NULL;
  m_virtualConsole = NULL;
  m_doc = NULL;
  m_workspace = NULL;
  m_functionConsumer = NULL;
  m_outputPlugin = NULL;
  m_dummyOutPlugin = NULL;
  m_settings = NULL;
  m_mode = Design;
}

App::~App()
{
  delete m_functionConsumer;
  delete m_dmView;
  delete m_virtualConsole;
  delete m_doc;
  delete m_modeIndicator;
  delete m_workspace;
  delete m_settings;
}


//////////////////////////////////////////////////////////////////////
// Main initialization function                                     //
//                                                                  //
// This creates all items that are not saved in workspace files     //
//////////////////////////////////////////////////////////////////////
void App::init(void)
{
  //
  // Default size
  //
  resize(maximumSize());

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
  // Plugins
  //
  initPlugins();

  //
  // Device classes
  //
  initDeviceClasses();

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
  // Views
  //
  initDeviceManagerView();
  initVirtualConsole();

  //
  // Load the previous workspace
  //
  bool success = false;
  QString config;
  if (settings()->get(KEY_OPEN_LAST_WORKSPACE, config))
    {
      if (config == Settings::trueValue())
	{
	  if (settings()->get(KEY_LAST_WORKSPACE_NAME, config))
	    {
	      doc()->loadWorkspaceAs(config);
              success = true;
	    }
	}
    }

  if (!success)
    {
      newDocument();
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
  //
  // Create workspace
  //
  m_workspace = new QWorkspace(this, "Main Workspace");
  setCentralWidget(m_workspace);
  m_workspace->setScrollBarsEnabled(true);

  //
  // Set background picture
  //
  QString path;
  settings()->get(KEY_APP_BACKGROUND, path);
  m_workspace->setBackgroundPixmap(QPixmap(path));

  //
  // Set App proportions
  //
  QString max;
  settings()->get(KEY_APP_MAXIMIZED, max);
  if (max == Settings::trueValue())
    {
      showMaximized();
    }
  else
    {
      QString x, y, w, h;
      settings()->get(KEY_APP_X, x);
      settings()->get(KEY_APP_Y, y);
      settings()->get(KEY_APP_W, w);
      settings()->get(KEY_APP_H, h);
      setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());
    }

  //
  // Tooltips
  //
  QString tips;
  settings()->get(KEY_APP_SHOW_TOOLTIPS, tips);
  if (tips == Settings::falseValue())
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
  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;
  setIcon(QPixmap(dir + QString("/Q.xpm")));

  //
  // Get the widget style from settings
  //
  QString widgetStyle;
  m_settings->get(KEY_WIDGET_STYLE, widgetStyle);

  //
  // Construct the style thru stylefactory and set it if it's valid
  //
  QStyleFactory f;
  QStyle* style = f.create(widgetStyle);
  if (style != NULL)
    {
      _qapp.setStyle(style);
    }
}


//
// Menu bar
//
void App::initMenuBar()
{
  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  ///////////////////////////////////////////////////////////////////
  // File Menu
  m_fileMenu = new QPopupMenu();
  m_fileMenu->insertItem(QPixmap(dir + QString("/filenew.xpm")),
			 "&New", this, SLOT(slotFileNew()),
			 CTRL+Key_N, ID_FILE_NEW);
  m_fileMenu->insertItem(QPixmap(dir + QString("/fileopen.xpm")), 
			 "&Open...", this, SLOT(slotFileOpen()), 
			 CTRL+Key_O, ID_FILE_OPEN);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(dir + QString("/filesave.xpm")),
			 "&Save", this, SLOT(slotFileSave()), 
			 CTRL+Key_S, ID_FILE_SAVE);
  m_fileMenu->insertItem("Save &As...", this, SLOT(slotFileSaveAs()), 
			 0, ID_FILE_SAVE_AS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(dir + QString("/settings.xpm")),
			 "Se&ttings...", this, SLOT(slotFileSettings()), 
			 0, ID_FILE_SETTINGS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(dir + QString("/exit.xpm")),
			 "E&xit", this, SLOT(slotFileQuit()), 
			 CTRL+Key_Q, ID_FILE_QUIT);
  
  connect(m_fileMenu, SIGNAL(aboutToShow()), 
	  this, SLOT(slotRefreshMenus()));

  ///////////////////////////////////////////////////////////////////
  // Tools Menu
  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->setCheckable(true);
  m_toolsMenu->insertItem(QPixmap(dir + QString("/device.xpm")),
			  "Device Manager", this,
			  SLOT(slotViewDeviceManager()),
			  CTRL + Key_M, ID_VIEW_DEVICE_MANAGER);
  m_toolsMenu->insertItem(QPixmap(dir + QString("/virtualconsole.xpm")), 
			  "Virtual Console", this, 
			  SLOT(slotViewVirtualConsole()), 
			  CTRL + Key_V, ID_VIEW_VIRTUAL_CONSOLE);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(dir + QString("/function.xpm")), 
			  "Function Tree", this, SLOT(slotViewFunctionTree()), 
			  CTRL + Key_F, ID_VIEW_FUNCTION_TREE);
  m_toolsMenu->insertItem(QPixmap(dir + QString("/bus.xpm")),
			  "Bus Properties", this, 
			  SLOT(slotViewBusProperties()), CTRL + Key_B,
			  ID_VIEW_BUS_PROPERTIES);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(dir + QString("/panic.xpm")), "Panic!", 
			  this, SLOT(slotPanic()), CTRL + Key_P,
			  ID_FUNCTIONS_PANIC);

  connect(m_toolsMenu, SIGNAL(aboutToShow()), 
	  this, SLOT(slotRefreshMenus()));

  ////////////////////////////////////////////////////////////////////
  // Mode menu
  m_modeMenu = new QPopupMenu();
  m_modeMenu->setCheckable(true);
  m_modeMenu->insertItem(QPixmap(dir + QString("/unlocked.xpm")),
			 "Design", this,
			 SLOT(slotSetDesignMode()), CTRL + Key_D,
			 ID_FUNCTIONS_MODE_DESIGN);

  m_modeMenu->insertItem(QPixmap(dir + QString("/locked.xpm")),
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
  m_helpMenu->insertItem(QPixmap(dir + QString("/help.xpm")),
			 "Index...", this, SLOT(slotHelpIndex()),
			 SHIFT + Key_F1, ID_HELP_INDEX);
  m_helpMenu->insertSeparator();
  m_helpMenu->insertItem(QPixmap(dir + QString("/Q.xpm")),
			 "About...", this, SLOT(slotHelpAbout()),
			 0, ID_HELP_ABOUT);
  m_helpMenu->insertItem(QPixmap(dir + QString("/qt.xpm")),
			 "About Qt...", this, SLOT(slotHelpAboutQt()),
			 0, ID_HELP_ABOUT_QT);
  m_helpMenu->insertSeparator();
  m_helpMenu->insertItem(QPixmap(dir + QString("")),
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
  m_modeIndicator = new QLabel(KModeTextDesign, statusBar());
  statusBar()->addWidget(m_modeIndicator, 0, true);
}


//
// Create & fill toolbar
//
void App::initToolBar()
{
  m_toolbar = new QToolBar(this, "Workspace");

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;
  
  m_newTB = new QToolButton(QPixmap(dir + QString("/filenew.xpm")), 
			    "New workspace; clear everything", 0, 
			    this, SLOT(slotFileNew()), m_toolbar);

  m_openTB = new QToolButton(QPixmap(dir + QString("/fileopen.xpm")), 
			     "Open existing workspace", 0, 
			     this, SLOT(slotFileOpen()), m_toolbar);

  m_saveTB = new QToolButton(QPixmap(dir + QString("/filesave.xpm")), 
			     "Save current workspace", 0, 
			     this, SLOT(slotFileSave()), m_toolbar);

  m_toolbar->addSeparator();

  m_dmTB = new QToolButton(QPixmap(dir + QString("/device.xpm")), 
			   "View device manager", 0, 
			   this, SLOT(slotViewDeviceManager()), m_toolbar);

  m_vcTB = new QToolButton(QPixmap(dir + QString("/virtualconsole.xpm")), 
			   "View virtual console", 0, 
			   this, SLOT(slotViewVirtualConsole()), m_toolbar);
  
  m_panicTB = new QToolButton(QPixmap(dir + QString("/panic.xpm")), 
			      "Panic; Shut down all running functions", 0, 
			      this, SLOT(slotPanic()), m_toolbar);

  // The pixmap needs to be changed in this button -> save its pointer
  m_modeTB = new QToolButton(QPixmap(dir + QString("/unlocked.xpm")), 
			     "Design Mode; All editing features enabled", 
			     0, this, SLOT(slotSetMode()), m_toolbar);
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
  if (m_doc)
    {
      delete m_doc;
    }

  m_doc = new Doc();
}


//
// Device manager
//
void App::initDeviceManagerView()
{
  if (m_dmView)
    {
      delete m_dmView;
    }

  // Create device manager view
  m_dmView = new DeviceManagerView(workspace());
  m_dmView->initView();

  connect(m_dmView, SIGNAL(closed()), 
  	  this, SLOT(slotDeviceManagerViewClosed()));

  connect(m_doc, SIGNAL(deviceListChanged()),
  	  m_dmView, SLOT(slotUpdate()));
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
  initDeviceManagerView();
  initVirtualConsole();
  
  //
  // Set the last workspace name
  //
  m_settings->set(KEY_LAST_WORKSPACE_NAME, QString::null);
  m_doc->setModified(false);
}


//
// Open an existing document
//
void App::slotFileOpen()
{
  bool ok = true;

  if (doc()->isModified())
    {
      QString msg;
      msg = "Are you sure you want to clear the current workspace?\n";
      msg += "There are unsaved changes.";
      if (QMessageBox::warning(this, KApplicationNameShort, msg,
			       QMessageBox::Yes, QMessageBox::No)
	  == QMessageBox::No)
	{
	  ok = false;
	}
    }

  if (ok)
    {
      QString fn = QFileDialog::getOpenFileName(m_doc->fileName(), 
						"*.qlc", this);
      if (fn == QString::null)
	{
	  return;
	}
      else
	{
	  newDocument();
	  if (doc()->loadWorkspaceAs(fn) == false)
	    {
	      QMessageBox::critical(this, KApplicationNameShort, 
				    "Errors occurred while reading file.");
	    }
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
      if (m_doc->saveWorkspace() == false)
        {
          QMessageBox::warning(this, KApplicationNameShort, 
			       "Unable to save file!");
	}
    }
}


//
// Save current workspace, ask for a file name
//
void App::slotFileSaveAs()
{
  QString fn = QFileDialog::getSaveFileName(m_doc->fileName(), "*.qlc", this);
  if (!fn.isEmpty())
    {
      // Use the suffix ".qlc" always
      if (fn.right(4) != ".qlc")
        {
          fn += ".qlc";
        }

      if (m_doc->saveWorkspaceAs(fn) == false)
        {
          QMessageBox::information(this, KApplicationNameShort, 
				   "Unable to save file!");
	}
      else
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
// Quit the program
//
void App::slotFileQuit()
{
  close();
}


//
// View device manager
//
void App::slotViewDeviceManager()
{
  m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, true);
  m_dmView->show();
  m_dmView->setFocus();
}


//
// Device manager was closed, uncheck menu item
//
void App::slotDeviceManagerViewClosed()
{
  m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, false);
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
// View function tree
//
void App::slotViewFunctionTree()
{
  if (m_functionTree == NULL)
    {
      m_functionTree = new FunctionTree(this, false);
      connect(m_functionTree, SIGNAL(closed()),
	      this, SLOT(slotFunctionTreeClosed()));
    }
  else
    {
      // Hide the ft first so that it pops on top
      m_functionTree->hide();
    }

  m_functionTree->show();
}


//
// Function tree was closed, delete the instance
//
void App::slotFunctionTreeClosed()
{
  if (m_functionTree)
    {
      disconnect(m_functionTree);
      delete m_functionTree;
      m_functionTree = NULL;
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

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;
  
  m_windowMenu->clear();
  m_windowMenu->insertItem(QPixmap(dir + QString("/cascadewindow.xpm")), 
			   "Cascade", this, SLOT(slotWindowCascade()), 
			   0, ID_WINDOW_CASCADE);
  m_windowMenu->insertItem(QPixmap(dir + QString("/tilewindow.xpm")), "Tile", 
			   this, SLOT(slotWindowTile()), 0, ID_WINDOW_TILE);
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
      m_documentBrowser = new DocumentBrowser();
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
  int result;

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
	  saveSettings();
	  e->accept();
        }
      else if (result == QMessageBox::No)
        {
	  saveSettings();
	  e->accept();
        }
      else if (result == QMessageBox::Cancel)
	{
	  e->ignore();
	}
    }
  else
    {
      saveSettings();
      e->accept();
    }
}


//
// Save some App-related global settings
//
void App::saveSettings()
{
  //
  // Main window geometry
  // 
  m_settings->set(KEY_APP_X, rect().x());
  m_settings->set(KEY_APP_Y, rect().y());
  m_settings->set(KEY_APP_W, rect().width());
  m_settings->set(KEY_APP_H, rect().height());

  m_settings->set(KEY_APP_MAXIMIZED, (isMaximized()) ? Settings::trueValue() :
		  Settings::falseValue());
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
  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

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
      m_modeTB->setPixmap(dir + QString("/unlocked.xpm"));
      QToolTip::add(m_modeTB, "Design mode; All edit features available");

      m_newTB->setEnabled(true);
      m_openTB->setEnabled(true);
      m_panicTB->setEnabled(false);

      m_mode = Design;
    }
  else
    {
      m_modeIndicator->setText(KModeTextOperate);
      m_modeTB->setPixmap(dir + QString("/locked.xpm"));
      QToolTip::add(m_modeTB, "Operate mode; Edit features disabled");

      m_newTB->setEnabled(false);
      m_openTB->setEnabled(false);
      m_panicTB->setEnabled(true);

      m_mode = Operate;
    }

  emit modeChanged();
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

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PLUGINPATH + QString("/");

  // First of all, add the dummy output plugin
  m_dummyOutPlugin = new DummyOutPlugin(NextPluginID++);
  connect(m_dummyOutPlugin, SIGNAL(activated(Plugin*)),
	  this, SLOT(slotPluginActivated(Plugin*)));
  addPlugin(m_dummyOutPlugin);

  QDir d(dir);
  d.setFilter(QDir::Files);
  if (d.exists() == false || d.isReadable() == false)
    {
      fprintf(stderr, "Unable to access plugin directory %s.\n",
	      (const char*) dir);
      return;
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

      path = dir + *it;

      probePlugin(path);
    }

  //
  // Use the output plugin that user has selected previously
  //
  QString config;
  settings()->get(KEY_OUTPUT_PLUGIN, config);
  Plugin* plugin = searchPlugin(config, Plugin::OutputType);
  if (plugin != NULL)
    {
      m_outputPlugin = static_cast<OutputPlugin*> (plugin);
    }
  else
    {
      m_outputPlugin = m_dummyOutPlugin;
    }

  slotPluginActivated(m_outputPlugin);
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


//
// Create joystick plugin contents... now what the heck is this
// doing here???
//
void App::createJoystickContents(QPtrList <QString> &list)
{
  QString name;
  QString fdName;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("FDName"))
	{
	  fdName = *(list.next());
	}
      else if (*s == QString("Name"))
	{
	  name = *(list.next());
	}
    }
}




////////////////////////
// Device Class stuff //
////////////////////////
//
// Read all device classes from files
//
void App::initDeviceClasses()
{
  DeviceClass* dc = NULL;
  QString path = QString::null;

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + DEVICECLASSPATH + QString("/");

  QDir d(dir);
  d.setFilter(QDir::Files);
  d.setNameFilter("*.deviceclass");
  if (d.exists() == false || d.isReadable() == false)
    {
      QString msg("Unable to read from device directory!");
      QMessageBox::warning(this, KApplicationNameShort, msg);

      return;
    }

  QStringList dirlist(d.entryList());
  QStringList::Iterator it;

  QPtrList <QString> list; // Our stringlist that contains the files' contents

  // Put a slash to the end of the directory name if it isn't there
  if (dir.right(1) != QString("/"))
    {
      dir = dir + QString("/");
    }

  // Go thru all files
  for (it = dirlist.begin(); it != dirlist.end(); ++it)
    {
      path = dir + *it;
      FileHandler::readFileToList(path, list);
      dc = DeviceClass::createDeviceClass(list);
      if (dc != NULL)
	{
	  addDeviceClass(dc);
	}

      // The list needs to be cleared between files
      while (list.isEmpty() == false)
	{
	  list.first();
	  delete list.take();
	}
    }
}


//
// Add a device class
//
void App::addDeviceClass(DeviceClass* dc)
{
  assert(dc);
  
  if (searchDeviceClass(dc->manufacturer(), dc->model()) == NULL)
    {
      m_deviceClassList.append(dc);
    }
  else
    {
      qDebug("Cannot add same device class twice");
    }
}

//
// Search for a deviceclass by its manufacturer & model
//
DeviceClass* App::searchDeviceClass(const QString &manufacturer, 
				    const QString &model)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; 
       d = m_deviceClassList.next())
    {
      if (d->manufacturer() == manufacturer && d->model() == model)
	{
	  return d;
	}
    }

  return NULL;
}


//
// Search for a deviceclass by its ID
//
DeviceClass* App::searchDeviceClass(const t_deviceclass_id id)
{
  for (DeviceClass* d = m_deviceClassList.first(); d != NULL; 
       d = m_deviceClassList.next())
    {
      if (d->id() == id)
	{
	  return d;
	}
    }

  return NULL;
}

void App::initSubmasters()
{
  for (t_channel i = 0; i < 512; i++)
    {
      m_submasters[i] = 0;
      m_submasterValues[i] = 1;
    }
}

void App::initValues()
{
  m_outputPlugin->readRange(0, m_values, 512);
}

//
// Set a DMX value
//
void App::setValue(t_channel ch, t_value value)
{
  assert(ch < 512 && ch >= 0);

  m_values[ch] = value;
  
  m_outputPlugin->writeChannel(ch,
			       static_cast<t_value> 
			       (((float) value) * m_submasterValues[ch]));
}

t_value App::value(t_channel ch)
{
  assert(ch < 512 && ch >= 0);

  return m_values[ch];
}

void App::valueRange(t_channel address, t_value* values, t_channel num)
{
  assert(address < 512 && address >= 0);
  memcpy(values, m_values + address, num);
}


////////////////
// Submasters //
////////////////
//
// Indicate that someone is setting submaster value to a channel
//
bool App::assignSubmaster(t_channel ch)
{
  assert(ch < 512 && ch >= 0);

  m_submasters[ch]++;
  return true;
}


//
// Indicate that someone is no longer setting submaster value to a channel
//
bool App::resignSubmaster(t_channel ch)
{
  assert(ch < 512 && ch >= 0);

  m_submasters[ch]--;

  assert(m_submasters[ch] >= 0);

  return true;
}


//
// Check if a channel has a submaster
//
int App::hasSubmaster(t_channel ch)
{
  assert(ch < 512 && ch >= 0);

  return m_submasters[ch];
}


//
// After assign/resign, reset all non-assigned submaster channel values to 100%
//
void App::resetSubmasters()
{
  for (t_channel i = 0; i < 512; i++)
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
  assert(ch < 512 && ch >= 0);

  m_submasterValues[ch] = (float) percent / (float) 100;
  setValue(ch, m_values[ch]);
}


//
// Get the submaster value for a channel
//
int App::submasterValue(t_channel ch)
{
  assert(ch < 512 && ch >= 0);
  
  return static_cast<int> (m_submasterValues[ch]);
}

