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
#include "common/qlctypes.h"
#include "common/qlcfile.h"

#define KModeTextOperate tr("Operate")
#define KModeTextDesign tr("Design")

App* _app;
QStyle* App::s_saneStyle = NULL;

/*********************************************************************
 * Initialization
 *********************************************************************/

App::App() : QMainWindow()
{
	_app = this;

	m_outputMap = NULL;
	m_inputMap = NULL;
	m_functionConsumer = NULL;
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

	setWindowTitle(tr("%1 - New Workspace").arg(KApplicationNameLong));
}

App::~App()
{
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
#ifdef __APPLE__
        loadFixtureDefinitions(QString("%1/%2")
                               .arg(QApplication::applicationDirPath())
                               .arg(FIXTUREDIR));
#else
	loadFixtureDefinitions(FIXTUREDIR);
#endif

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
}

/*****************************************************************************
 * Function consumer
 *****************************************************************************/

void App::initFunctionConsumer()
{
	Q_ASSERT(m_outputMap != NULL);

	m_functionConsumer = new FunctionConsumer(this, m_outputMap);
	Q_ASSERT(m_functionConsumer != NULL);

	/* Start function consumer */
	m_functionConsumer->start();
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::initDoc()
{
	// Delete existing document object and create a new one
	if (m_doc != NULL)
		delete m_doc;
	m_doc = new Doc();

	connect(m_doc, SIGNAL(modified(bool)),
		this, SLOT(slotDocModified(bool)));

	connect(this, SIGNAL(modeChanged(App::Mode)),
		m_doc, SLOT(slotModeChanged(App::Mode)));

	emit documentChanged(m_doc);
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
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCFixtureDef* fxi;
		QString path;

		path = dir.absoluteFilePath(it.next());

		fxi = new QLCFixtureDef();
		Q_ASSERT(fxi != NULL);

		if (fxi->loadXML(path) == true)
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
			m_functionConsumer->stopAll();
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
	centralWidget()->setContextMenuPolicy(Qt::CustomContextMenu);

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
					    tr("OOutputs"), this);
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

	m_controlPanicAction = new QAction(QIcon(":/panic.png"),
					   tr("&Panic!"), this);
	m_controlPanicAction->setShortcut(QKeySequence("CTRL+SHIFT+ESC"));
	connect(m_controlPanicAction, SIGNAL(triggered(bool)),
		this, SLOT(slotControlPanic()));

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
	m_controlMenu->addAction(m_controlPanicAction);
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

	setWindowTitle(tr("%1 - New Workspace").arg(KApplicationNameLong));

	doc()->resetModified();
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
	dialog.setWindowTitle(tr("Open Workspace"));
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

	if (FunctionManager::instance() != NULL)
		FunctionManager::instance()->updateTree();
	if (OutputManager::instance() != NULL)
		InputManager::instance()->updateTree();
	if (InputManager::instance() != NULL)
		InputManager::instance()->updateTree();
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
	dialog.setWindowTitle(tr("Save Workspace As"));
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

void App::slotControlPanic()
{
	/* Shut down all running functions */
	m_functionConsumer->stopAll();
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
