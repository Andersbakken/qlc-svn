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

#include <assert.h>
#include <unistd.h>

#include "app.h"
#include "aboutbox.h"

#include "common/qlcworkspace.h"
#include "common/settings.h"
#include "common/documentbrowser.h"
#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "fixtureeditor.h"
#include "configkeys.h"

// Old headers that will be removed in the future
#include "common/deviceclass.h"
#include "common/filehandler.h"

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
#define ID_HELP_INDEX			1010
#define ID_HELP_ABOUT               	1020
#define ID_HELP_ABOUT_QT                1030

//////////////////////////////////////////////////////////////////
// Status bar messages
#define IDS_STATUS_DEFAULT              "Ready"

//////////////////////////////////////////////////////////////////
// Configuration keys

const QString KApplicationRectX("DCE_ApplicationRectX");
const QString KApplicationRectY("DCE_ApplicationRectY");
const QString KApplicationRectW("DCE_ApplicationRectW");
const QString KApplicationRectH("DCE_ApplicationRectH");

const QString KFixtureFilter ( "Fixtures (*.qxf *.deviceclass)" );

App::App(Settings* settings) :
	m_settings         ( settings ),
	m_workspace        ( NULL ),
	m_documentBrowser  ( NULL ),
	m_fileMenu         ( NULL ),
	m_toolsMenu        ( NULL ),
	m_windowMenu       ( NULL ),
	m_helpMenu         ( NULL ),
	m_toolbar          ( NULL ),
	m_copyChannel      ( NULL )
{
}

App::~App()
{
	if (m_workspace)
		delete m_workspace;
	m_workspace = NULL;

	if (m_documentBrowser)
		delete m_documentBrowser;
	m_documentBrowser = NULL;

	if (m_settings)
		delete m_settings;
	m_settings = NULL;

	if (m_copyChannel)
		delete m_copyChannel;
	m_copyChannel = NULL;
}

void App::initView(void)
{
	initSettings();

	setIcon(QPixmap(QString(PIXMAPS) + QString("/qlc-fixtureeditor.png")));

	initWorkspace();

	initMenuBar();
	initStatusBar();
	initToolBar();

	m_lastPath = QString(FIXTURES);
}

void App::initStatusBar()
{
}


void App::initToolBar()
{
	m_toolbar = new QToolBar(this, "Workspace");
	m_toolbar->setMovingEnabled(false);

	new QToolButton(QPixmap(QString(PIXMAPS) + QString("/filenew.png")),
			"New...", 0, this, SLOT(slotFileNew()), m_toolbar);

	new QToolButton(QPixmap(QString(PIXMAPS) + QString("/fileopen.png")),
			"Load...", 0, this, SLOT(slotFileOpen()), m_toolbar);

	new QToolButton(QPixmap(QString(PIXMAPS) + QString("/filesave.png")),
			"Save",	0, this, SLOT(slotFileSave()), m_toolbar);
}


void App::setCopyChannel(QLCChannel* ch)
{
	if (m_copyChannel != NULL)
		delete m_copyChannel;
	m_copyChannel = NULL;

	if (ch != NULL)
		m_copyChannel = new QLCChannel(ch);
}

bool App::event(QEvent* e)
{
	return QWidget::event(e);
}


void App::initSettings()
{
	QString x, y, w, h;
	if (settings()->get(KApplicationRectX, x) == -1)
	{
		x = QString("0");
	}
	if (settings()->get(KApplicationRectY, y) == -1)
	{
		y = QString("0");
	}
	if (settings()->get(KApplicationRectW, w) == -1)
	{
		w = QString("640");
	}
	if (settings()->get(KApplicationRectH, h) == -1)
	{
		h = QString("480");
	}

	// Set the main window geometry
	setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());
}


void App::initWorkspace()
{
	m_workspace = new QLCWorkspace(this);
	setCentralWidget(m_workspace);

	QString path;
	settings()->get(KEY_APP_BACKGROUND, path);

	m_workspace->setBackground(path);

	connect(m_workspace, SIGNAL(backgroundChanged(const QString&)),
		this, SLOT(slotBackgroundChanged(const QString&)));
}


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

	m_fileMenu->insertItem("Save As...", this, SLOT(slotFileSaveAs()),
			       0, ID_FILE_SAVE_AS);

	m_fileMenu->insertSeparator();

	m_fileMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/exit.png")),
			       "E&xit", this, SLOT(slotFileQuit()),
			       CTRL+Key_Q, ID_FILE_QUIT);

	///////////////////////////////////////////////////////////////////
	// Window Menu
	m_windowMenu = new QPopupMenu();
	connect(m_windowMenu, SIGNAL(aboutToShow()),
		this, SLOT(slotRefreshWindowMenu()));

	///////////////////////////////////////////////////////////////////
	// Help menu
	m_helpMenu = new QPopupMenu();
	m_helpMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/help.png")),
			       "Index...", this, SLOT(slotHelpIndex()),
			       SHIFT + Key_F1, ID_HELP_INDEX);
	m_helpMenu->insertSeparator();
	m_helpMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/Q.png")),
			       "About...", this, SLOT(slotHelpAbout()),
			       0, ID_HELP_ABOUT);

	m_helpMenu->insertItem("About Qt...", this, SLOT(slotHelpAboutQt()),
			       0, ID_HELP_ABOUT_QT);

	///////////////////////////////////////////////////////////////////
	// Menubar configuration
	menuBar()->insertItem("File", m_fileMenu);
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
	QLCFixtureDef* fixtureDef = new QLCFixtureDef();

	QLCFixtureEditor* editor = new QLCFixtureEditor(m_workspace, fixtureDef);
	connect(editor, SIGNAL(closed(QLCFixtureEditor*)),
		this, SLOT(slotEditorClosed(QLCFixtureEditor*)));
	editor->init();
	editor->show();
}


void App::slotFileOpen()
{
	QLCFixtureEditor* editor = NULL;
	QLCFixtureDef* fixtureDef = NULL;
	QString path;

	path = QFileDialog::getOpenFileName(m_lastPath, KFixtureFilter, this);

	// Save the last path so that the next file dialog starts from
	// the same place, unless the path is empty
	if (path == QString::null)
		return;
	else
		m_lastPath = path;
	
	if (path.right(strlen(KExtFixture)) == KExtFixture)
	{
		fixtureDef = new QLCFixtureDef(path);
	}
	else if (path.right(strlen(KExtLegacyDeviceClass)) 
		 == KExtLegacyDeviceClass)
	{
		/* Open as an old DC but convert it to a QLCFixture */
		DeviceClass* dc = openLegacyFile(path);
		fixtureDef = new QLCFixtureDef(dc);
		delete dc;
	}

	if (fixtureDef == NULL)
	{
		QMessageBox::warning(this, "Fixture loading failed",
				     "File didn't contain a valid fixture.");
	}
	else
	{
		editor = new QLCFixtureEditor(m_workspace, fixtureDef);
		editor->setFileName(path);
		
		connect(editor, SIGNAL(closed(QLCFixtureEditor*)),
			this, SLOT(slotEditorClosed(QLCFixtureEditor*)));

		editor->init();
		editor->show();
	}
}

DeviceClass* App::openLegacyFile(QString path)
{
	DeviceClass* dc = NULL; 
	QPtrList <QString> list;

	FileHandler::readFileToList(m_lastPath, list);

	// Attempt to read & create a device class from list
	dc = DeviceClass::createDeviceClass(list);

	return dc;
}


void App::slotFileSave()
{
	QLCFixtureEditor* editor = NULL;
	editor = static_cast<QLCFixtureEditor*> (m_workspace->activeWindow());
	if (editor != NULL && editor->save() == true)
	{
		// Save the last path so that the next file
		// dialog starts from the same place
		m_lastPath = editor->fileName();
	}
}


void App::slotFileSaveAs()
{
	QLCFixtureEditor* editor = NULL;
	editor = static_cast<QLCFixtureEditor*> (m_workspace->activeWindow());
	if (editor && editor->saveAs())
	{
		// Save the last path so that the next file dialog starts from there
		m_lastPath = editor->fileName();
	}
}


void App::slotFileQuit()
{
	close();
}


void App::slotRefreshWindowMenu()
{
	QWidget* widget;
	int id = 0;

	QPtrList <QWidget> wl = workspace()->windowList();

	m_windowMenu->clear();
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_sidetree.png")),
				 "Cascade", this, SLOT(slotWindowCascade()),
				 0, ID_WINDOW_CASCADE);
	m_windowMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/view_left_right.png")),
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
			QMessageBox::critical(this,
					      "Unable to select window",
					      "Handle not found");
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

//
// Document browser window has been closed
//
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
	AboutBox* ab = NULL;
	ab = new AboutBox(this);
	ab->exec();
	delete ab;
}

//
// Help -> About QT
//
void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, KApplicationNameLong);
}

//
// An editor window has been closed
//
void App::slotEditorClosed(QLCFixtureEditor* editor)
{
	delete editor;
}

void App::slotBackgroundChanged(const QString& path)
{
	m_settings->set(KEY_APP_BACKGROUND, path);
}

//
// File -> Exit or Window destroyed
//
void App::closeEvent(QCloseEvent* e)
{
	QLCFixtureEditor* editor = NULL;

	e->accept();

	QWidgetList wl = workspace()->windowList();
	for (unsigned int i = 0; i < wl.count(); i++)
	{
		editor = static_cast<QLCFixtureEditor*> (wl.at(i));
		assert(editor);

		editor->show();
		editor->setFocus();
		if ( !editor->close() )
		{
			e->ignore();
		}
	}

	// Save main window geometry for next session
	m_settings->set(KApplicationRectX, rect().x());
	m_settings->set(KApplicationRectY, rect().y());
	m_settings->set(KApplicationRectW, rect().width());
	m_settings->set(KApplicationRectH, rect().height());
	m_settings->save();
}
