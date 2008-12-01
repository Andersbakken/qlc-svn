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

#include <QCoreApplication>
#include <QMdiSubWindow>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStatusBar>
#include <QSettings>
#include <QMdiArea>
#include <QMenuBar>
#include <QToolBar>
#include <QToolTip>
#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QColor>
#include <QTimer>
#include <QMenu>
#include <QRect>
#include <QFile>
#include <QIcon>

#include "functionconsumer.h"
#include "functionmanager.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "outputmanager.h"
#include "inputmanager.h"
#include "busmanager.h"
#include "outputmap.h"
#include "inputmap.h"
#include "aboutbox.h"
#include "monitor.h"
#include "bus.h"
#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

#include "common/qlcdocbrowser.h"
#include "common/qlcfixturedef.h"
#include "common/qlcworkspace.h"
#include "common/qlctypes.h"
#include "common/qlcfile.h"

#define KModeTextOperate tr("Operate")
#define KModeTextDesign tr("Design")

extern App* _app;

/*********************************************************************
 * Initialization
 *********************************************************************/

App::App() : QMainWindow()
{
	m_outputMap = NULL;
	m_inputMap = NULL;
	m_functionConsumer = NULL;
	m_doc = NULL;

	m_inputManager = NULL;
	m_outputManager = NULL;
	m_functionManager = NULL;
	m_busManager = NULL;
	m_fixtureManager = NULL;
	m_virtualConsole = NULL;
	m_docBrowser = NULL;
	m_monitor = NULL;

	m_mode = Design;
	m_modeIndicator = NULL;

	m_blackoutIndicator = NULL;
	m_blackoutIndicatorTimer = NULL;

	QCoreApplication::setOrganizationName("qlc");
	QCoreApplication::setOrganizationDomain("sf.net");
	QCoreApplication::setApplicationName("Q Light Controller");
     
	init();
	slotModeDesign();
	
	setWindowTitle(tr("%1 - New Workspace").arg(KApplicationNameLong));
}

App::~App()
{
	// Delete function tree
	if (m_functionManager != NULL)
		delete m_functionManager;
	m_functionManager = NULL;

	// Delete bus properties
	if (m_busManager != NULL)
		delete m_busManager;
	m_busManager = NULL;

	// Delete monitor
	if (m_monitor != NULL)
		delete m_monitor;
	m_monitor = NULL;
	
	// Delete fixture manager view
	if (m_fixtureManager != NULL)
		delete m_fixtureManager;
	m_fixtureManager = NULL;

	// Delete virtual console
	if (m_virtualConsole != NULL)
		delete m_virtualConsole;
	m_virtualConsole = NULL;

	// Delete doc
	if (m_doc != NULL)
		delete m_doc;
	m_doc = NULL;

	// Delete function consumer
	if (m_functionConsumer != NULL)
		delete m_functionConsumer;
	m_functionConsumer = NULL;

	// Delete mode indicator
	if (m_modeIndicator != NULL)
		delete m_modeIndicator;
	m_modeIndicator = NULL;

	// Delete blackout indicator's timer
	if (m_blackoutIndicatorTimer != NULL)
		delete m_blackoutIndicatorTimer;
	m_blackoutIndicatorTimer = NULL;

	// Delete the blackout indicator
	if (m_blackoutIndicator != NULL)
		delete m_blackoutIndicator;
	m_blackoutIndicator = NULL;

	// Delete document browser
	if (m_docBrowser != NULL)
		delete m_docBrowser;
	m_docBrowser = NULL;

	// Delete fixture definitions
	while (m_fixtureDefList.isEmpty() == false)
		delete m_fixtureDefList.takeFirst();
}


/**
 * Main initialization function
 */
void App::init()
{
	QSettings settings;

	setWindowTitle(KApplicationNameLong);
	setWindowIcon(QIcon(":/qlc.png"));

	/* MDI Area */
	setCentralWidget(new QMdiArea(this));
	centralWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(centralWidget(),
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotCustomContextMenuRequested(const QPoint&)));

	/* Workspace background */
	setBackgroundImage(settings.value("/workspace/background").toString());

	/* Resize the whole application to default size */
	resize(KApplicationDefaultWidth, KApplicationDefaultHeight);

	/* Input & output mappers and their plugins */
	initOutputMap();
	initInputMap();
	
	/* Function running engine */
	initFunctionConsumer();

	/* Buses */
	Bus::init();

	/* Fixture definitions */
#ifdef Q_WS_X11
	/* First, load user fixtures (overrides system fixtures) */
	QDir dir(QString(getenv("HOME")));
	loadFixtureDefinitions(dir.absoluteFilePath(QString(USERFIXTUREDIR)));
#endif
	/* Then, load system fixtures */
	loadFixtureDefinitions(FIXTUREDIR);

	// The main view
	initStatusBar();
	initActions();
	initMenuBar();
	initToolBar();

	// Document
	initDoc();

	// Virtual Console
	initVirtualConsole();

	// Start up in non-modified state
	m_doc->resetModified();
}

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
			"Do you wish to save the current workspace \n"	\
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
 * Output mapping
 *****************************************************************************/

void App::initOutputMap()
{
	m_outputMap = new OutputMap(this, KUniverseCount);
	Q_ASSERT(m_outputMap != NULL);

	connect(m_outputMap, SIGNAL(blackoutChanged(bool)),
		this, SLOT(slotOutputMapBlackoutChanged(bool)));
}

void App::slotOutputMapBlackoutChanged(bool state)
{
	if (state == true)
	{
		m_blackoutIndicator->show();
		m_controlBlackoutAction->setChecked(true);

		connect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			this, SLOT(slotFlashBlackoutIndicator()));
		m_blackoutIndicatorTimer->start(500);
 	}
	else
	{
		m_blackoutIndicator->hide();
		m_controlBlackoutAction->setChecked(false);

		m_blackoutIndicatorTimer->stop();
		disconnect(m_blackoutIndicatorTimer, SIGNAL(timeout()),
			   this, SLOT(slotFlashBlackoutIndicator()));

		m_blackoutIndicator->setPalette(QApplication::palette());
	}
}

void App::slotFlashBlackoutIndicator()
{
	QPalette pal;
	QColor bg;
	QColor fg;

	pal = m_blackoutIndicator->palette();
	bg = pal.color(QPalette::Background);
	bg.setRgb(bg.red() ^ 0xff, bg.green() ^ 0xff, bg.blue() ^ 0xff);
	pal.setColor(QPalette::Background, bg);
	
	fg = pal.color(QPalette::Foreground);
	fg.setRgb(fg.red() ^ 0xff, fg.green() ^ 0xff, fg.blue() ^ 0xff);
	pal.setColor(QPalette::Foreground, fg);

	m_blackoutIndicator->setPalette(pal);
}

void App::slotOutputManager()
{
	if (m_outputManager == NULL)
	{
		QMdiSubWindow* sub;

		sub = new QMdiSubWindow(centralWidget());
		m_outputManager = new OutputManager(sub);
		m_outputManager->show();

		sub->setWidget(m_outputManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowTitle(tr("Output Manager"));
		sub->setWindowIcon(QIcon(":/output.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_outputManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotOutputManagerDestroyed(QObject*)));

		sub->resize(600, 300);
		sub->show();
	}
}

void App::slotOutputManagerDestroyed(QObject*)
{
	m_outputManager = NULL;
}

/*****************************************************************************
 * Input mapping
 *****************************************************************************/

void App::initInputMap()
{
	m_inputMap = new InputMap(this, KInputUniverseCount);
	Q_ASSERT(m_inputMap != NULL);
}

void App::slotInputManager()
{
	if (m_inputManager == NULL)
	{
		QMdiSubWindow* sub;

		sub = new QMdiSubWindow(centralWidget());
		m_inputManager = new InputManager(sub);
		m_inputManager->show();

		sub->setWidget(m_inputManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowTitle(tr("Input Manager"));
		sub->setWindowIcon(QIcon(":/input.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_inputManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotInputManagerDestroyed(QObject*)));

		sub->resize(600, 300);
		sub->show();
	}
}

void App::slotInputManagerDestroyed(QObject*)
{
	m_inputManager = NULL;
}

/*****************************************************************************
 * Function consumer
 *****************************************************************************/

void App::initFunctionConsumer()
{
	Q_ASSERT(m_outputMap != NULL);

	m_functionConsumer = new FunctionConsumer(m_outputMap);
	Q_ASSERT(m_functionConsumer != NULL);

	/* TODO: Put this into some kind of a settings dialog */
	// m_functionConsumer->setTimerType(FunctionConsumer::RTCTimer);
	m_functionConsumer->setTimerType(FunctionConsumer::NanoSleepTimer);
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
	
	connect(this, SIGNAL(modeChanged(App::Mode)),
		m_doc, SLOT(slotModeChanged(App::Mode)));
	
	/* Connect fixture list change signals from the new document object */
	if (m_fixtureManager != NULL)
	{
		connect(m_doc, SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc, SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));
	}
}

void App::slotDocModified(bool state)
{
	QString caption(KApplicationNameLong);

	if (m_doc->fileName() != QString::null)
		caption += QString(" - ") + m_doc->fileName();

	if (state == true)
		setWindowTitle(caption + QString(" *"));
	else
		setWindowTitle(caption);
}

/*****************************************************************************
 * Fixture definitions
 *****************************************************************************/

bool App::loadFixtureDefinitions(QString fixturePath)
{
	QDir dir(fixturePath, QString("*%1").arg(KExtFixture),
		 QDir::Name, QDir::Files);
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load fixture definitions from"
			   << fixturePath;
		return false;
	}

	/* Attempt to read all specified files from the given directory */
	QStringList dirlist(dir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		QString path = QString(fixturePath) + QDir::separator() + *it;
		QLCFixtureDef* fxi = new QLCFixtureDef(path);

		if (fxi != NULL)
		{
			/* Check that there are no duplicates */
			QLCFixtureDef* prev;
			prev = fixtureDef(fxi->manufacturer(), fxi->model());
			if (prev == NULL)
				m_fixtureDefList.append(fxi);
			else
				delete fxi;
		}
		else
		{
			qWarning() << "Fixture definition loading from"
				   << path << "failed. Skipping.";
		}
	}

	return true;
}

QLCFixtureDef* App::fixtureDef(const QString& manufacturer,
			       const QString& model)
{
	QListIterator <QLCFixtureDef*> it(m_fixtureDefList);
	while (it.hasNext() == true)
	{	
		QLCFixtureDef* fd = it.next();
		if (fd->manufacturer() == manufacturer && fd->model() == model)
			return fd;
	}
	
	return NULL;
}

/*****************************************************************************
 * Main application Mode
 *****************************************************************************/

void App::slotModeOperate()
{
	/* Nothing to do if we're already in the desired mode */
	if (m_mode == Operate)
		return;

	/* Set highlighted palette to mode indicator */
	m_modeIndicator->setText(KModeTextOperate);
	QPalette pal = palette();
	pal.setColor(QPalette::Window,
		     QApplication::palette().color(QPalette::Highlight));
	pal.setColor(QPalette::WindowText,
		     QApplication::palette().color(QPalette::HighlightedText));
	m_modeIndicator->setPalette(pal);

	m_modeOperateAction->setChecked(true);
	
	m_fileNewAction->setEnabled(false);
	m_fileOpenAction->setEnabled(false);
	m_fileQuitAction->setEnabled(false);

	m_functionManagerAction->setEnabled(false);
	m_outputManagerAction->setEnabled(false);
	m_inputManagerAction->setEnabled(false);
	m_busManagerAction->setEnabled(false);
	
	/* Close function manager if it's open */
	if (m_functionManager != NULL)
		m_functionManager->parentWidget()->close();
	
	/* Close bus manager if it's open */
	if (m_busManager != NULL)
		m_busManager->parentWidget()->close();

	/* Close input manager if it's open */
	if (m_inputManager != NULL)
		m_inputManager->parentWidget()->close();

	/* Close output manager if it's open */
	if (m_outputManager != NULL)
		m_outputManager->parentWidget()->close();

	/* Start function consumer */
	m_functionConsumer->start();

	/* Prevent opening a context menu */
	centralWidget()->setContextMenuPolicy(Qt::PreventContextMenu);

	m_mode = Operate;
	emit modeChanged(Operate);
}

void App::slotModeDesign()
{
	/* Nothing to do if we're already in the desired mode */
	if (m_mode == Design)
		return;

	if (m_functionConsumer->runningFunctions())
	{
		int result = QMessageBox::warning(
			this,
			tr("Switch to Design Mode"),
			tr("There are still running functions.\n"
			   "Really stop them and switch back to "
			   "Design mode?"),
			QMessageBox::Yes,
			QMessageBox::No);
		
		if (result == QMessageBox::No)
			return;
		else
			m_functionConsumer->purge();
	}
	
	/* Stop function consumer */
	m_functionConsumer->stop();
	
	/* Set normal palette to mode indicator */
	m_modeIndicator->setText(KModeTextDesign);
	QPalette pal = palette();
	pal.setColor(QPalette::Window,
		     QApplication::palette().color(QPalette::Window));
	pal.setColor(QPalette::WindowText,
		     QApplication::palette().color(QPalette::WindowText));
	m_modeIndicator->setPalette(pal);

	m_modeDesignAction->setChecked(true);
	
	m_fileNewAction->setEnabled(true);
	m_fileOpenAction->setEnabled(true);
	m_fileQuitAction->setEnabled(true);

	m_functionManagerAction->setEnabled(true);
	m_outputManagerAction->setEnabled(true);
	m_inputManagerAction->setEnabled(true);
	m_busManagerAction->setEnabled(true);

	/* Allow opening a context menu */
	centralWidget()->setContextMenuPolicy(Qt::CustomContextMenu);

	m_mode = Design;
	emit modeChanged(Design);
}

/*****************************************************************************
 * Virtual Console
 *****************************************************************************/

void App::initVirtualConsole(void)
{
	if (m_virtualConsole != NULL)
		delete m_virtualConsole;

	QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
	m_virtualConsole = new VirtualConsole(sub);
	m_virtualConsole->show();

	sub->setWidget(m_virtualConsole);

	connect(m_virtualConsole, SIGNAL(closed()),
		this, SLOT(slotVirtualConsoleClosed()));

	connect(this, SIGNAL(modeChanged(App::Mode)),
		m_virtualConsole, SLOT(slotModeChanged(App::Mode)));

	sub->hide();
}

/*****************************************************************************
 * Actions, menubar, toolbar, statusbar
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
	connect(m_fileOpenAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileOpen()));

	m_fileSaveAction = new QAction(QIcon(":/filesave.png"),
				       tr("Save"), this);
	connect(m_fileSaveAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSave()));

	m_fileSaveAsAction = new QAction(QIcon(":/filesaveas.png"),
					 tr("Save As..."), this);
	connect(m_fileSaveAsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSaveAs()));

	m_fileQuitAction = new QAction(QIcon(":/exit.png"),
				       tr("Quit"), this);
	connect(m_fileQuitAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileQuit()));

	/* Manager actions */
	m_fixtureManagerAction = new QAction(QIcon(":/fixture.png"),
					     tr("Fixtures"), this);
	connect(m_fixtureManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFixtureManager()));

	m_functionManagerAction = new QAction(QIcon(":/function.png"),
					      tr("Functions"), this);
	connect(m_functionManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFunctionManager()));

	m_busManagerAction = new QAction(QIcon(":/bus.png"),
					 tr("Buses"), this);
	connect(m_busManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBusManager()));

	m_inputManagerAction = new QAction(QIcon(":/input.png"),
					   tr("Inputs"), this);
	connect(m_inputManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotInputManager()));

	m_outputManagerAction = new QAction(QIcon(":/output.png"),
					    tr("Outputs"), this);
	connect(m_outputManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotOutputManager()));

	/* Mode actions */
	QActionGroup* modeGroup = new QActionGroup(this);
	m_modeDesignAction = new QAction(tr("Design"), this);
	m_modeDesignAction->setCheckable(true);
	m_modeDesignAction->setChecked(true);
	modeGroup->addAction(m_modeDesignAction);
	connect(m_modeDesignAction, SIGNAL(triggered(bool)),
		this, SLOT(slotModeDesign()));

	m_modeOperateAction = new QAction(tr("Operate"), this);
	m_modeOperateAction->setCheckable(true);
	modeGroup->addAction(m_modeOperateAction);
	connect(m_modeOperateAction, SIGNAL(triggered(bool)),
		this, SLOT(slotModeOperate()));

	/* Control actions */
	m_controlVCAction = new QAction(QIcon(":/virtualconsole.png"),
					tr("Virtual Console"), this);
	connect(m_controlVCAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlVirtualConsole()));

	m_controlMonitorAction = new QAction(QIcon(":/monitor.png"),
					     tr("Monitor"), this);
	connect(m_controlMonitorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlMonitor()));

	m_controlBlackoutAction = new QAction(QIcon(":/blackout.png"),
					      tr("Toggle Blackout"), this);
	m_controlBlackoutAction->setCheckable(true);
	connect(m_controlBlackoutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlBlackout()));
	
	m_controlPanicAction = new QAction(QIcon(":/panic.png"),
					   tr("Panic!"), this);
	connect(m_controlPanicAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlPanic()));

	/* Help actions */
	m_helpIndexAction = new QAction(QIcon(":/help.png"),
					tr("Index"), this);
	connect(m_helpIndexAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpIndex()));

	m_helpAboutAction = new QAction(QIcon(":/qlc.png"),
					tr("About QLC"), this);
	connect(m_helpAboutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAbout()));

	m_helpAboutQtAction = new QAction(tr("About Qt"), this);
	connect(m_helpAboutQtAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAboutQt()));
}

QMenuBar* App::menuBar()
{
#ifdef __APPLE__
        static QMenuBar *mainMenuBar = NULL;
	if (mainMenuBar == NULL)
		mainMenuBar = new QMenuBar(0);
	return mainMenuBar;
#else
	return QMainWindow::menuBar();
#endif
}

void App::initMenuBar()
{
	/* File Menu */
	m_fileMenu = new QMenu(menuBar());
	m_fileMenu->setTitle(tr("File"));
	menuBar()->addMenu(m_fileMenu);
	m_fileMenu->addAction(m_fileNewAction);
	m_fileMenu->addAction(m_fileOpenAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_fileSaveAction);
	m_fileMenu->addAction(m_fileSaveAsAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_fileQuitAction);

	/* Manager Menu */
	m_managerMenu = new QMenu(menuBar());
	m_managerMenu->setTitle(tr("Manager"));
	menuBar()->addMenu(m_managerMenu);
	m_managerMenu->addAction(m_fixtureManagerAction);
	m_managerMenu->addAction(m_functionManagerAction);
	m_managerMenu->addAction(m_busManagerAction);
	m_managerMenu->addSeparator();
	m_managerMenu->addAction(m_inputManagerAction);
	m_managerMenu->addAction(m_outputManagerAction);

	/* Control Menu */
	m_controlMenu = new QMenu(menuBar());
	m_controlMenu->setTitle(tr("Control"));
	menuBar()->addMenu(m_controlMenu);
	
	/* Mode menu */
	m_modeMenu = new QMenu(m_controlMenu);
	m_modeMenu->setTitle(tr("Mode"));
	m_modeMenu->addAction(m_modeDesignAction);
	m_modeMenu->addAction(m_modeOperateAction);
	m_controlMenu->addMenu(m_modeMenu);

	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlVCAction);
	m_controlMenu->addAction(m_controlMonitorAction);
	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlPanicAction);
	m_controlMenu->addAction(m_controlBlackoutAction);

	menuBar()->addSeparator();

	/* Help menu */
	m_helpMenu = new QMenu(menuBar());
	m_helpMenu->setTitle(tr("Help"));
	menuBar()->addMenu(m_helpMenu);
	m_helpMenu->addAction(m_helpIndexAction);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_helpAboutAction);
	m_helpMenu->addAction(m_helpAboutQtAction);
}

void App::initStatusBar()
{
	/* Mode Indicator */
	m_modeIndicator = new QLabel(statusBar());
	m_modeIndicator->setMargin(2);
	m_modeIndicator->setText(KModeTextDesign);
	m_modeIndicator->setAutoFillBackground(true);
	statusBar()->addWidget(m_modeIndicator);

	/* Blackout Indicator */
	m_blackoutIndicatorTimer = new QTimer(this);
	m_blackoutIndicator = new QLabel(statusBar());
	m_blackoutIndicator->setMargin(2);
	m_blackoutIndicator->setText(tr("Blackout"));
	m_blackoutIndicator->setAutoFillBackground(true);
	m_blackoutIndicator->hide();
	statusBar()->addWidget(m_blackoutIndicator);
}

void App::initToolBar()
{
	m_toolbar = new QToolBar(tr("Workspace"), this);
	m_toolbar->setFloatable(false);
	m_toolbar->setMovable(false);
	addToolBar(m_toolbar);
	m_toolbar->addAction(m_fileNewAction);
	m_toolbar->addAction(m_fileOpenAction);
	m_toolbar->addAction(m_fileSaveAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_fixtureManagerAction);
	m_toolbar->addAction(m_functionManagerAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_controlVCAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_controlMonitorAction);
	m_toolbar->addAction(m_controlBlackoutAction);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

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
	initDoc();
	initVirtualConsole();
	doc()->resetModified();

	setWindowTitle(tr("%1 - New Workspace").arg(KApplicationNameLong));
}

void App::slotFileOpen()
{
	QString fileName;

	/* Check that the user is aware of losing previous changes */
	if (doc()->isModified() == true)
	{
		QString msg = tr("Do you wish to save the current workspace?\n"
				 "Otherwise you will lose changes.");
		int result = QMessageBox::warning(this, "Open Workspace", msg,
						  QMessageBox::Yes,
						  QMessageBox::No,
						  QMessageBox::Cancel);
		if (result == QMessageBox::Yes)
			slotFileSave(); /* Save first */
		else if (result == QMessageBox::Cancel)
			return; /* Whoops, go back! */
	}

	/* Create a file open dialog */
	QFileDialog dialog(this);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.selectFile(m_doc->fileName());

	/* Append file filters to the dialog */
	QStringList filters;
	filters << QString("Workspaces (*%1)").arg(KExtWorkspace);
	filters << QString("All Files (*)");
	dialog.setNameFilters(filters);

	/* Append useful URLs to the dialog */
	QList <QUrl> sidebar;
	sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
	sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
	dialog.setSidebarUrls(sidebar);

	/* Get file name */
	if (dialog.exec() != QDialog::Accepted)
		return;
	fileName = dialog.selectedFiles().first();
	if (fileName.isEmpty() == true)
		return;

	/* Clear existing document data */
	newDocument();
	
	/* Load the file */
	if (doc()->loadXML(fileName) == false)
		QMessageBox::critical(this, tr("Unable to open file"),
				      tr("Workspace file might be corrupt."));
	else
		doc()->resetModified();
}

void App::slotFileSave()
{
	/* Attempt to save with the existing name. Fall back to Save As. */
	if (m_doc->fileName().isEmpty() == true)
		slotFileSaveAs();
	else if (m_doc->saveXML(m_doc->fileName()) == true)
		setWindowTitle(QString("%1 - %2").arg(KApplicationNameLong)
						 .arg(doc()->fileName()));
}

void App::slotFileSaveAs()
{
	QString fileName;

	/* Create a file save dialog */
	QFileDialog dialog(this);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.selectFile(m_doc->fileName());

	/* Append file filters to the dialog */
	QStringList filters;
	filters << QString("Workspaces (*%1)").arg(KExtWorkspace);
	filters << QString("All Files (*)");
	dialog.setNameFilters(filters);
	
	/* Append useful URLs to the dialog */
	QList <QUrl> sidebar;
	sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
	sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
	dialog.setSidebarUrls(sidebar);

	/* Get file name */
	if (dialog.exec() != QDialog::Accepted)
		return;
	fileName = dialog.selectedFiles().first();
	if (fileName.isEmpty() == true)
		return;

	/* Always use the workspace suffix */
	if (fileName.right(4) != KExtWorkspace)
		fileName += KExtWorkspace;

	/* Save the document and set workspace name */
	if (m_doc->saveXML(fileName) == true)
		setWindowTitle(QString("%1 - %2").arg(KApplicationNameLong)
						 .arg(doc()->fileName()));
}

void App::slotFileQuit()
{
	close();
}

/*****************************************************************************
 * Manager action slots
 *****************************************************************************/

void App::slotFixtureManager()
{
	if (m_fixtureManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_fixtureManager = new FixtureManager(sub);

		sub->setWidget(m_fixtureManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(":/fixture.png"));
		sub->setWindowTitle(tr("Fixture Manager"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_fixtureManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotFixtureManagerDestroyed(QObject*)));

		/* To disable some actions when switching to operate mode */
		connect(this, SIGNAL(modeChanged(App::Mode)),
			m_fixtureManager, SLOT(slotModeChanged(App::Mode)));

		/* Listen to fixture additions/removals */
		connect(m_doc,
			SIGNAL(fixtureAdded(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureAdded(t_fixture_id)));
		
		connect(m_doc,
			SIGNAL(fixtureRemoved(t_fixture_id)),
			m_fixtureManager,
			SLOT(slotFixtureRemoved(t_fixture_id)));

		m_fixtureManager->show();
		sub->show();
		sub->resize(600, 350);
	}
}

void App::slotFixtureManagerDestroyed(QObject*)
{
	m_fixtureManager = NULL;
}

void App::slotFunctionManager()
{
	if (m_functionManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_functionManager = new FunctionManager(sub);

		sub->setWidget(m_functionManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(":/function.png"));
		sub->setWindowTitle("Function Manager");

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_functionManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotFunctionManagerDestroyed(QObject*)));

		connect(m_doc, SIGNAL(functionAdded(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionAdded(t_function_id)));

		connect(m_doc, SIGNAL(functionRemoved(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionRemoved(t_function_id)));

		connect(m_doc, SIGNAL(functionChanged(t_function_id)),
			m_functionManager,
			SLOT(slotFunctionChanged(t_function_id)));

		m_functionManager->show();
		sub->show();
		sub->resize(600, 450);
	}
}

void App::slotFunctionManagerDestroyed(QObject*)
{
	m_functionManager = NULL;
}

void App::slotBusManager()
{
	if (m_busManager == NULL)
	{
		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_busManager = new BusManager(sub);

		sub->setWidget(m_busManager);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowTitle(tr("Bus Manager"));
		sub->setWindowIcon(QIcon(":/bus.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_busManager, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotBusManagerDestroyed(QObject*)));

		m_busManager->show();
		sub->show();
	}
}

void App::slotBusManagerDestroyed(QObject*)
{
	m_busManager = NULL;
}

/*****************************************************************************
 * Control action slots
 *****************************************************************************/

void App::slotControlBlackout()
{
	Q_ASSERT(m_outputMap != NULL);

	if (m_outputMap->blackout() == true)
		m_outputMap->setBlackout(false);
	else
		m_outputMap->setBlackout(true);
}

void App::slotControlVirtualConsole()
{
	Q_ASSERT(m_virtualConsole != NULL);

	m_virtualConsole->show();
	m_virtualConsole->parentWidget()->show();
	m_virtualConsole->parentWidget()->setFocus();
}

void App::slotVirtualConsoleClosed()
{
}

void App::slotControlMonitor()
{
	if (m_monitor == NULL)
	{
		Q_ASSERT(m_outputMap != NULL);

		QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
		m_monitor = new Monitor(sub);

		sub->setWidget(m_monitor);
		sub->setAttribute(Qt::WA_DeleteOnClose);
		sub->setWindowIcon(QIcon(":/monitor.png"));

		qobject_cast <QMdiArea*> (centralWidget())->addSubWindow(sub);

		connect(m_monitor, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotMonitorDestroyed(QObject*)));
		
		m_monitor->show();
		m_monitor->setFocus();
		sub->show();
	}
}

void App::slotMonitorDestroyed(QObject*)
{
	m_monitor = NULL;
}

void App::slotControlPanic()
{
	/* Shut down all running functions */
	m_functionConsumer->purge();
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
	if (m_docBrowser == NULL)
	{
		m_docBrowser = new QLCDocBrowser(this);
		m_docBrowser->setAttribute(Qt::WA_DeleteOnClose);

		connect(m_docBrowser, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotDocBrowserDestroyed(QObject*)));

		m_docBrowser->show();
	}
}

void App::slotDocBrowserDestroyed(QObject*)
{
	m_docBrowser = NULL;
}

void App::slotHelpAbout()
{
	AboutBox ab(this);
	ab.exec();
}

void App::slotHelpAboutQt()
{
	QMessageBox::aboutQt(this, QString("Q Light Controller"));
}

void App::slotCustomContextMenuRequested(const QPoint&)
{
	QMenu menu;
	menu.addAction(QIcon(":/image.png"), tr("Set background image..."),
		       this, SLOT(slotSetBackgroundImage()));
	menu.addAction(QIcon(":/editdelete.png"),
		       tr("Clear background image"),
		       this, SLOT(slotClearBackgroundImage()));
	menu.exec(QCursor::pos());
}

/*****************************************************************************
 * Workspace background
 *****************************************************************************/

void App::setBackgroundImage(QString path)
{
	if (path.isEmpty() == true)
	{
		/* TODO: Is there some better way to "reset" the palette
		   instead of explicitly specifying a color? */
		qobject_cast<QMdiArea*> (centralWidget())->setBackground(
			QBrush(QApplication::palette().color(QPalette::Dark)));
	}
	else if (path != m_backgroundImage)
	{
		/* Set background image only if it's not already there */
		qobject_cast<QMdiArea*>	(centralWidget())->setBackground(
								QPixmap(path));
	}

	/* Save workspace background setting */
	QSettings settings;
	settings.setValue("/workspace/background", path);
	m_backgroundImage = path;
}

void App::slotSetBackgroundImage()
{
	QString path = QFileDialog::getOpenFileName(
		this, tr("Open an image file"), getenv("HOME"),
		tr("Images (*.png *.xpm *.jpg *.gif)"));

	if (path.isEmpty() == false)
		setBackgroundImage(path);
}

void App::slotClearBackgroundImage()
{
	setBackgroundImage(QString::null);
}
