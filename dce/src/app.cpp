/*
  Q Light Controller - Device Class Editor
  app.cpp

  Copyright (C) Heikki Junnila

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
#include "aboutbox.h"

#include <unistd.h>

#include "app.h"

#include "../../main/src/deviceclass.h"
#include "../../main/src/settings.h"
#include "../../main/src/configkeys.h"

#include "../../libs/common/filehandler.h"

#include "deviceclasseditor.h"

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
// Edit menu entries                    
#define ID_EDIT_ADD_CHANNEL             11000
#define ID_EDIT_REMOVE_CHANNEL          11010
#define ID_EDIT_CHANNEL                 11020

#define ID_EDIT_RAISE_CHANNEL           11030
#define ID_EDIT_LOWER_CHANNEL           11040

#define ID_EDIT_ADD_CAPABILITY          11050
#define ID_EDIT_REMOVE_CAPABILITY       11060
#define ID_EDIT_CAPABILITY              11070

///////////////////////////////////////////////////////////////////
// View menu entries                    
#define ID_VIEW_TOOLBAR       	        12010
#define ID_VIEW_STATUSBAR		12020

///////////////////////////////////////////////////////////////////
// Window menu entries
#define ID_WINDOW_MENU			14000
#define ID_WINDOW_CASCADE		14010
#define ID_WINDOW_TILE			14020

///////////////////////////////////////////////////////////////////
// Help menu entries
#define ID_HELP                         1000
#define ID_HELP_ABOUT               	1001
#define ID_HELP_ABOUT_QT                1002

//////////////////////////////////////////////////////////////////
// Status bar messages
#define IDS_STATUS_DEFAULT              "Ready"

//////////////////////////////////////////////////////////////////
// Configuration keys
const QString KApplicationRectX("DCE_ApplicationRectX");
const QString KApplicationRectY("DCE_ApplicationRectY");
const QString KApplicationRectW("DCE_ApplicationRectW");
const QString KApplicationRectH("DCE_ApplicationRectH");

App::App(Settings* settings)
{
  m_workspace = NULL;

  ASSERT(settings != NULL);
  m_settings = settings;
}

App::~App()
{
  delete m_workspace;
}

void App::initView(void)
{
  resize(640, 480);

  initSettings();

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(QPixmap(dir + QString("/Q.xpm")));

  initWorkspace();

  initMenuBar();
  initStatusBar();
  initToolBar();
  
  QString path;
  settings()->get(KEY_SYSTEM_DIR, m_lastPath);
  m_lastPath += QString("/") + DEVICECLASSPATH;
}

void App::initStatusBar()
{
}


void App::initToolBar()
{
  m_toolbar = new QToolBar(this, "Workspace");

  QString dir;
  settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  new QToolButton(QPixmap(dir + QString("/filenew.xpm")), "New...",
		  0, this, SLOT(slotFileNew()), m_toolbar);

  new QToolButton(QPixmap(dir + QString("/fileopen.xpm")), "Load...",
		  0, this, SLOT(slotFileOpen()), m_toolbar);

  new QToolButton(QPixmap(dir + QString("/filesave.xpm")), "Save",
		  0, this, SLOT(slotFileSave()), m_toolbar);
}


bool App::event(QEvent* e)
{
  return QWidget::event(e);
}


void App::initSettings()
{
  QString x, y, w, h;
  settings()->get(KApplicationRectX, x);
  settings()->get(KApplicationRectY, y);
  settings()->get(KApplicationRectW, w);
  settings()->get(KApplicationRectH, h);

  // Set the main window geometry
  setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());
}


void App::initWorkspace()
{
  m_workspace = new QWorkspace(this, "Main Workspace");
  setCentralWidget(m_workspace);

  QString path;
  settings()->get(KEY_APP_BACKGROUND, path);

  // Set background picture
  m_workspace->setBackgroundPixmap(QPixmap(path));
}


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

  m_fileMenu->insertItem("Save As...", this, SLOT(slotFileSaveAs()), 
			 0, ID_FILE_SAVE_AS);

  m_fileMenu->insertSeparator();

  m_fileMenu->insertItem(QPixmap(dir + QString("/exit.xpm")),
			 "E&xit", this, SLOT(slotFileQuit()), 
			 CTRL+Key_Q, ID_FILE_QUIT);

  ///////////////////////////////////////////////////////////////////
  // Edit Menu
  m_editMenu = new QPopupMenu();
  m_editMenu->insertItem(QPixmap(dir + QString("/add.xpm")),
			 "Add &Channel...", this, SLOT(slotEmpty()),
			 CTRL+Key_C, ID_EDIT_ADD_CHANNEL);

  m_editMenu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			 "&Remove Channel", this, SLOT(slotEmpty()),
			 CTRL+Key_R, ID_EDIT_REMOVE_CHANNEL);

  m_editMenu->insertItem(QPixmap(dir + QString("/edit.xpm")),
			 "Edit C&hannel...", this, SLOT(slotEmpty()),
			 CTRL+Key_H, ID_EDIT_CHANNEL);

  m_editMenu->insertItem(QPixmap(dir + QString("/up.xpm")),
			 "Raise Channel", this, SLOT(slotEmpty()),
			 CTRL+Key_Up, ID_EDIT_RAISE_CHANNEL);

  m_editMenu->insertItem(QPixmap(dir + QString("/down.xpm")),
			 "Lower Channel", this, SLOT(slotEmpty()),
			 CTRL+Key_Down, ID_EDIT_LOWER_CHANNEL);

  m_editMenu->insertSeparator();

  m_editMenu->insertItem(QPixmap(dir + QString("/add.xpm")),
			 "&Add Capability...", this, SLOT(slotEmpty()),
			 CTRL+Key_A, ID_EDIT_ADD_CAPABILITY);

  m_editMenu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			 "R&emove Capability", this, SLOT(slotEmpty()),
			 CTRL+Key_E, ID_EDIT_REMOVE_CAPABILITY);

  m_editMenu->insertItem(QPixmap(dir + QString("/edit.xpm")),
			 "Ed&it Capability...", this, SLOT(slotEmpty()),
			 CTRL+Key_I, ID_EDIT_CAPABILITY);

  connect(m_editMenu, SIGNAL(activated(int)),
	  this, SLOT(slotEditMenuActivated(int)));
  
  ///////////////////////////////////////////////////////////////////
  // Window Menu
  m_windowMenu = new QPopupMenu();
  connect(m_windowMenu, SIGNAL(aboutToShow()), 
	  this, SLOT(slotRefreshWindowMenu()));
  
  ///////////////////////////////////////////////////////////////////
  // Help menu
  m_helpMenu = new QPopupMenu();
  m_helpMenu->insertItem(QPixmap(dir + QString("/help.xpm")),
			 "About...", this, SLOT(slotHelpAbout()), 
			 0, ID_HELP_ABOUT);

  m_helpMenu->insertItem(QPixmap(dir + QString("/qt.xpm")),
			 "About Qt...", this, SLOT(slotHelpAboutQt()), 
			 0, ID_HELP_ABOUT_QT);

  ///////////////////////////////////////////////////////////////////
  // Menubar configuration
  menuBar()->insertItem("File", m_fileMenu);
  menuBar()->insertItem("Edit", m_editMenu);
  menuBar()->insertItem("Window", m_windowMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem("Help", m_helpMenu);

  menuBar()->setSeparator(QMenuBar::InWindowsStyle);
}

void App::slotEmpty()
{
}

void App::slotFileNew()
{
  DeviceClass* dc = new DeviceClass();
  ASSERT(dc);

  dc->setManufacturer(QString("Manufacturer"));
  dc->setModel(QString("Model"));

  DeviceClassEditor* editor = new DeviceClassEditor(m_workspace, dc);
  connect(editor, SIGNAL(closed(DeviceClassEditor*)),
	  this, SLOT(slotEditorClosed(DeviceClassEditor*)));
  editor->init();
  editor->setCaption("New Device Class*");
  editor->show();
}


void App::slotFileOpen()
{
  QPtrList <QString> list;

  m_lastPath = QFileDialog::getOpenFileName(m_lastPath, 
					    "Device Classes (*.deviceclass)",
					    this);

  if (m_lastPath != QString::null)
    {
      FileHandler::readFileToList(m_lastPath, list);

      DeviceClass* dc = DeviceClass::createDeviceClass(list);
      
      if (!dc)
	{
	  QMessageBox::warning(this, KApplicationNameShort,
			       "File didn't contain a valid device class.");
	}
      else
	{
	  DeviceClassEditor* editor = new DeviceClassEditor(m_workspace, dc);
	  editor->setFileName(m_lastPath);
	  connect(editor, SIGNAL(closed(DeviceClassEditor*)),
		  this, SLOT(slotEditorClosed(DeviceClassEditor*)));
	  editor->init();
	  editor->show();
	}
    }
}


void App::slotFileSave()
{
  DeviceClassEditor* editor = NULL;
  editor = static_cast<DeviceClassEditor*> (m_workspace->activeWindow());

  if (editor && editor->save())
    {
      m_lastPath = editor->fileName();
    }
}


void App::slotFileSaveAs()
{
  DeviceClassEditor* editor = NULL;
  editor = static_cast<DeviceClassEditor*> (m_workspace->activeWindow());

  if (editor && editor->saveAs())
    {
      m_lastPath = editor->fileName();
    }
}


void App::slotFileQuit()
{
  close();
}


void App::slotEditMenuActivated(int id)
{
  DeviceClassEditor* editor = NULL;
  editor = static_cast<DeviceClassEditor*> (m_workspace->activeWindow());

  if (editor)
    {
      switch(id)
	{
	case ID_EDIT_ADD_CHANNEL:
	  editor->slotAddChannelClicked();
	  break;

	case ID_EDIT_REMOVE_CHANNEL:
	  editor->slotRemoveChannelClicked();
	  break;

	case ID_EDIT_CHANNEL:
	  editor->slotEditChannelClicked();
	  break;

	case ID_EDIT_ADD_CAPABILITY:
	  editor->slotAddPresetClicked();
	  break;

	case ID_EDIT_REMOVE_CAPABILITY:
	  editor->slotRemovePresetClicked();
	  break;

	case ID_EDIT_CAPABILITY:
	  editor->slotEditPresetClicked();	
	  break;

	default:
	  break;
	}
    }

}


void App::slotRefreshWindowMenu()
{
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
  m_windowMenu->insertItem(QPixmap(dir + QString("/tilewindow.xpm")),
			   "Tile", this, SLOT(slotWindowTile()),
			   0, ID_WINDOW_TILE);
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

  connect(m_windowMenu, SIGNAL(activated(int)), 
	  this, SLOT(slotWindowMenuCallback(int)));
}


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
	  QMessageBox::critical(this, KApplicationNameShort, 
				"Unable to focus window! Handle not found.");
	}

      disconnect(m_windowMenu);
    }
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
  AboutBox* ab = NULL;
  ab = new AboutBox(this);
  ab->exec();
  delete ab;
}


void App::slotHelpAboutQt()
{
  QMessageBox::aboutQt(this, KApplicationNameLong);
}

void App::slotEditorClosed(DeviceClassEditor* editor)
{
  delete editor;
}

void App::closeEvent(QCloseEvent* e)
{
  DeviceClassEditor* editor = NULL;

  e->accept();

  QWidgetList wl = workspace()->windowList();
  for (unsigned int i = 0; i < wl.count(); i++)
    {
      editor = static_cast<DeviceClassEditor*> (wl.at(i));
      if (editor == NULL)
	{
	  qDebug("Strange...");
	}
      else
	{
	  editor->show();
	  editor->setFocus();
	  if ( !editor->close() )
	    {
	      e->ignore();
	    }
	}
    }

  // Save main window geometry for next session 
  m_settings->set(KApplicationRectX, rect().x());
  m_settings->set(KApplicationRectY, rect().y());
  m_settings->set(KApplicationRectW, rect().width());
  m_settings->set(KApplicationRectH, rect().height());
  m_settings->save();
}
