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

#include <unistd.h>

#include "app.h"
#include "doc.h"
#include "settings.h"
#include "devicemanagerview.h"
#include "deviceclasseditor.h"
#include "virtualconsole.h"
#include "aboutbox.h"
#include "dmxaddresstool.h"
#include "sequenceeditor.h"
#include "sequenceprovider.h"
#include "sequencetimer.h"
#include "globalfunctionsview.h"
#include "inputdeviceview.h"

App::App()
{
  m_globalFunctionsView = NULL;
  m_inputDeviceView = NULL;
  initSettings();
}

App::~App()
{
  m_sequenceTimer->stop();
}

void App::initView(void)
{
  setIcon(settings()->getPixmapPath() + QString("/Q.xpm"));

  initWorkspace();
  initDoc();
  doc()->readDeviceClasses();
  
  initMenuBar();
  initStatusBar();
  initToolBar();

  initDeviceManagerView();
  initVirtualConsole();

  initSequenceEngine();
}

void App::initSequenceEngine()
{
  m_sequenceProvider = new SequenceProvider();
  m_sequenceTimer = new SequenceTimer(m_sequenceProvider);
  m_sequenceTimer->start();
}

bool App::event(QEvent* e)
{
  return QWidget::event(e);
}

void App::initDoc()
{
  m_doc = new Doc();
  m_doc->initDMX();
}

Doc* App::doc(void)
{
  return m_doc;
}

void App::initSettings()
{
  m_settings = new Settings(this, "Settings");
  m_settings->load();

  // Set the main window geometry
  setGeometry(m_settings->getAppRect());
}

void App::initWorkspace()
{
  m_workspace = new QWorkspace(this, "Main Workspace");
  setCentralWidget(m_workspace);

  // Set a fancy background pixmap
  m_workspace->setBackgroundPixmap(m_settings->getPixmapPath() + QString("background.xpm"));
}

void App::initDeviceManagerView()
{
  // Create but don't show dm
  m_dmView = new DeviceManagerView(workspace());
  m_dmView->initView();
  m_viewMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, false);
  m_dmView->resize(500, 300);
  m_dmView->hide();

  connect(m_dmView, SIGNAL(closed()), this, SLOT(slotDeviceManagerViewClosed()));
  connect(m_doc, SIGNAL(deviceListChanged()), m_dmView->deviceManager(), SLOT(slotUpdateDeviceList()));
}

void App::initVirtualConsole(void)
{
  // Create but don't show vc
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
  m_fileMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("filenew.xpm")), "&New", this, SLOT(slotFileNew()), CTRL+Key_N, ID_FILE_NEW);
  m_fileMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("fileopen.xpm")), "&Open...", this, SLOT(slotFileOpen()), CTRL+Key_O, ID_FILE_OPEN);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("filesave.xpm")), "&Save", this, SLOT(slotFileSave()), CTRL+Key_S, ID_FILE_SAVE);
  m_fileMenu->insertItem("Save &As...", this, SLOT(slotFileSaveAs()), 0, ID_FILE_SAVE_AS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("settings.xpm")), "Se&ttings...", this, SLOT(slotFileSettings()), 0, ID_FILE_SETTINGS);
  m_fileMenu->insertSeparator();
  m_fileMenu->insertItem("E&xit", this, SLOT(slotFileQuit()), CTRL+Key_Q, ID_FILE_QUIT);
  
  ///////////////////////////////////////////////////////////////////
  // View Menu
  m_viewMenu = new QPopupMenu();

  m_viewMenu->setCheckable(true);
  m_viewMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("device.xpm")), "Device Manager", this, SLOT(slotViewDeviceManager()), 0, ID_VIEW_DEVICE_MANAGER);
  m_viewMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("virtualconsole.xpm")), "Virtual Console", this, SLOT(slotViewVirtualConsole()), 0, ID_VIEW_VIRTUAL_CONSOLE);
  m_viewMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("deviceclasseditor.xpm")), "Device Class Editor", this, SLOT(slotViewDeviceClassEditor()), 0, ID_VIEW_DEVICE_CLASS_EDITOR);
  //
  // Sequence Editor is disabled for now because it doesn't work at all
  //
  // m_viewMenu->insertItem("Sequence Editor", this, SLOT(slotViewSequenceEditor()), 0, ID_VIEW_SEQUENCE_EDITOR);
  //
  //m_viewMenu->insertSeparator();
  //m_viewMenu->insertItem(QIconSet(m_settings->getPixmapPath() + QString("joystick.xpm")), "Input devices", this, SLOT(slotViewInputDevices()), 0, ID_VIEW_INPUT_DEVICES);
  m_viewMenu->insertSeparator();
  m_viewMenu->insertItem("Toolbar", this, SLOT(slotViewToolBar()), 0, ID_VIEW_TOOLBAR);
  m_viewMenu->insertItem("Statusbar", this, SLOT(slotViewStatusBar()), 0, ID_VIEW_STATUSBAR);
  m_viewMenu->insertSeparator();
  m_viewMenu->insertItem("DMX Address Tool", this, SLOT(slotViewDMXAddressTool()), 0, ID_VIEW_DMXADDRESSTOOL);

  m_viewMenu->setItemChecked(ID_VIEW_TOOLBAR, true);
  m_viewMenu->setItemChecked(ID_VIEW_STATUSBAR, true);

  ///////////////////////////////////////////////////////////////////
  // Functions Menu
  m_functionsMenu = new QPopupMenu();
  
  m_functionsMenu->insertItem("Global Functions", this, SLOT(slotViewGlobalFunctions()), 0, ID_VIEW_GLOBAL_FUNCTIONS);

  ///////////////////////////////////////////////////////////////////
  // Window Menu
  m_windowMenu = new QPopupMenu();
  connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotRefreshWindowMenu()));
  
  ///////////////////////////////////////////////////////////////////
  // Help menu
  m_helpMenu = new QPopupMenu();
  m_helpMenu->insertItem("About...", this, SLOT(slotHelpAbout()), 0, ID_HELP_ABOUT);

  ///////////////////////////////////////////////////////////////////
  // Menubar configuration
  menuBar()->insertItem("&File", m_fileMenu);
  menuBar()->insertItem("&View", m_viewMenu);
  menuBar()->insertItem("F&unctions", m_functionsMenu);
  menuBar()->insertItem("&Window", m_windowMenu);
  menuBar()->insertSeparator();  	
  menuBar()->insertItem("&Help", m_helpMenu);

  menuBar()->setSeparator(QMenuBar::InWindowsStyle);
}

void App::slotViewGlobalFunctions()
{
  if (m_globalFunctionsView == NULL)
    {
      m_globalFunctionsView = new GlobalFunctionsView(workspace());
      connect(m_globalFunctionsView, SIGNAL(closed()), this, SLOT(slotGlobalFunctionsViewClosed()));
      connect(m_doc, SIGNAL(deviceListChanged()), m_globalFunctionsView, SLOT(slotUpdateFunctionList()));
      m_globalFunctionsView->show();
    }
  m_globalFunctionsView->setFocus();
}

void App::slotViewInputDevices()
{
  if (m_inputDeviceView == NULL)
    {
      m_inputDeviceView = new InputDeviceView(workspace());
      connect(m_inputDeviceView, SIGNAL(closed()), this, SLOT(slotInputDeviceViewClosed()));
      m_inputDeviceView->show();
    }
  m_inputDeviceView->setFocus();
}

void App::slotInputDeviceViewClosed()
{
  if (m_inputDeviceView)
    {
      disconnect(m_inputDeviceView);
      delete m_inputDeviceView;
      m_inputDeviceView = NULL;
    }
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
  DeviceClassEditor* dCE = new DeviceClassEditor(NULL, "Device Class Editor", true);
  dCE->show();
}
void App::slotViewSequenceEditor()
{
  SequenceEditor* se;
  se = new SequenceEditor(workspace());
  se->show();
  se->setFocus();
}

void App::slotSequenceEditorClosed()
{
}

void App::slotViewDMXAddressTool()
{
  DMXAddressTool* d = new DMXAddressTool(this);
  d->exec();
  delete d;
}

void App::slotRefreshWindowMenu()
{
  QWidget* widget;
  int id = 0;

  QList <QWidget> wl = workspace()->windowList();

  m_windowMenu->clear();
  m_windowMenu->insertItem("Cascade", this, SLOT(slotWindowCascade()), 0, ID_WINDOW_CASCADE);
  m_windowMenu->insertItem("Tile", this, SLOT(slotWindowTile()), 0, ID_WINDOW_TILE);
  m_windowMenu->insertSeparator();

  for (widget = wl.first(); widget != NULL; widget = wl.next())
  {
    m_windowMenu->insertItem(widget->caption(), id);
    id++;
  }

  connect(m_windowMenu, SIGNAL(activated(int)), this, SLOT(slotWindowMenuCallback(int)));
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
	  widget->setFocus();
	}
      else
	{
	  QMessageBox::critical(this, IDS_APP_NAME_SHORT, "Unable to focus window! Handle not found.");
	}
      
      disconnect(m_windowMenu);
    }
}

void App::initStatusBar()
{
  statusBar()->message("Q Light Controller");
}

void App::initToolBar()
{
  QPixmap pixmap;

  m_toolbar = new QToolBar(this, "Workspace Toolbar");

  new QToolButton(QPixmap(m_settings->getPixmapPath() + QString("filenew.xpm")), "New Workspace", 0, this, SLOT(slotFileNew()), m_toolbar);
  
  new QToolButton(QPixmap(m_settings->getPixmapPath() + QString("fileopen.xpm")), "Open Workspace", 0, this, SLOT(slotFileOpen()), m_toolbar);
  
  new QToolButton(QPixmap(m_settings->getPixmapPath() + QString("filesave.xpm")), "Save Workspace", 0, this, SLOT(slotFileSave()), m_toolbar);

  m_toolbar->addSeparator();

  new QToolButton(QPixmap(m_settings->getPixmapPath() + QString("device.xpm")), "Device Manager", 0, this, SLOT(slotViewDeviceManager()), m_toolbar);

  new QToolButton(QPixmap(m_settings->getPixmapPath() + QString("virtualconsole.xpm")), "Virtual Console", 0, this, SLOT(slotViewVirtualConsole()), m_toolbar);
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

  doc()->newDocument();
  virtualConsole()->newDocument();
  setCaption(IDS_APP_NAME_LONG);
}

void App::slotFileOpen()
{
  QString fn = QFileDialog::getOpenFileName(m_doc->workspaceFileName(), "*.qlc", this);

  if (doc()->loadWorkspaceAs(fn) == false)
    {
      statusBar()->message("Load failed", 2000);
      MSG_CRIT("Errors occurred while reading file. Data may be lost.");
    }
  else
    {
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
          MSG_WARN("An error occurred while attempting to save configuration.\nData may be lost.");
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
  m_settings->show();
}

void App::slotFileQuit()
{
  close();
}

void App::slotViewDeviceManager()
{
  m_viewMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, true);
  m_dmView->show();
  m_dmView->setFocus();
}

void App::slotDeviceManagerViewClosed()
{
  m_viewMenu->setItemChecked(ID_VIEW_DEVICE_MANAGER, false);
}

void App::slotViewVirtualConsole()
{
  m_viewMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, true);
  m_virtualConsole->show();
  m_virtualConsole->setFocus();
}

void App::slotVirtualConsoleClosed()
{
  m_viewMenu->setItemChecked(ID_VIEW_VIRTUAL_CONSOLE, false);
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
	  e->accept();
	  // close();
        }
      else if (result == QMessageBox::No)
        {
	  // Save main window geometry for next session 
	  m_settings->setAppRect(rect());
	  m_settings->save();
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
      e->accept();
      // close();
    }

  statusBar()->message("Exit aborted", 2000);
}




