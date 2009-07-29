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
#include <QStyleFactory>
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
#include <QStyle>
#include <QMenu>
#include <QRect>
#include <QFile>
#include <QIcon>

#include "functionmanager.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "outputmanager.h"
#include "inputmanager.h"
#include "mastertimer.h"
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

#include "common/qlcfixturedefcache.h"
#include "common/qlcdocbrowser.h"
#include "common/qlcfixturedef.h"
#include "common/qlctypes.h"
#include "common/qlcfile.h"

#define KModeTextOperate tr("Operate")
#define KModeTextDesign tr("Design")
#define KXMLQLCGeometry "Geometry"
#define KXMLQLCWindowState "WindowState"

App* _app;
QStyle* App::s_saneStyle = NULL;

/*********************************************************************
 * Initialization
 *********************************************************************/

App::App() : QMainWindow()
{
	_app = this;

	m_masterTimer = NULL;
	m_outputMap = NULL;
	m_inputMap = NULL;
	m_doc = NULL;

	m_mode = Design;
	m_modeIndicator = NULL;

	m_blackoutIndicator = NULL;
	m_blackoutIndicatorTimer = NULL;

	m_fixtureAllocationIndicator = NULL;
	m_functionAllocationIndicator = NULL;

	QCoreApplication::setOrganizationName("qlc");
	QCoreApplication::setOrganizationDomain("sf.net");
	QCoreApplication::setApplicationName("Q Light Controller");

	init();
	slotModeDesign();
	slotDocModified(false);
}

App::~App()
{
#ifndef __APPLE__
	/* Save application geometry */
	QSettings settings;
	settings.setValue(KXMLQLCWindowState, int(windowState()));
	/* Save window geometry only if the window is not maximized. Otherwise
	   the non-maximized state is just 1px smaller than the maximized one */
	if (!(windowState() & Qt::WindowMaximized))
		settings.setValue(KXMLQLCGeometry, rect());
#endif

	// Store outputmap defaults
	if (m_outputMap != NULL)
		m_outputMap->saveDefaults();

	// Store inputmap defaults
	if (m_inputMap != NULL)
		m_inputMap->saveDefaults();

	// Delete doc
	if (m_doc != NULL)
		delete m_doc;
	m_doc = NULL;

	// Delete master timer
	if (m_masterTimer != NULL)
		delete m_masterTimer;
	m_masterTimer = NULL;

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

	// Remove the reference to the application
	_app = NULL;
}


/**
 * Main initialization function
 */
void App::init()
{
	QSettings settings;

	setWindowIcon(QIcon(":/qlc.png"));

#ifndef __APPLE__
	/* MDI Area */
	setCentralWidget(new QMdiArea(this));
	centralWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(centralWidget(),
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotCustomContextMenuRequested(const QPoint&)));

	/* Workspace background */
	setBackgroundImage(settings.value("/workspace/background").toString());

	/* Application geometry and window state */
	QVariant var;
	var = settings.value(KXMLQLCGeometry, QRect(0, 0, 800, 600));
	if (var.isValid() == true)
		setGeometry(var.toRect());
	var = settings.value(KXMLQLCWindowState, Qt::WindowNoState);
	if (var.isValid() == true)
		setWindowState(Qt::WindowState(var.toInt()));
#else
	/* App is just a toolbar, we only need it to be the size of the
	   toolbar's buttons */
	resize(600, 32);
	move(0, 22);
#endif

	/* Input & output mappers and their plugins */
	initOutputMap();
	initInputMap();

	/* Function running engine/master timer */
	m_masterTimer = new MasterTimer(this, m_outputMap);
	m_masterTimer->start();

	/* Buses */
	Bus::init(this);

	/* Fixture definitions */
	loadFixtureDefinitions();

	// The main view
	initStatusBar();
	initActions();
	initMenuBar();
	initToolBar();

	// Document
	initDoc();

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

	m_outputMap->loadPlugins();
	m_outputMap->loadDefaults();

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

/*****************************************************************************
 * Input mapping
 *****************************************************************************/

void App::initInputMap()
{
	m_inputMap = new InputMap(this, KInputUniverseCount);
	Q_ASSERT(m_inputMap != NULL);

	/* Load input plugins */
	m_inputMap->loadPlugins();

#ifdef Q_WS_X11
	/* First, load user profiles (UNIX only). Override system profiles,
	 * since duplicates in system profiles are ignored. */
	QDir dir(QString(getenv("HOME")));
	m_inputMap->loadProfiles(dir.absoluteFilePath(
					QString(USERINPUTPROFILEDIR)));
#endif

	/* Then, load system profiles */
#ifdef __APPLE__
	m_inputMap->loadProfiles(QString("%1/%2")
				 .arg(QApplication::applicationDirPath())
				 .arg(INPUTPROFILEDIR));
#else
	m_inputMap->loadProfiles(INPUTPROFILEDIR);
#endif

	m_inputMap->loadDefaults();
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::initDoc()
{
	// Delete existing document object and create a new one
	if (m_doc != NULL)
		delete m_doc;
	m_doc = new Doc(this, fixtureDefCache());

	connect(m_doc, SIGNAL(modified(bool)),
		this, SLOT(slotDocModified(bool)));

	connect(this, SIGNAL(modeChanged(App::Mode)),
		m_doc, SLOT(slotModeChanged(App::Mode)));

	emit documentChanged(m_doc);
}

void App::slotDocModified(bool state)
{
	QString caption(KApplicationNameLong);

	if (fileName() != QString::null)
	{
		caption += QString(" - ")
			+ QDir::toNativeSeparators(fileName());
	}
	else
	{
		caption += tr(" - New Workspace");
	}

	if (state == true)
		setWindowTitle(caption + QString(" *"));
	else
		setWindowTitle(caption);

	/* Update fixture & function allocation status */
	m_fixtureAllocationIndicator->setText(tr("Fixtures: %1/%2")
		.arg(m_doc->fixtures()).arg(KFixtureArraySize));
	m_fixtureAllocationIndicator->setToolTip(
		tr("Space left for %1 fixtures")
		.arg(KFixtureArraySize - m_doc->fixtures()));

	m_functionAllocationIndicator->setText(tr("Functions: %1/%2")
		.arg(m_doc->functions()).arg(KFunctionArraySize));
	m_functionAllocationIndicator->setToolTip(
		tr("Space left for %1 functions")
		.arg(KFunctionArraySize - m_doc->functions()));
}

/*****************************************************************************
 * Fixture definitions
 *****************************************************************************/

void App::loadFixtureDefinitions()
{
#ifdef Q_WS_X11
	/* First, load user fixtures (overrides system fixtures) */
	QDir dir(QString(getenv("HOME")));
	m_fixtureDefCache.load(dir.absoluteFilePath(QString(USERFIXTUREDIR)));
#endif

	/* Then, load system fixtures */
#ifdef __APPLE__
        m_fixtureDefCache.load(QString("%1/%2")
                               .arg(QApplication::applicationDirPath())
                               .arg(FIXTUREDIR));
#else
	m_fixtureDefCache.load(FIXTUREDIR);
#endif
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
	m_modeIndicator->setToolTip(
		tr("Operate mode is active; editing facilities are disabled"));

	QPalette pal = palette();
	pal.setColor(QPalette::Window,
		     QApplication::palette().color(QPalette::Highlight));
	pal.setColor(QPalette::WindowText,
		     QApplication::palette().color(QPalette::HighlightedText));
	m_modeIndicator->setPalette(pal);

	m_fileNewAction->setEnabled(false);
	m_fileOpenAction->setEnabled(false);
	m_fileQuitAction->setEnabled(false);

	m_functionManagerAction->setEnabled(false);
	m_outputManagerAction->setEnabled(false);
	m_inputManagerAction->setEnabled(false);
	m_busManagerAction->setEnabled(false);

	m_modeToggleAction->setIcon(QIcon(":/design.png"));
	m_modeToggleAction->setText(tr("Design"));
	m_modeToggleAction->setToolTip(tr("Switch to design mode"));

	/* Prevent opening a context menu */
	#ifndef __APPLE__
		// No centralWidget() on APPLE
		centralWidget()->setContextMenuPolicy(Qt::PreventContextMenu);
	#endif

	m_mode = Operate;
	emit modeChanged(Operate);
}

void App::slotModeDesign()
{
	/* Nothing to do if we're already in the desired mode */
	if (m_mode == Design)
		return;

	if (m_masterTimer->runningFunctions())
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
			m_masterTimer->stopAll();
	}

	/* Set normal palette to mode indicator */
	m_modeIndicator->setText(KModeTextDesign);
	m_modeIndicator->setToolTip(
		tr("Design mode is active; editing facilities are enabled"));

	QPalette pal = palette();
	pal.setColor(QPalette::Window,
		     QApplication::palette().color(QPalette::Window));
	pal.setColor(QPalette::WindowText,
		     QApplication::palette().color(QPalette::WindowText));
	m_modeIndicator->setPalette(pal);

	m_fileNewAction->setEnabled(true);
	m_fileOpenAction->setEnabled(true);
	m_fileQuitAction->setEnabled(true);

	m_functionManagerAction->setEnabled(true);
	m_outputManagerAction->setEnabled(true);
	m_inputManagerAction->setEnabled(true);
	m_busManagerAction->setEnabled(true);

	m_modeToggleAction->setIcon(QIcon(":/operate.png"));
	m_modeToggleAction->setText(tr("Operate"));
	m_modeToggleAction->setToolTip(tr("Switch to operate mode"));

	/* Allow opening a context menu */
	#ifndef __APPLE__
		// No centralWidget on APPLE
		centralWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
	#endif

	m_mode = Design;
	emit modeChanged(Design);
}

void App::slotModeToggle()
{
	if (m_mode == Design)
		slotModeOperate();
	else
		slotModeDesign();
}

/*****************************************************************************
 * Sane style
 *****************************************************************************/

QStyle* App::saneStyle()
{
#ifdef WIN32
	if (s_saneStyle == NULL)
	{
		QStringList keys(QStyleFactory::keys());
		if (keys.contains("Cleanlooks") == true)
			s_saneStyle = QStyleFactory::create("Cleanlooks");
		else
			s_saneStyle = QApplication::style();
	}

	return s_saneStyle;
#else
	return QApplication::style();
#endif
}

/*****************************************************************************
 * Status bar
 *****************************************************************************/

void App::initStatusBar()
{
	/* Fixture Allocation Indicator */
	m_fixtureAllocationIndicator = new QLabel(statusBar());
	m_fixtureAllocationIndicator->setFrameStyle(QFrame::StyledPanel |
						    QFrame::Sunken);
	m_fixtureAllocationIndicator->setText(tr("Fixtures: %1/%2")
		.arg(KFixtureArraySize).arg(KFixtureArraySize));
	statusBar()->addWidget(m_fixtureAllocationIndicator);

	/* Function Allocation Indicator */
	m_functionAllocationIndicator = new QLabel(statusBar());
	m_functionAllocationIndicator->setFrameStyle(QFrame::StyledPanel |
						     QFrame::Sunken);
	m_functionAllocationIndicator->setText(tr("Functions: %1/%2")
		.arg(KFunctionArraySize).arg(KFunctionArraySize));
	statusBar()->addWidget(m_functionAllocationIndicator);

	/* Mode Indicator */
	m_modeIndicator = new QLabel(statusBar());
	m_modeIndicator->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	m_modeIndicator->setText(KModeTextDesign);
	m_modeIndicator->setAutoFillBackground(true);
	m_modeIndicator->setToolTip(
		tr("Design mode is active; editing facilities are enabled"));
	statusBar()->addWidget(m_modeIndicator);

	/* Blackout Indicator */
	m_blackoutIndicatorTimer = new QTimer(this);
	m_blackoutIndicator = new QLabel(statusBar());
	m_blackoutIndicator->setFrameStyle(QFrame::StyledPanel |
					   QFrame::Sunken);
	m_blackoutIndicator->setText(tr("Blackout"));
	m_blackoutIndicator->setAutoFillBackground(true);
	m_blackoutIndicator->setToolTip(
		tr("Blackout is active; all outputs are disabled"));
	m_blackoutIndicator->hide();
	statusBar()->addWidget(m_blackoutIndicator);
}

/*****************************************************************************
 * Actions, menubar, toolbar, statusbar
 *****************************************************************************/

void App::initActions()
{
	/* File actions */
	m_fileNewAction = new QAction(QIcon(":/filenew.png"),
				      tr("&New"), this);
	m_fileNewAction->setShortcut(QKeySequence("CTRL+N"));
	connect(m_fileNewAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileNew()));

	m_fileOpenAction = new QAction(QIcon(":/fileopen.png"),
				       tr("&Open"), this);
	m_fileOpenAction->setShortcut(QKeySequence("CTRL+O"));
	connect(m_fileOpenAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileOpen()));

	m_fileSaveAction = new QAction(QIcon(":/filesave.png"),
				       tr("&Save"), this);
	m_fileSaveAction->setShortcut(QKeySequence("CTRL+S"));
	connect(m_fileSaveAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSave()));

	m_fileSaveAsAction = new QAction(QIcon(":/filesaveas.png"),
					 tr("Save &As..."), this);
	connect(m_fileSaveAsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileSaveAs()));

	m_fileQuitAction = new QAction(QIcon(":/exit.png"),
				       tr("&Quit"), this);
	connect(m_fileQuitAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFileQuit()));

	/* Manager actions */
	m_fixtureManagerAction = new QAction(QIcon(":/fixture.png"),
					     tr("&Fixtures"), this);
	m_fixtureManagerAction->setShortcut(QKeySequence("ALT+X"));
	connect(m_fixtureManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFixtureManager()));

	m_functionManagerAction = new QAction(QIcon(":/function.png"),
					      tr("&Functions"), this);
	m_functionManagerAction->setShortcut(QKeySequence("ALT+U"));
	connect(m_functionManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFunctionManager()));

	m_busManagerAction = new QAction(QIcon(":/bus.png"),
					 tr("&Buses"), this);
	m_busManagerAction->setShortcut(QKeySequence("ALT+B"));
	connect(m_busManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBusManager()));

	m_inputManagerAction = new QAction(QIcon(":/input.png"),
					   tr("&Inputs"), this);
	m_inputManagerAction->setShortcut(QKeySequence("ALT+I"));
	connect(m_inputManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotInputManager()));

	m_outputManagerAction = new QAction(QIcon(":/output.png"),
					    tr("&Outputs"), this);
	m_outputManagerAction->setShortcut(QKeySequence("ALT+O"));
	connect(m_outputManagerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotOutputManager()));

	/* Control actions */
	m_modeToggleAction = new QAction(QIcon(":/operate.png"),
					 tr("&Operate"), this);
	m_modeToggleAction->setToolTip(tr("Switch to operate mode"));
	m_modeToggleAction->setShortcut(QKeySequence("CTRL+TAB"));
	connect(m_modeToggleAction, SIGNAL(triggered(bool)),
		this, SLOT(slotModeToggle()));

	m_controlVCAction = new QAction(QIcon(":/virtualconsole.png"),
					tr("&Virtual Console"), this);
	m_controlVCAction->setShortcut(QKeySequence("CTRL+R"));
	connect(m_controlVCAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlVC()));

	m_controlMonitorAction = new QAction(QIcon(":/monitor.png"),
					     tr("&Monitor"), this);
	m_controlMonitorAction->setShortcut(QKeySequence("CTRL+M"));
	connect(m_controlMonitorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlMonitor()));

	m_controlBlackoutAction = new QAction(QIcon(":/blackout.png"),
					      tr("Toggle &Blackout"), this);
	m_controlBlackoutAction->setCheckable(true);
	m_controlBlackoutAction->setShortcut(QKeySequence("CTRL+B"));
	connect(m_controlBlackoutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlBlackout()));

	/* Help actions */
	m_helpIndexAction = new QAction(QIcon(":/help.png"),
					tr("&Index"), this);
	m_helpIndexAction->setShortcut(QKeySequence("SHIFT+F1"));
	connect(m_helpIndexAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpIndex()));

	m_helpAboutAction = new QAction(QIcon(":/qlc.png"),
					tr("&About QLC"), this);
	connect(m_helpAboutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotHelpAbout()));

	m_helpAboutQtAction = new QAction(QIcon(":/qt.png"),
					  tr("About &Qt"), this);
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
	m_fileMenu->setTitle(tr("&File"));
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
	m_managerMenu->setTitle(tr("&Manager"));
	menuBar()->addMenu(m_managerMenu);
	m_managerMenu->addAction(m_fixtureManagerAction);
	m_managerMenu->addAction(m_functionManagerAction);
	m_managerMenu->addAction(m_busManagerAction);
	m_managerMenu->addSeparator();
	m_managerMenu->addAction(m_inputManagerAction);
	m_managerMenu->addAction(m_outputManagerAction);

	/* Control Menu */
	m_controlMenu = new QMenu(menuBar());
	m_controlMenu->setTitle(tr("&Control"));
	menuBar()->addMenu(m_controlMenu);
	m_controlMenu->addAction(m_modeToggleAction);
	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlVCAction);
	m_controlMenu->addAction(m_controlMonitorAction);
	m_controlMenu->addSeparator();
	m_controlMenu->addAction(m_controlBlackoutAction);

	menuBar()->addSeparator();

	/* Help menu */
	m_helpMenu = new QMenu(menuBar());
	m_helpMenu->setTitle(tr("&Help"));
	menuBar()->addMenu(m_helpMenu);
	m_helpMenu->addAction(m_helpIndexAction);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_helpAboutAction);
	m_helpMenu->addAction(m_helpAboutQtAction);
}

void App::initToolBar()
{
	QWidget* widget;

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
	m_toolbar->addAction(m_controlMonitorAction);

	/* Create an empty widget between the last toolbar button and
	   BO & mode buttons to separate the critical ones from the rest. */
	widget = new QWidget(this);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_toolbar->addWidget(widget);

	m_toolbar->addAction(m_controlBlackoutAction);
	m_toolbar->addAction(m_modeToggleAction);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

bool App::handleFileError(QFile::FileError error)
{
	QString msg;

	switch (error)
	{
	case QFile::NoError:
		return true;
		break;
	case QFile::ReadError:
		msg = tr("Unable to read from file");
		break;
	case QFile::WriteError:
		msg = tr("Unable to write to file");
		break;
	case QFile::FatalError:
		msg = tr("A fatal error occurred");
		break;
	case QFile::ResourceError:
		msg = tr("Unable to access resource");
		break;
	case QFile::OpenError:
		msg = tr("Unable to open file for reading or writing");
		break;
	case QFile::AbortError:
		msg = tr("Operation was aborted");
		break;
	case QFile::TimeOutError:
		msg = tr("Operation timed out");
		break;
	default:
	case QFile::UnspecifiedError:
		msg = tr("An unspecified error has occurred. Nice.");
		break;
	}

	QMessageBox::warning(this, tr("File error"), msg);

	return false;
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
	initDoc();
	VirtualConsole::resetContents();
	doc()->resetModified();
}

QFile::FileError App::slotFileOpen()
{
	QString fn;

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
		{
			/* Save first, but don't proceed unless it succeeded. */
			QFile::FileError error = slotFileSaveAs();
			if (handleFileError(error) == false)
				return error;
		}
		else if (result == QMessageBox::Cancel)
		{
			/* Second thoughts... Cancel loading. */
			return QFile::NoError;
		}
	}

	/* Create a file open dialog */
	QFileDialog dialog(this);
	dialog.setWindowTitle(tr("Open Workspace"));
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.selectFile(fileName());

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
		return QFile::NoError;

	fn = dialog.selectedFiles().first();
	if (fn.isEmpty() == true)
		return QFile::NoError;

	/* Clear existing document data */
	newDocument();

	/* Load the file */
	QFile::FileError error = loadXML(fn);
	if (handleFileError(error) == true)
		doc()->resetModified();

	/* Update these in any case, since they are at least emptied now as
	   a result of calling newDocument() a few lines ago. */
	if (FunctionManager::instance() != NULL)
		FunctionManager::instance()->updateTree();
	if (OutputManager::instance() != NULL)
		InputManager::instance()->updateTree();
	if (InputManager::instance() != NULL)
		InputManager::instance()->updateTree();

	return error;
}

QFile::FileError App::slotFileSave()
{
	QFile::FileError error;

	/* Attempt to save with the existing name. Fall back to Save As. */
	if (fileName().isEmpty() == true)
		error = slotFileSaveAs();
	else
		error = saveXML(fileName());

	handleFileError(error);
	return error;
}

QFile::FileError App::slotFileSaveAs()
{
	QString fn;

	/* Create a file save dialog */
#ifdef __APPLE__
	// Don't create it above the screen...
	QFileDialog dialog(NULL);
#else
	QFileDialog dialog(this);
#endif
	dialog.setWindowTitle(tr("Save Workspace As"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.selectFile(fileName());

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
		return QFile::NoError;

	fn = dialog.selectedFiles().first();
	if (fn.isEmpty() == true)
		return QFile::NoError;

	/* Always use the workspace suffix */
	if (fn.right(4) != KExtWorkspace)
		fn += KExtWorkspace;

	/* Save the document and set workspace name */
	QFile::FileError error = saveXML(fn);
	handleFileError(error);
	return error;
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
	FixtureManager::create(this);
}

void App::slotFunctionManager()
{
	FunctionManager::create(this);
}

void App::slotBusManager()
{
	BusManager::create(this);
}

void App::slotOutputManager()
{
	OutputManager::create(this);
}

void App::slotInputManager()
{
	InputManager::create(this);
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

void App::slotControlVC()
{
	VirtualConsole::create(this);
}

void App::slotControlMonitor()
{
	Monitor::create(this);
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
	QLCDocBrowser* browser = new QLCDocBrowser(this, Qt::Window);
	browser->setAttribute(Qt::WA_DeleteOnClose);
	browser->show();
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

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

void App::setFileName(const QString& fileName)
{
	m_fileName = fileName;
}

QString App::fileName() const
{
	return m_fileName;
}

QFile::FileError App::loadXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QDomDocumentType doctype;
	QString errorString;
	QFile::FileError retval;

	retval = QLCFile::readXML(fileName, &doc);
	if (retval == QFile::NoError)
	{
		if (doc->doctype().name() == KXMLQLCWorkspace)
		{
			if (loadXML(doc) == false)
			{
				retval = QFile::ReadError;
			}
			else
			{
				setFileName(fileName);
				m_doc->resetModified();
				retval = QFile::NoError;
			}
		}
		else
		{
			retval = QFile::ReadError;
		}
	}

	return retval;
}

bool App::loadXML(const QDomDocument* doc)
{
	QDomElement root;
	QDomElement tag;
	QDomNode node;

	Q_ASSERT(m_doc != NULL);
	Q_ASSERT(doc != NULL);

	root = doc->documentElement();
	if (root.tagName() != KXMLQLCWorkspace)
	{
		qWarning() << "Workspace node not found in file!";
		return false;
	}

	node = root.firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCEngine)
		{
			m_doc->loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCMonitor)
		{
			Monitor::loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCVirtualConsole)
		{
			VirtualConsole::loadXML(&tag);
		}
		else if (tag.tagName() == KXMLFixture)
		{
			/* Legacy support code, nowadays in Doc */
			Fixture::loader(&tag, m_doc);
		}
		else if (tag.tagName() == KXMLQLCFunction)
		{
			/* Legacy support code, nowadays in Doc */
			Function::loader(&tag, m_doc);
		}
		else if (tag.tagName() == KXMLQLCBus)
		{
			/* Legacy support code, nowadays in Doc */
			Bus::instance()->loadXML(&tag);
		}
		else if (tag.tagName() == KXMLQLCCreator)
		{
			/* Ignore creator information */
		}
		else
		{
			qDebug() << "Unknown Workspace tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

QFile::FileError App::saveXML(const QString& fileName)
{
	QDomDocument* doc = NULL;
	QFile::FileError retval;
	QDomElement root;
	QDomElement tag;
	QDomText text;

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
		return file.error();

	if (QLCFile::getXMLHeader(KXMLQLCWorkspace, &doc) == true)
	{
		/* Create a text stream for the file */
		QTextStream stream(&file);

		/* THE MASTER XML ROOT NODE */
		root = doc->documentElement();

		/* Write engine components to the XML document */
		m_doc->saveXML(doc, &root);

		/* Write Monitor state to the XML document */
		Monitor::saveXML(doc, &root);

		/* Write virtual console to the XML document */
		VirtualConsole::saveXML(doc, &root);

		/* Write the XML document to the stream (=file) */
		stream << doc->toString() << "\n";

		/* Set the file name for the current Doc instance and
		   set it also in an unmodified state. */
		setFileName(fileName);
		m_doc->resetModified();

		/* Delete the XML document */
		delete doc;

		retval = QFile::NoError;
	}
	else
	{
		retval = QFile::ReadError;
	}

	file.close();

	return retval;
}
