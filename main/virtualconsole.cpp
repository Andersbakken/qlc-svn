/*
  Q Light Controller
  virtualconsole.cpp

  Copyright (c) Heikki Junnila

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

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <QString>
#include <QPoint>
#include <QDebug>
#include <QMenu>
#include <QFile>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "vcdockslider.h"
#include "vcdockarea.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "keybind.h"
#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

extern App* _app;
extern QApplication* _qapp;

VirtualConsole::VirtualConsole(QWidget* parent) : QWidget(parent)
{
	m_dockArea = NULL;
	m_drawArea = NULL;

	m_gridEnabled = true;
	m_gridX = 10;
	m_gridY = 10;

	m_keyRepeatOff = true;
	m_grabKeyboard = true;

	m_selectedWidget = NULL;
	m_clipboardAction = ClipboardNone;

	m_editMenu = NULL;

	// Main layout
	new QHBoxLayout(this);
	layout()->setMargin(0);
	layout()->setSpacing(0);

	// Window title & icon
	parentWidget()->setWindowIcon(QIcon(":/virtualconsole.png"));
	parentWidget()->setWindowTitle(tr("Virtual Console"));

	// Init top menu bar & actions
	initActions();
	initMenuBar();

	// Init left dock area
	initDockArea();

	// Init right drawing area
	setDrawArea(new VCFrame(this));

	// Set some default size
	parentWidget()->resize(300, 400);
}

VirtualConsole::~VirtualConsole()
{
	/* The parent widget is a QMdiSubWindow, which must be deleted now
	   since its child is deleted. */
	parentWidget()->deleteLater();
}

void VirtualConsole::initActions()
{
	/* Add menu actions */
	m_addButtonAction = new QAction(QIcon(":/button.png"),
					tr("Button"), this);
	connect(m_addButtonAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddButton()));

	m_addSliderAction = new QAction(QIcon(":/slider.png"),
					tr("Slider"), this);
	connect(m_addSliderAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddSlider()));

	m_addXYPadAction = new QAction(QIcon(":/xypad.png"),
				       tr("XY pad"), this);
	connect(m_addXYPadAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddXYPad()));

	m_addCueListAction = new QAction(QIcon(":/cuelist.png"),
					 tr("Cue list"), this);
	connect(m_addCueListAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddCueList()));

	m_addFrameAction = new QAction(QIcon(":/frame.png"),
				       tr("Frame"), this);
	connect(m_addFrameAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddFrame()));

	m_addLabelAction = new QAction(QIcon(":/label.png"),
				       tr("Label"), this);
	connect(m_addLabelAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddLabel()));

	/* Tools menu actions */
	m_toolsSettingsAction = new QAction(QIcon(":/configure.png"),
					    tr("Settings"), this);
	connect(m_toolsSettingsAction, SIGNAL(triggered(bool)),
		this, SLOT(slotToolsSettings()));

	m_toolsSlidersAction = new QAction(QIcon(":/slider.png"),
					   tr("Default sliders"), this);
	connect(m_toolsSlidersAction, SIGNAL(triggered(bool)),
		this, SLOT(slotToolsSliders()));

	m_toolsPanicAction = new QAction(QIcon(":/panic.png"),
					 tr("Stop ALL functions!"), this);
	connect(m_toolsPanicAction, SIGNAL(triggered(bool)),
		this, SLOT(slotToolsPanic()));

	/* Edit menu actions */
	m_editCutAction = new QAction(QIcon(":/editcut.png"),
				      tr("Cut"), this);
	connect(m_editCutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditCut()));

	m_editCopyAction = new QAction(QIcon(":/editcopy.png"),
				       tr("Copy"), this);
	connect(m_editCopyAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditCopy()));

	m_editPasteAction = new QAction(QIcon(":/editpaste.png"),
					tr("Paste"), this);
	connect(m_editPasteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditPaste()));

	m_editDeleteAction = new QAction(QIcon(":/editdelete.png"),
					 tr("Delete"), this);
	connect(m_editDeleteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditDelete()));

	m_editPropertiesAction = new QAction(QIcon(":/configure.png"),
					     tr("Properties"), this);
	connect(m_editPropertiesAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditProperties()));

	m_editRenameAction = new QAction(QIcon(":/editclear.png"),
					 tr("Rename"), this);
	connect(m_editRenameAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditRename()));

	/* Foreground menu actions */
	m_fgColorAction = new QAction(QIcon(":/color.png"),
				      tr("Color"), this);
	connect(m_fgColorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotForegroundColor()));

	m_fgFontAction = new QAction(QIcon(":/fonts.png"),
				     tr("Font"), this);
	connect(m_fgFontAction, SIGNAL(triggered(bool)),
		this, SLOT(slotForegroundFont()));

	m_fgDefaultAction = new QAction(QIcon(":/undo.png"),
					tr("Default"), this);
	connect(m_fgDefaultAction, SIGNAL(triggered(bool)),
		this, SLOT(slotForegroundNone()));

	/* Background menu actions */
	m_bgColorAction = new QAction(QIcon(":/color.png"),
				      tr("Color"), this);
	connect(m_bgColorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBackgroundColor()));

	m_bgDefaultAction = new QAction(QIcon(":/undo.png"),
					tr("Default"), this);
	connect(m_bgDefaultAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBackgroundNone()));

	/* Stacking menu actions */
	m_stackingRaiseAction = new QAction(QIcon(":/up.png"),
					    tr("Raise"), this);
	connect(m_stackingRaiseAction, SIGNAL(triggered(bool)),
		this, SLOT(slotStackingRaise()));

	m_stackingLowerAction = new QAction(QIcon(":/down.png"),
					    tr("Lower"), this);
	connect(m_stackingLowerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotStackingLower()));

	/* TODO */
	m_editCutAction->setEnabled(false);
	m_editCopyAction->setEnabled(false);
	m_editPasteAction->setEnabled(false);
}

void VirtualConsole::initMenuBar()
{
	QMenuBar* menuBar;

	menuBar = new QMenuBar(this);
	layout()->setMenuBar(menuBar);

	/* Add menu */
	m_addMenu = new QMenu(menuBar);
	m_addMenu->setTitle(tr("Add"));
	menuBar->addMenu(m_addMenu);
	m_addMenu->addAction(m_addButtonAction);
	m_addMenu->addAction(m_addSliderAction);
	m_addMenu->addAction(m_addXYPadAction);
	m_addMenu->addSeparator();
	m_addMenu->addAction(m_addCueListAction);
	m_addMenu->addSeparator();
	m_addMenu->addAction(m_addFrameAction);
	m_addMenu->addAction(m_addLabelAction);

	/* Edit menu */
	m_editMenu = new QMenu(menuBar);
	m_editMenu->setTitle(tr("Edit"));
	menuBar->addMenu(m_editMenu);
	m_editMenu->addAction(m_editCutAction);
	m_editMenu->addAction(m_editCopyAction);
	m_editMenu->addAction(m_editPasteAction);
	m_editMenu->addAction(m_editDeleteAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_editPropertiesAction);
	m_editMenu->addAction(m_editRenameAction);
	m_editMenu->addSeparator();

	/* Tools menu */
	m_toolsMenu = new QMenu(menuBar);
	m_toolsMenu->setTitle(tr("Tools"));
	menuBar->addMenu(m_toolsMenu);
	m_toolsMenu->addAction(m_toolsPanicAction);
	m_toolsMenu->addAction(m_toolsSlidersAction);
	m_toolsMenu->addSeparator();
	m_toolsMenu->addAction(m_toolsSettingsAction);

	/* Foreground menu */
	QMenu* fgMenu = new QMenu(m_editMenu);
	fgMenu->setTitle(tr("Foreground"));
	m_editMenu->addMenu(fgMenu);
	fgMenu->addAction(m_fgColorAction);
	fgMenu->addAction(m_fgFontAction);
	fgMenu->addAction(m_fgDefaultAction);

	/* Background Menu */
	QMenu* bgMenu = new QMenu(m_editMenu);
	bgMenu->setTitle(tr("Background"));
	m_editMenu->addMenu(bgMenu);
	bgMenu->addAction(m_bgColorAction);
	bgMenu->addAction(m_bgDefaultAction);

	/* Stacking order menu */
	QMenu* stackMenu = new QMenu(m_editMenu);
	stackMenu->setTitle(tr("Stacking order"));
	m_editMenu->addMenu(stackMenu);
	stackMenu->addAction(m_stackingRaiseAction);
	stackMenu->addAction(m_stackingLowerAction);
}

void VirtualConsole::initDockArea()
{
	if (m_dockArea != NULL)
		delete m_dockArea;

	m_dockArea = new VCDockArea(this);
	m_dockArea->setSizePolicy(QSizePolicy::Maximum,
				  QSizePolicy::Expanding);

	// Add the dock area into the master horizontal layout
	layout()->addWidget(m_dockArea);

	/* Show the dock area by default */
	m_dockArea->show();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VirtualConsole::loader(QDomDocument* doc, QDomElement* vc_root)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	return _app->virtualConsole()->loadXML(doc, vc_root);
}

bool VirtualConsole::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVirtualConsole)
	{
		qDebug() << "Virtual Console node not found!";
		return false;
	}

	/* Timer type */
	if (root->attribute(KXMLQLCVirtualConsoleTimerType) ==
	    KXMLQLCVirtualConsoleTimerHardware)
	{
		_app->functionConsumer()->setTimerType(
			FunctionConsumer::RTCTimer);
	}
	else
	{
		_app->functionConsumer()->setTimerType(
			FunctionConsumer::NanoSleepTimer);
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
						    &visible);
		}
		else if (tag.tagName() == KXMLQLCVirtualConsoleGrid)
		{
			str = tag.attribute(KXMLQLCVirtualConsoleGridXResolution);
			setGridX(str.toInt());

			str = tag.attribute(KXMLQLCVirtualConsoleGridYResolution);
			setGridY(str.toInt());

			str = tag.attribute(KXMLQLCVirtualConsoleGridEnabled);
			setGridEnabled((bool) str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVirtualConsoleKeyboard)
		{
			str = tag.attribute(KXMLQLCVirtualConsoleKeyboardGrab);
			setGrabKeyboard((bool) str.toInt());

			str = tag.attribute(KXMLQLCVirtualConsoleKeyboardRepeat);
			setKeyRepeatOff((bool) str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVCDockArea)
		{
			m_dockArea->loadXML(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCFrame)
		{
			VCFrame::loader(doc, &tag, this);
		}
		else
		{
			qDebug() << "Unknown virtual console tag:"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	parentWidget()->setGeometry(x, y, w, h);
	if (visible == true)
		parentWidget()->showNormal();
	else
		parentWidget()->hide();

	return true;
}

bool VirtualConsole::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Virtual Console entry */
	root = doc->createElement(KXMLQLCVirtualConsole);
	wksp_root->appendChild(root);
	if (_app->functionConsumer()->timerType() ==
	    FunctionConsumer::NanoSleepTimer)
	{
		root.setAttribute(KXMLQLCVirtualConsoleTimerType,
				  KXMLQLCVirtualConsoleTimerSoftware);
	}
	else
	{
		root.setAttribute(KXMLQLCVirtualConsoleTimerType,
				  KXMLQLCVirtualConsoleTimerHardware);
	}

	/* Save window state */
	QLCFile::saveXMLWindowState(doc, &root, parentWidget());

	/* Grid */
	tag = doc->createElement(KXMLQLCVirtualConsoleGrid);
	root.appendChild(tag);
	str.setNum((m_gridEnabled) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleGridEnabled, str);
	str.setNum(m_gridX);
	tag.setAttribute(KXMLQLCVirtualConsoleGridXResolution, str);
	str.setNum(m_gridY);
	tag.setAttribute(KXMLQLCVirtualConsoleGridYResolution, str);

	/* Keyboard settings */
	tag = doc->createElement(KXMLQLCVirtualConsoleKeyboard);
	root.appendChild(tag);
	str.setNum((m_grabKeyboard) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleKeyboardGrab, str);
	str.setNum((m_keyRepeatOff) ? 1 : 0);
	tag.setAttribute(KXMLQLCVirtualConsoleKeyboardRepeat, str);

	/* Dock Area */
	m_dockArea->saveXML(doc, &root);

	/* Save children */
	if (m_drawArea != NULL)
		m_drawArea->saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Selected widget
 *****************************************************************************/

void VirtualConsole::setSelectedWidget(VCWidget* w)
{
	if (m_selectedWidget != NULL)
	{
		VCWidget* old = m_selectedWidget;
		m_selectedWidget = w;
		old->update();
	}
	else
	{
		m_selectedWidget = w;
	}

	if (m_selectedWidget != NULL)
	{
		m_selectedWidget->update();
	}
	else
	{
		/* Usually the selected widget is NULL only when we have
		   deleted some widget from virtual console. So, clear the
		   clipboard as well so that we don't end up pasting an
		   invalid object -> segfault. */
		clearClipboard();
	}
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

void VirtualConsole::cut(QList <VCWidget*> *widgets)
{
	Q_ASSERT(widgets != NULL);

	/* Clipboard will cut the widgets when paste is invoked */
	m_clipboardAction = ClipboardCut;

	/* Copy the contents of the widget list. Just pointers. */
	m_clipboard = *widgets;
}

void VirtualConsole::copy(QList <VCWidget*> *widgets)
{
	Q_ASSERT(widgets != NULL);

	/* Clipboard will copy the widgets when paste is invoked */
	m_clipboardAction = ClipboardCopy;

	/* Copy the contents of the widget list. Just pointers. */
	m_clipboard = *widgets;
}

void VirtualConsole::paste(VCFrame* parent, QPoint point)
{
	QListIterator <VCWidget*> it(m_clipboard);
	while (it.hasNext() == true)
	{
		VCWidget* widget = it.next();
		if (m_clipboardAction == ClipboardCut)
		{
			widget->setParent(parent);
			widget->move(point);
			widget->show();
		}
		else if (m_clipboardAction == ClipboardCopy)
		{
			copyWidget(widget, parent, point);
		}
		else
		{
			qDebug() << "Cannot paste from an empty clipboard!";
		}
	}

	/* If the action was about cutting something, the originals are removed
	   by now and moved to another parent. From now on, paste actions just
	   create copies of them, and not move them any further. */
	m_clipboardAction = ClipboardCopy;
}

void VirtualConsole::copyWidget(VCWidget*, VCFrame*, QPoint)
{
	/* TODO:
	   Create a copy of the widget and place it into the parent,
	   at the given point */
}

void VirtualConsole::clearClipboard()
{
	m_clipboard.clear();
	m_clipboardAction = ClipboardNone;
}

/*****************************************************************************
 * Draw area
 *****************************************************************************/

void VirtualConsole::setDrawArea(VCFrame* drawArea)
{
	Q_ASSERT(drawArea != NULL);
	Q_ASSERT(layout() != NULL);

	if (m_drawArea != NULL)
		delete m_drawArea;
	m_drawArea = drawArea;

	/* Add the draw area into the master horizontal layout */
	layout()->addWidget(m_drawArea);
	m_drawArea->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

void VirtualConsole::slotAddButton()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddButton();
}

void VirtualConsole::slotAddSlider()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddSlider();
}

void VirtualConsole::slotAddXYPad()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddXYPad();
}

void VirtualConsole::slotAddCueList()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddCueList();
}
void VirtualConsole::slotAddFrame()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddFrame();
}

void VirtualConsole::slotAddLabel()
{
	VCFrame* frame;
	if (m_selectedWidget != NULL &&
	    m_selectedWidget->objectName() == "VCFrame")
		frame = qobject_cast<VCFrame*>(m_selectedWidget);
	else
		frame = m_drawArea;

	Q_ASSERT(frame != NULL);
	frame->slotAddLabel();
}

/*********************************************************************
 * Tools menu callbacks
 *********************************************************************/

void VirtualConsole::slotToolsSettings()
{
	VirtualConsoleProperties prop(this);
	t_input_universe uni;
	t_input_channel ch;
	t_bus_value lo;
	t_bus_value hi;

	Q_ASSERT(m_dockArea != NULL);

	/* Layout */
	prop.setKeyRepeatOff(m_keyRepeatOff);
	prop.setGrabKeyboard(m_grabKeyboard);
	prop.setGrid(m_gridEnabled, m_gridX, m_gridY);

	/* Default sliders */
	lo = m_dockArea->defaultFadeSlider()->busLowLimit();
	hi = m_dockArea->defaultFadeSlider()->busHighLimit();
	prop.setFadeLimits(lo, hi);

	lo = m_dockArea->defaultHoldSlider()->busLowLimit();
	hi = m_dockArea->defaultHoldSlider()->busHighLimit();
	prop.setHoldLimits(lo, hi);

	uni = m_dockArea->defaultFadeSlider()->inputUniverse();
	ch = m_dockArea->defaultFadeSlider()->inputChannel();
	prop.setFadeInputSource(uni, ch);

	uni = m_dockArea->defaultHoldSlider()->inputUniverse();
	ch = m_dockArea->defaultHoldSlider()->inputChannel();
	prop.setHoldInputSource(uni, ch);

	/* Timer */
	prop.setTimerType(_app->functionConsumer()->timerType());

	if (prop.exec() == QDialog::Accepted)
	{
		setGridEnabled(prop.isGridEnabled());
		setGridX(prop.gridX());
		setGridY(prop.gridY());

		setKeyRepeatOff(prop.isKeyRepeatOff());
		setGrabKeyboard(prop.isGrabKeyboard());

		lo = prop.fadeLowLimit();
		hi = prop.fadeHighLimit();
		m_dockArea->defaultFadeSlider()->setBusRange(lo, hi);

		lo = prop.holdLowLimit();
		hi = prop.holdHighLimit();
		m_dockArea->defaultHoldSlider()->setBusRange(lo, hi);

		uni = prop.fadeInputUniverse();
		ch = prop.fadeInputChannel();
		m_dockArea->defaultFadeSlider()->setInputSource(uni, ch);

		uni = prop.holdInputUniverse();
		ch = prop.holdInputChannel();
		m_dockArea->defaultHoldSlider()->setInputSource(uni, ch);

		_app->functionConsumer()->setTimerType(prop.timerType());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotToolsSliders()
{
	if (m_dockArea->isHidden())
		m_dockArea->show();
	else
		m_dockArea->hide();

	_app->doc()->setModified();
}

void VirtualConsole::slotToolsPanic()
{
	// Panic button pressed: stop all running functions
	_app->slotControlPanic();
}

/*********************************************************************
 * Edit menu callbacks
 *********************************************************************/

void VirtualConsole::slotEditCut()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditCopy()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditPaste()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VirtualConsole::slotEditDelete()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotDelete();
}

void VirtualConsole::slotEditProperties()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotProperties();
}

void VirtualConsole::slotEditRename()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotRename();
}

/*********************************************************************
 * Foreground menu callbacks
 *********************************************************************/

void VirtualConsole::slotForegroundFont()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseFont();
}

void VirtualConsole::slotForegroundColor()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseForegroundColor();
}

void VirtualConsole::slotForegroundNone()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotResetForegroundColor();
}

/*********************************************************************
 * Background menu callbacks
 *********************************************************************/

void VirtualConsole::slotBackgroundColor()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseBackgroundColor();
}

void VirtualConsole::slotBackgroundImage()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotChooseBackgroundImage();
}

void VirtualConsole::slotBackgroundNone()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->slotResetBackgroundColor();
}

/*********************************************************************
 * Stacking menu callbacks
 *********************************************************************/

void VirtualConsole::slotStackingRaise()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->raise();
}

void VirtualConsole::slotStackingLower()
{
	if (m_selectedWidget != NULL)
		m_selectedWidget->lower();
}

/*********************************************************************
 * Misc callbacks
 *********************************************************************/

void VirtualConsole::slotModeChanged(App::Mode mode)
{
	QString config;

	/* Key repeat */
	if (isKeyRepeatOff() == true)
	{
#if !defined(WIN32) && !defined(__APPLE__)
		Display* display;
		display = XOpenDisplay(NULL);
		Q_ASSERT(display != NULL);

		if (mode == App::Design)
			XAutoRepeatOn(display);
		else
			XAutoRepeatOff(display);
		
		XCloseDisplay(display);
#else
#endif
	}
	
	/* Grab keyboard */
	if (isGrabKeyboard() == true)
	{
		if (mode == App::Design)
			releaseKeyboard();
		else
			grabKeyboard();
	}

	if (mode == App::Operate)
	{
		// Don't allow edits in operate mode
		m_editMenu->setEnabled(false);
		m_addMenu->setEnabled(false);
		m_toolsSettingsAction->setEnabled(false);
	}
	else
	{
		// Allow edits in design mode
		m_editMenu->setEnabled(true);
		m_addMenu->setEnabled(true);
		m_toolsSettingsAction->setEnabled(true);
	}

	/* Patch the event thru to all children */
	emit modeChanged(mode);
}

/*********************************************************************
 * Event handlers
 *********************************************************************/

void VirtualConsole::closeEvent(QCloseEvent* e)
{
	e->accept();
	emit closed();
}

void VirtualConsole::keyPressEvent(QKeyEvent* e)
{
	if (_app->mode() == App::Operate)
	{
		emit keyPressed(e);
		e->accept();
	}
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* e)
{
	if (_app->mode() == App::Operate)
	{
		emit keyReleased(e);
		e->accept();
	}
}
