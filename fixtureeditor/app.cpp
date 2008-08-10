/*
  Q Light Controller - Fixture Definition Editor
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

#include <QMdiSubWindow>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QWidgetList>
#include <QMessageBox>
#include <QSettings>
#include <QMdiArea>
#include <QMenuBar>
#include <QToolBar>
#include <QToolTip>
#include <QAction>
#include <QLabel>
#include <QColor>
#include <QMenu>
#include <QIcon>

#include <iostream>

#include "common/qlcdocbrowser.h"
#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"

#include "app.h"
#include "aboutbox.h"
#include "fixtureeditor.h"

const QString KFixtureFilter ("Fixtures (*.qxf)");

App::App(QWidget* parent) : QMainWindow(parent)
{
	QCoreApplication::setOrganizationName("qlc");
	QCoreApplication::setOrganizationDomain("sf.net");
	QCoreApplication::setApplicationName("Q Light Controller");

	m_fileMenu = NULL;
	m_helpMenu = NULL;
	m_toolBar = NULL;

	m_copyChannel = NULL;

	QSettings s;
	m_lastPath = s.value("directories/fixtures").toString();

	setWindowTitle(KApplicationName);
	setWindowIcon(QIcon(":/qlc-fixtureeditor.png"));
	setCentralWidget(new QMdiArea(this));

	initActions();
	initMenuBar();
	initToolBar();

	loadDefaults();
}

App::~App()
{
	saveDefaults();
	setCopyChannel(NULL);
}

void App::loadDefaults()
{
	QSettings settings(KApplicationName, KApplicationName);
	QPoint pos = settings.value(KApplicationName + "/mainwindow/position", 
				    QPoint(0, 0)).toPoint();
	QSize size = settings.value(KApplicationName + "/mainwindow/size",
				    QSize(800, 600)).toSize();
	resize(size);
	move(pos);
}

void App::saveDefaults()
{
	QSettings settings(KApplicationName, KApplicationName);
	settings.setValue(KApplicationName + "/mainwindow/position", pos());
	settings.setValue(KApplicationName + "/mainwindow/size", size());
}

void App::closeEvent(QCloseEvent* e)
{
	/* Accept the close event by default */
	e->accept();

	QListIterator <QMdiSubWindow*> it(
		qobject_cast<QMdiArea*> (centralWidget())->subWindowList());
	while (it.hasNext() == true)
	{
		QLCFixtureEditor* editor;
		QMdiSubWindow* sub;

		sub = it.next();
		Q_ASSERT(sub != NULL);

		editor = static_cast<QLCFixtureEditor*> (sub->widget());
		Q_ASSERT(editor != NULL);

		editor->show();
		editor->setFocus();

		if (editor->close() == false)
		{
			/* Ignore the close event if just one editor refuses */
			e->ignore();
			break;
		}
	}
}

/*****************************************************************************
 * Copy channel
 *****************************************************************************/

void App::setCopyChannel(QLCChannel* ch)
{
	if (m_copyChannel != NULL)
		delete m_copyChannel;
	m_copyChannel = NULL;

	if (ch != NULL)
		m_copyChannel = new QLCChannel(ch);
}

QLCChannel* App::copyChannel() const
{
	return m_copyChannel;
}

/*****************************************************************************
 * Actions, toolbar & menubar
 *****************************************************************************/

void App::initActions()
{
	/* File actions */
	m_fileNewAction = new QAction(QIcon(":/filenew.png"),
				      tr("New"), this);
	connect(m_fileNewAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileNew()));

	m_fileOpenAction = new QAction(QIcon(":/fileopen.png"),
				       tr("Open"), this);
	m_fileOpenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	connect(m_fileOpenAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileOpen()));

	m_fileSaveAction = new QAction(QIcon(":/filesave.png"),
				       tr("Save"), this);
	m_fileSaveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(m_fileSaveAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSave()));

	m_fileSaveAsAction = new QAction(tr("Save As"), this);
	m_fileSaveAsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT +
						     Qt::Key_S));
	connect(m_fileSaveAsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSaveAs()));

	m_fileQuitAction = new QAction(QIcon(":/exit.png"),
				       tr("Quit"), this);
	m_fileQuitAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
	connect(m_fileQuitAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileQuit()));

	/* Help actions */
	m_helpIndexAction = new QAction(QIcon(":/help.png"),
					tr("Index"), this);
	connect(m_helpIndexAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpIndex()));

	m_helpAboutAction = new QAction(QIcon(":/qlc.png"),
					tr("About..."), this);
	connect(m_helpAboutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAbout()));

	m_helpAboutQtAction = new QAction(tr("About Qt..."), this);
	connect(m_helpAboutQtAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAboutQt()));
}

void App::initToolBar()
{
	m_toolBar = new QToolBar(KApplicationName, this);
	addToolBar(m_toolBar);
	m_toolBar->setMovable(false);

	m_toolBar->addAction(m_fileNewAction);
	m_toolBar->addAction(m_fileOpenAction);
	m_toolBar->addAction(m_fileSaveAction);
}

void App::initMenuBar()
{
	/* File Menu */
	m_fileMenu = new QMenu(menuBar());
	m_fileMenu->setTitle(tr("File"));
	m_fileMenu->addAction(m_fileNewAction);
	m_fileMenu->addAction(m_fileOpenAction);
	m_fileMenu->addAction(m_fileSaveAction);
	m_fileMenu->addAction(m_fileSaveAsAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_fileQuitAction);

	/* Help menu */
	m_helpMenu = new QMenu(menuBar());
	m_helpMenu->setTitle(tr("Help"));
	m_helpMenu->addAction(m_helpIndexAction);
	m_helpMenu->addAction(m_helpAboutAction);
	m_helpMenu->addAction(m_helpAboutQtAction);

	menuBar()->addMenu(m_fileMenu);
	menuBar()->addMenu(m_helpMenu);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

void App::slotFileNew()
{
	QLCFixtureEditor* editor;
	QMdiSubWindow* sub;

	sub = new QMdiSubWindow(centralWidget());
	editor = new QLCFixtureEditor(sub, new QLCFixtureDef());

	sub->setWidget(editor);
	sub->setAttribute(Qt::WA_DeleteOnClose);
	sub->setWindowIcon(QIcon(":/fixture.png"));

	qobject_cast<QMdiArea*> (centralWidget())->addSubWindow(sub);

	editor->show();
	sub->show();
}

void App::slotFileOpen()
{
	QLCFixtureEditor* editor;
	QLCFixtureDef* fixtureDef;	
	QMdiSubWindow* sub;
	QString path;

	path = QFileDialog::getOpenFileName(this,
					    tr("Open a fixture definition"),
					    m_lastPath, KFixtureFilter);
	if (path == QString::null)
		return;
	else
		m_lastPath = path;
	
	fixtureDef = new QLCFixtureDef(path);
	if (fixtureDef == NULL)
	{
		QMessageBox::warning(this, tr("Fixture loading failed"),
				     tr("File didn't contain a valid fixture."));
	}
	else
	{
		sub = new QMdiSubWindow(centralWidget());
		editor = new QLCFixtureEditor(sub, fixtureDef, path);
		
		sub->setWidget(editor);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		qobject_cast<QMdiArea*> (centralWidget())->addSubWindow(sub);
		
		editor->show();
		sub->show();
	}
}

void App::slotFileSave()
{
	QLCFixtureEditor* editor;
	QMdiSubWindow* sub;

	sub = (qobject_cast<QMdiArea*> (centralWidget()))->activeSubWindow();
	if (sub == NULL)
		return;
	
	editor = static_cast<QLCFixtureEditor*> (sub->widget());
	if (editor == NULL)
		return;
	
	if (editor->save() == true)
		m_lastPath = editor->fileName();
}

void App::slotFileSaveAs()
{
	QLCFixtureEditor* editor;
	QMdiSubWindow* sub;

	sub = (qobject_cast<QMdiArea*> (centralWidget()))->activeSubWindow();
	if (sub == NULL)
		return;
	
	editor = static_cast<QLCFixtureEditor*> (sub->widget());
	if (editor == NULL)
		return;
	
	if (editor->saveAs() == true)
		m_lastPath = editor->fileName();
}

void App::slotFileQuit()
{
	close();
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
	QLCDocBrowser browser(NULL);
	browser.setAttribute(Qt::WA_DeleteOnClose);
}

void App::slotHelpAbout()
{
	AboutBox aboutbox(this);
	aboutbox.exec();
}

void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, KApplicationName);
}
