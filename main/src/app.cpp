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

#include <unistd.h>

#include "app.h"
#include "doc.h"
#include "settings.h"
#include "devicemanagerview.h"
#include "deviceclasseditor.h"
#include "virtualconsole.h"
#include "aboutbox.h"
#include "dmxaddresstool.h"
#include "sequenceprovider.h"
#include "sequencetimer.h"
#include "globalfunctionsview.h"
#include "functiontree.h"

#include "../../libs/common/outputplugin.h"

static const QString KModeTextOperate = QString("Operate");
static const QString KModeTextDesign = QString("Design");

static const QColor KModeColorOperate = QColor(255, 0, 0);
static const QColor KModeColorDesign = QColor(0, 255, 0);

///////////////////////////////////////////////////////////////////
// File menu entries
#define ID_FILE_NEW                 	10020
#define ID_FILE_OPEN                	10030
#define ID_FILE_SAVE                	10050
#define ID_FILE_SAVE_AS             	10060
#define ID_FILE_CLOSE               	10070
#define ID_FILE_PRINT               	10080
#define ID_FILE_SETTINGS                10090
#define ID_FILE_QUIT                	10100

///////////////////////////////////////////////////////////////////
// Tools menu entries                    
#define ID_VIEW_TOOLBAR       	        12010
#define ID_VIEW_STATUSBAR		12020

#define ID_VIEW_DEVICE_MANAGER		12030
#define ID_VIEW_DEVICE_CLASS_EDITOR	12035
#define ID_VIEW_VIRTUAL_CONSOLE         12040
#define ID_VIEW_SEQUENCE_EDITOR         12050
#define ID_VIEW_DMXADDRESSTOOL          12060
#define ID_VIEW_INPUT_DEVICES           12070

///////////////////////////////////////////////////////////////////
// Functions menu entries
#define ID_FUNCTIONS_GLOBAL_FUNCTIONS       13010

#define ID_FUNCTIONS_PANIC                  13030

///////////////////////////////////////////////////////////////////
// Window menu entries
#define ID_WINDOW_MENU			14000
#define ID_WINDOW_CASCADE		14010
#define ID_WINDOW_TILE			14020

///////////////////////////////////////////////////////////////////
// Help menu entries
#define ID_HELP_ABOUT               	1002
#define ID_HELP_ABOUT_QT                1003

//////////////////////////////////////////////////////////////////
// Status bar messages
#define IDS_STATUS_DEFAULT              "Ready"

App::App()
{
  m_globalFunctionsView = NULL;
  m_sequenceTimer = NULL;
  m_sequenceProvider = NULL;
  m_dmView = NULL;
  m_modeIndicator = NULL;
  m_virtualConsole = NULL;
  m_doc = NULL;
  m_workspace = NULL;
}

App::~App()
{
  m_sequenceTimer->stop();

  delete m_sequenceProvider;
  delete m_globalFunctionsView;
  delete m_dmView;
  delete m_modeIndicator;
  delete m_virtualConsole;
  delete m_doc;
  delete m_workspace;
}

void App::initView(void)
{
  initSettings();
  initDoc();
  initSequenceEngine();

  setIcon(QPixmap(settings()->pixmapPath() + QString("/Q.xpm")));

  initWorkspace();
  connect(m_settings, SIGNAL(outputPluginChanged(const QString&)), m_doc, SLOT(slotChangeOutputPlugin(const QString&)));

  initMenuBar();
  initStatusBar();
  initToolBar();

  initVirtualConsole();
  initDeviceManagerView();

  if (m_settings->openLastWorkspace() == true)
    {
      QString fileName = m_settings->lastWorkspaceFileName();

      doc()->loadWorkspaceAs(fileName);
      setCaption(IDS_APP_NAME_LONG + QString(" - ") + doc()->workspaceFileName());
      virtualConsole()->hide();
    }
}

void App::initStatusBar()
{
  statusBar()->message("Q Light Controller");

  m_modeIndicator = new QLabel("Design Mode", statusBar());
  statusBar()->addWidget(m_modeIndicator, 0, true);
}

void App::initToolBar()
{
  QPixmap pixmap;

  m_toolbar = new QToolBar(this, "Workspace Toolbar");

  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("filenew.xpm")), "New workspace; clear everything", 0, this, SLOT(slotFileNew()), m_toolbar);
  
  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("fileopen.xpm")), "Open existing workspace", 0, this, SLOT(slotFileOpen()), m_toolbar);
  
  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("filesave.xpm")), "Save current workspace", 0, this, SLOT(slotFileSave()), m_toolbar);

  m_toolbar->addSeparator();

  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("device.xpm")), "View device manager", 0, this, SLOT(slotViewDeviceManager()), m_toolbar);

  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("virtualconsole.xpm")), "View virtual console", 0, this, SLOT(slotViewVirtualConsole()), m_toolbar);

  QToolBar* panic = new QToolBar(this, "!");
  new QToolButton(QPixmap(m_settings->pixmapPath() + QString("panic.xpm")), "Panic; Shut down all running functions", 0, this, SLOT(slotPanic()), panic);
}

void App::initSequenceEngine()
{
  m_sequenceProvider = new SequenceProvider();
  m_sequenceProvider->init();

  ASSERT(m_sequenceProvider);

  ASSERT(m_sequenceTimer);

  m_sequenceTimer->setSequenceProvider(m_sequenceProvider);
  m_sequenceTimer->start();
}

void App::setSequenceTimer(SequenceTimer* timer)
{
  ASSERT(timer != NULL);
  m_sequenceTimer = timer;
}

bool App::event(QEvent* e)
{
  return QWidget::event(e);
}

void App::initDoc()
{
  m_doc = new Doc();
  m_doc->init();
}

Doc* App::doc(void)
{
  return m_doc;
}

void App::initSettings()
{
  m_settings = new Settings();
  m_settings->load();

  // Set the main window geometry
  setGeometry(m_settings->appRect());
}

void App::initWorkspace()
{
  m_workspace = new QWorkspace(this, "Main Workspace");
  setCentralWidget(m_workspace);

  // Set background picture
  m_workspace->setBackgroundPixmap(QPixmap(m_settings->workspaceBackgroundPath()));
}

void App::initDeviceManagerView()
{
  // Create but don't show dm
  m_dmView = new DeviceManagerView(workspace());
  m_dmView->initView();
  m_dmView->resize(500, 300);

  if (m_settings->openDeviceManager() == true)
    {
      m_dmView->show();
      m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, true);
    }
  else
    {
      m_dmView->hide();
      m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, false);
    }

  connect(m_dmView, SIGNAL(closed()), this, SLOT(slotDeviceManagerViewClosed()));
  connect(m_doc, SIGNAL(deviceListChanged()), m_dmView->deviceManager(), SLOT(slotUpdateDeviceList()));
}

void App::initVirtualConsole(void)
{
  m_virtualConsole = new VirtualConsole(workspace());
  m_virtualConsole->initView();
  m_virtualConsole->resize(400, 400);
  m_virtualConsole->hide();
  connect(m_virtualConsole, SIGNAL(closed()), this, SLOT(slotVirtualConsoleClosed()));
}

void App::initMenuBar()
{
  ///////////////////////////////////////////////////////////////////
  // File Menu
  m_fileMenu = new QPopupMenu();
  m_fileMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("filenew.xpm")),
			 "&New", this, SLOT(slotFileNew()), CTRL+Key_N, ID_FILE_NEW);
  m_fileMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("fileopen.xpm")), 
			 "&Open...", this, SLOT(slotFileOpen()), CTRL+Key_O, ID_FILE_OPEN);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("filesave.xpm")),
			 "&Save", this, SLOT(slotFileSave()), CTRL+Key_S, ID_FILE_SAVE);
  m_fileMenu->insertItem("Save &As...", this, SLOT(slotFileSaveAs()), 0, ID_FILE_SAVE_AS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("info.xpm")),
			 "Se&ttings...", this, SLOT(slotFileSettings()), 0, ID_FILE_SETTINGS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("exit.xpm")),
			 "E&xit", this, SLOT(slotFileQuit()), CTRL+Key_Q, ID_FILE_QUIT);
  
  ///////////////////////////////////////////////////////////////////
  // View Menu
  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->setCheckable(true);
  m_toolsMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("device.xpm")), 
			  "Device Manager", this, SLOT(slotViewDeviceManager()), CTRL + Key_D, ID_VIEW_DEVICE_MANAGER);
  m_toolsMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("virtualconsole.xpm")), 
			  "Virtual Console", this, SLOT(slotViewVirtualConsole()), CTRL + Key_G, ID_VIEW_VIRTUAL_CONSOLE);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("deviceclasseditor.xpm")), 
			  "Device Class Editor", this, SLOT(slotViewDeviceClassEditor()), CTRL + Key_E, ID_VIEW_DEVICE_CLASS_EDITOR);
  m_toolsMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("function.xpm")), 
			  "Functions", this, SLOT(slotViewGlobalFunctions()), CTRL + Key_F, ID_FUNCTIONS_GLOBAL_FUNCTIONS);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("panic.xpm")), "Panic!", this, SLOT(slotPanic()), CTRL + Key_C, ID_FUNCTIONS_PANIC);
  connect(m_toolsMenu, SIGNAL(aboutToShow()), this, SLOT(slotRefreshToolsMenu()));

  ///////////////////////////////////////////////////////////////////
  // Window Menu
  m_windowMenu = new QPopupMenu();
  connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotRefreshWindowMenu()));
  
  ///////////////////////////////////////////////////////////////////
  // Help menu
  m_helpMenu = new QPopupMenu();
  m_helpMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("help.xpm")),
			 "About QLC...", this, SLOT(slotHelpAbout()), 0, ID_HELP_ABOUT);
  m_helpMenu->insertItem(QPixmap(m_settings->pixmapPath() + QString("qt.xpm")),
			 "About Qt...", this, SLOT(slotHelpAboutQt()), 0, ID_HELP_ABOUT_QT);

  ///////////////////////////////////////////////////////////////////
  // Menubar configuration
  menuBar()->insertItem("&File", m_fileMenu);
  menuBar()->insertItem("&Tools", m_toolsMenu);
  menuBar()->insertItem("&Window", m_windowMenu);
  menuBar()->insertSeparator();  	
  menuBar()->insertItem("&Help", m_helpMenu);

  menuBar()->setSeparator(QMenuBar::InWindowsStyle);
}

void App::slotViewGlobalFunctions()
{
  FunctionTree* ft = new FunctionTree(this, false);
  ft->exec();
  delete ft;
}

void App::slotGlobalFunctionsViewClosed()
{
  if (m_globalFunctionsView)
    {
      disconnect(m_globalFunctionsView);
      delete m_globalFunctionsView;
      m_globalFunctionsView = NULL;
    }
}

void App::slotViewDeviceClassEditor()
{
  DeviceClassEditor* dce = new DeviceClassEditor(NULL);

  dce->init();
  dce->exec();

  delete dce;
}

void App::slotRefreshWindowMenu()
{
  QWidget* widget;
  int id = 0;

  QList <QWidget> wl = workspace()->windowList();

  m_windowMenu->clear();
  m_windowMenu->insertItem(QPixmap(settings()->pixmapPath() + QString("cascadewindow.xpm")), "Cascade", this, SLOT(slotWindowCascade()), 0, ID_WINDOW_CASCADE);
  m_windowMenu->insertItem(QPixmap(settings()->pixmapPath() + QString("tilewindow.xpm")), "Tile", this, SLOT(slotWindowTile()), 0, ID_WINDOW_TILE);
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

  connect(m_windowMenu, SIGNAL(activated(int)), this, SLOT(slotWindowMenuCallback(int)));
}

void App::slotRefreshToolsMenu()
{
  if (virtualConsole()->isDesignMode() == false)
    {
      m_toolsMenu->setItemEnabled(ID_FUNCTIONS_GLOBAL_FUNCTIONS, false);
      m_toolsMenu->setItemEnabled(ID_VIEW_DEVICE_CLASS_EDITOR, false);
    }
  else
    {
      m_toolsMenu->setItemEnabled(ID_FUNCTIONS_GLOBAL_FUNCTIONS, true);
      m_toolsMenu->setItemEnabled(ID_VIEW_DEVICE_CLASS_EDITOR, true);
    }
}
	
void App::slotWindowMenuCallback(int item)
{
  QList <QWidget> wl = workspace()->windowList();

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
	  QMessageBox::critical(this, IDS_APP_NAME_SHORT, "Unable to focus window! Handle not found.");
	}
      
      disconnect(m_windowMenu);
    }
}

void App::slotPanic()
{
  /* Shut down all running functions */
  m_sequenceProvider->flush();
}

void App::slotFileNew()
{
  if (doc()->isModified())
    {
      if (QMessageBox::warning(this, "QLC", "Are you sure you want to clear the current workspace & virtual console?", "&Yes", "&Cancel", 0, 1) == 1)
	{
	  return;
	}
    }

  m_settings->setLastWorkspaceFileName(QString::null);
  doc()->newDocument();
  virtualConsole()->newDocument();
  setCaption(IDS_APP_NAME_LONG);
}

void App::slotFileOpen()
{
  QString fn = QFileDialog::getOpenFileName(m_doc->workspaceFileName(), "*.qlc", this);

  if (fn == QString::null)
    {
      return;
    }

  if (doc()->loadWorkspaceAs(fn) == false)
    {
      statusBar()->message("Load failed", 2000);
      QMessageBox::critical(this, IDS_APP_NAME_SHORT, "Errors occurred while reading file. Data may be lost.");
    }
  else
    {
      m_settings->setLastWorkspaceFileName(m_doc->workspaceFileName());
      statusBar()->message("Load successful", 2000);
      setCaption(IDS_APP_NAME_LONG + QString(" - ") + doc()->workspaceFileName());
    }
}

void App::slotFileSave()
{
  statusBar()->message("Saving current workspace...");
  if (m_doc->workspaceFileName().isEmpty())
    {
      slotFileSaveAs();
    }
  else
    {
      if (m_doc->saveWorkspace() == false)
        {
          statusBar()->message("Save failed", 2000);
          QMessageBox::warning(this, IDS_APP_NAME_SHORT, "An error occurred while attempting to save configuration.\nData may be lost.");
	}
      else
        {
          statusBar()->message("Save successful", 2000);
        }
    }
  statusBar()->message(IDS_STATUS_DEFAULT);
}

void App::slotFileSaveAs()
{
  statusBar()->message("Saving file under new filename...");

  QString fn = QFileDialog::getSaveFileName(m_doc->workspaceFileName(), "*.qlc", this);
  if (!fn.isEmpty())
    {
      // Use the suffix ".qlc" always
      if (fn.right(4) != ".qlc")
        {
          fn += ".qlc";
        }

      if (m_doc->saveWorkspaceAs(fn) == false)
        {
          statusBar()->message("Save aborted", 2000);
          QMessageBox::information(this, IDS_APP_NAME_LONG, "An error occurred while attempting to save current workspace!");
	}
      else
        {
          statusBar()->message("Save successful", 2000);
	  setCaption(IDS_APP_NAME_LONG + QString(" - ") + doc()->workspaceFileName());
        }
    }
  else
    {
      statusBar()->message("Save aborted", 2000);
    }

  statusBar()->message(IDS_STATUS_DEFAULT);
}

void App::slotFileSettings()
{
  m_settings->load();
  m_settings->showUI();
}

void App::slotFileQuit()
{
  close();
}

void App::slotViewDeviceManager()
{
  m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, true);
  m_dmView->show();
  m_dmView->setFocus();
}

void App::slotDeviceManagerViewClosed()
{
  m_toolsMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, false);
}

void App::slotViewVirtualConsole()
{
  m_toolsMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, true);
  m_virtualConsole->show();
  m_virtualConsole->setFocus();
}

void App::slotVirtualConsoleClosed()
{
  m_toolsMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, false);
}

void App::slotViewToolBar()
{

}

void App::slotViewStatusBar()
{

}

void App::slotWindowCascade()
{
  workspace()->cascade();
}

void App::slotWindowTile()
{
  workspace()->tile();
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

void App::closeEvent(QCloseEvent* e)
{
  int result;

  statusBar()->message("About to close the application...");

  if (m_doc->isModified())
    {
      result = QMessageBox::information(this, "Exit...", "Do you want to save changes before exit?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
      if (result == QMessageBox::Yes)
        {
	  // Save main window geometry for next session 
	  m_settings->setAppRect(rect());
	  m_settings->save();
	  slotFileSave();
	  m_settings->setLastWorkspaceFileName(m_doc->workspaceFileName());
	  e->accept();
	  // close();
        }
      else if (result == QMessageBox::No)
        {
	  // Save main window geometry for next session 
	  m_settings->setAppRect(rect());
	  m_settings->save();
	  m_settings->setLastWorkspaceFileName(m_doc->workspaceFileName());
	  e->accept();
	  // close();
        }
      else if (result == QMessageBox::Cancel)
	{
	  e->ignore();
	}
    }
  else
    {
      // Save main window geometry for next session 
      m_settings->setAppRect(rect());
      m_settings->save();
      m_settings->setLastWorkspaceFileName(m_doc->workspaceFileName());
      e->accept();
      // close();
    }

  statusBar()->message("Exit aborted", 2000);
}

void App::slotSetModeIndicator(VirtualConsole::Mode mode)
{
  switch(mode)
    {
    case VirtualConsole::Design:
      m_modeIndicator->setText(KModeTextDesign);
      break;

    case VirtualConsole::Operate:
      m_modeIndicator->setText(KModeTextOperate);
      break;
      
    default:
      break;
    }
}
