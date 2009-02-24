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
#include <QInputDialog>
#include <QColorDialog>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QCloseEvent>
#include <QMetaObject>
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
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
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
	m_editActionGroup = NULL;
	m_addActionGroup = NULL;
	m_bgActionGroup = NULL;
	m_fgActionGroup = NULL;
	m_fontActionGroup = NULL;
	m_frameActionGroup = NULL;
	m_stackingActionGroup = NULL;

	m_customMenu = NULL;
	m_toolsMenu = NULL;
	m_editMenu = NULL;
	m_addMenu = NULL;

	m_dockArea = NULL;
	m_drawArea = NULL;

	m_gridEnabled = true;
	m_gridX = 10;
	m_gridY = 10;

	m_keyRepeatOff = true;
	m_grabKeyboard = true;

	m_editAction = EditNone;
	m_editMenu = NULL;

	// Main layout
	new QHBoxLayout(this);

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
	parentWidget()->resize(640, 480);
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

	/* Put add actions under the same group */
	m_addActionGroup = new QActionGroup(this);
	m_addActionGroup->setExclusive(false);
	m_addActionGroup->addAction(m_addButtonAction);
	m_addActionGroup->addAction(m_addSliderAction);
	m_addActionGroup->addAction(m_addXYPadAction);
	m_addActionGroup->addAction(m_addCueListAction);
	m_addActionGroup->addAction(m_addFrameAction);
	m_addActionGroup->addAction(m_addLabelAction);
	
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
	m_editCutAction->setShortcut(Qt::CTRL + Qt::Key_X);
	connect(m_editCutAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditCut()));

	m_editCopyAction = new QAction(QIcon(":/editcopy.png"),
				       tr("Copy"), this);
	m_editCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);
	connect(m_editCopyAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditCopy()));

	m_editPasteAction = new QAction(QIcon(":/editpaste.png"),
					tr("Paste"), this);
	m_editPasteAction->setShortcut(Qt::CTRL + Qt::Key_V);
	m_editPasteAction->setEnabled(false);
	connect(m_editPasteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditPaste()));

	m_editDeleteAction = new QAction(QIcon(":/editdelete.png"),
					 tr("Delete"), this);
	m_editDeleteAction->setShortcut(Qt::Key_Delete);
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

	/* Put edit actions under the same group */
	m_editActionGroup = new QActionGroup(this);
	m_editActionGroup->setExclusive(false);
	m_editActionGroup->addAction(m_editCutAction);
	m_editActionGroup->addAction(m_editCopyAction);
	m_editActionGroup->addAction(m_editPasteAction);
	m_editActionGroup->addAction(m_editDeleteAction);
	m_editActionGroup->addAction(m_editPropertiesAction);
	m_editActionGroup->addAction(m_editRenameAction);

	/* Background menu actions */
	m_bgColorAction = new QAction(QIcon(":/color.png"),
				      tr("Color"), this);
	connect(m_bgColorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBackgroundColor()));

	m_bgImageAction = new QAction(QIcon(":/image.png"),
				      tr("Image"), this);
	connect(m_bgImageAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBackgroundImage()));

	m_bgDefaultAction = new QAction(QIcon(":/undo.png"),
					tr("Default"), this);
	connect(m_bgDefaultAction, SIGNAL(triggered(bool)),
		this, SLOT(slotBackgroundNone()));

	/* Put BG actions under the same group */
	m_bgActionGroup = new QActionGroup(this);
	m_bgActionGroup->setExclusive(false);
	m_bgActionGroup->addAction(m_bgColorAction);
	m_bgActionGroup->addAction(m_bgImageAction);
	m_bgActionGroup->addAction(m_bgDefaultAction);

	/* Foreground menu actions */
	m_fgColorAction = new QAction(QIcon(":/color.png"),
				      tr("Color"), this);
	connect(m_fgColorAction, SIGNAL(triggered(bool)),
		this, SLOT(slotForegroundColor()));

	m_fgDefaultAction = new QAction(QIcon(":/undo.png"),
					tr("Default"), this);
	connect(m_fgDefaultAction, SIGNAL(triggered(bool)),
		this, SLOT(slotForegroundNone()));

	/* Put FG actions under the same group */
	m_fgActionGroup = new QActionGroup(this);
	m_fgActionGroup->setExclusive(false);
	m_fgActionGroup->addAction(m_fgColorAction);
	m_fgActionGroup->addAction(m_fgDefaultAction);

	/* Font menu actions */
	m_fontAction = new QAction(QIcon(":/fonts.png"),
				   tr("Font"), this);
	connect(m_fontAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFont()));

	m_resetFontAction = new QAction(QIcon(":/undo.png"),
				   tr("Default"), this);
	connect(m_resetFontAction, SIGNAL(triggered(bool)),
		this, SLOT(slotResetFont()));

	/* Put font actions under the same group */
	m_fontActionGroup = new QActionGroup(this);
	m_fontActionGroup->setExclusive(false);
	m_fontActionGroup->addAction(m_fontAction);
	m_fontActionGroup->addAction(m_resetFontAction);

	/* Frame menu actions */
	m_frameSunkenAction = new QAction(QIcon(":/framesunken.png"),
					  tr("Sunken"), this);
	connect(m_frameSunkenAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFrameSunken()));

	m_frameRaisedAction = new QAction(QIcon(":/frameraised.png"),
					  tr("Raised"), this);
	connect(m_frameRaisedAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFrameRaised()));

	m_frameNoneAction = new QAction(QIcon(":/framenone.png"),
					  tr("None"), this);
	connect(m_frameNoneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotFrameNone()));

	/* Put frame actions under the same group */
	m_frameActionGroup = new QActionGroup(this);
	m_frameActionGroup->setExclusive(false);
	m_frameActionGroup->addAction(m_frameRaisedAction);
	m_frameActionGroup->addAction(m_frameSunkenAction);
	m_frameActionGroup->addAction(m_frameNoneAction);

	/* Stacking menu actions */
	m_stackingRaiseAction = new QAction(QIcon(":/up.png"),
					    tr("Raise"), this);
	connect(m_stackingRaiseAction, SIGNAL(triggered(bool)),
		this, SLOT(slotStackingRaise()));

	m_stackingLowerAction = new QAction(QIcon(":/down.png"),
					    tr("Lower"), this);
	connect(m_stackingLowerAction, SIGNAL(triggered(bool)),
		this, SLOT(slotStackingLower()));

	/* Put stacking actions under the same group */
	m_stackingActionGroup = new QActionGroup(this);
	m_stackingActionGroup->setExclusive(false);
	m_stackingActionGroup->addAction(m_stackingRaiseAction);
	m_stackingActionGroup->addAction(m_stackingLowerAction);
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

	/* Background Menu */
	QMenu* bgMenu = new QMenu(m_editMenu);
	bgMenu->setTitle(tr("Background"));
	m_editMenu->addMenu(bgMenu);
	bgMenu->addAction(m_bgColorAction);
	bgMenu->addAction(m_bgImageAction);
	bgMenu->addAction(m_bgDefaultAction);

	/* Foreground menu */
	QMenu* fgMenu = new QMenu(m_editMenu);
	fgMenu->setTitle(tr("Foreground"));
	m_editMenu->addMenu(fgMenu);
	fgMenu->addAction(m_fgColorAction);
	fgMenu->addAction(m_fgDefaultAction);

	/* Font menu */
	QMenu* fontMenu = new QMenu(m_editMenu);
	fontMenu->setTitle(tr("Font"));
	m_editMenu->addMenu(fontMenu);
	fontMenu->addAction(m_fontAction);
	fontMenu->addAction(m_resetFontAction);

	/* Frame menu */
	QMenu* frameMenu = new QMenu(m_editMenu);
	frameMenu->setTitle(tr("Frame"));
	m_editMenu->addMenu(frameMenu);
	frameMenu->addAction(m_frameSunkenAction);
	frameMenu->addAction(m_frameRaisedAction);
	frameMenu->addAction(m_frameNoneAction);

	/* Stacking order menu */
	QMenu* stackMenu = new QMenu(m_editMenu);
	stackMenu->setTitle(tr("Stacking order"));
	m_editMenu->addMenu(stackMenu);
	stackMenu->addAction(m_stackingRaiseAction);
	stackMenu->addAction(m_stackingLowerAction);

	/* Add a separator that separates the common edit items from a custom
	   widget menu that gets appended to the edit menu when a selected
	   widget provides one. */
	m_editMenu->addSeparator();
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
			Q_ASSERT(m_dockArea != NULL);
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
 * Draw area
 *****************************************************************************/

void VirtualConsole::setDrawArea(VCFrame* drawArea)
{
	Q_ASSERT(layout() != NULL);

	if (m_drawArea != NULL)
		delete m_drawArea;
	m_drawArea = drawArea;

	/* Add the draw area into the master horizontal layout */
	layout()->addWidget(m_drawArea);
	m_drawArea->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);

	/* Update actions' enabled status */
	updateActions();
}

/*****************************************************************************
 * Selected widget
 *****************************************************************************/

void VirtualConsole::setWidgetSelected(VCWidget* widget, bool select)
{
	Q_ASSERT(widget != NULL);

	if (select == false)
	{
		m_selectedWidgets.removeAll(widget);
		widget->update();
	}
	else if (select == true && m_selectedWidgets.indexOf(widget) == -1)
	{
		m_selectedWidgets.append(widget);
		widget->update();
	}

	/* Change the custom menu to the latest-selected widget's menu */
	updateCustomMenu();

	/* Enable or disable actions */
	updateActions();
}

bool VirtualConsole::isWidgetSelected(VCWidget* widget) const
{
	if (widget == NULL || m_selectedWidgets.indexOf(widget) == -1)
		return false;
	else
		return true;
}

void VirtualConsole::clearWidgetSelection()
{
	/* Get a copy of selected widget list */
	QList <VCWidget*> widgets(m_selectedWidgets);

	/* Clear the list so isWidgetSelected() returns false for all widgets */
	m_selectedWidgets.clear();

	/* Update all widgets to clear the selection frame around them */
	QListIterator <VCWidget*> it(widgets);
	while (it.hasNext() == true)
		it.next()->update();

	/* Change the custom menu to the latest-selected widget's menu */
	updateCustomMenu();

	/* Enable or disable actions */
	updateActions();
}

void VirtualConsole::updateCustomMenu()
{
	/* Get rid of the custom menu, but delete it later because this might
	   be called from the very menu that is being deleted. */
	if (m_customMenu != NULL)
	{
		delete m_customMenu;
		m_customMenu = NULL;
	}

	if (m_selectedWidgets.count() > 0)
	{
		/* Change the custom menu to the last selected widget's menu */
		VCWidget* latestWidget = m_selectedWidgets.last();
		m_customMenu = latestWidget->customMenu(m_editMenu);
		if (m_customMenu != NULL)
			m_editMenu->addMenu(m_customMenu);
	}
	else
	{
		/* Change the custom menu to the bottom frame's menu */
		m_customMenu = m_drawArea->customMenu(m_editMenu);
		if (m_customMenu != NULL)
			m_editMenu->addMenu(m_customMenu);
	}
}

void VirtualConsole::updateActions()
{
	bool enable;

	/* When selected widgets is empty, all actions go to main draw area. */
	if (m_selectedWidgets.isEmpty() == true)
	{
		m_addActionGroup->setEnabled(true);

		m_editActionGroup->setEnabled(false);
		m_fgActionGroup->setEnabled(true); /* Enable draw area BG */
		m_fgActionGroup->setEnabled(false);
		m_fontActionGroup->setEnabled(false);
		m_frameActionGroup->setEnabled(false);
		m_stackingActionGroup->setEnabled(false);
	}
	else
	{
		m_editActionGroup->setEnabled(true);
		m_fgActionGroup->setEnabled(true);
		m_fgActionGroup->setEnabled(true);
		m_fontActionGroup->setEnabled(true);
		m_frameActionGroup->setEnabled(true);
		m_stackingActionGroup->setEnabled(true);

		/* Need to check, whether the last selected widget can hold
		   children, since it will get new additions if possible. */
		if (m_selectedWidgets.last()->canHaveChildren() == true)
			m_addActionGroup->setEnabled(true);
		else
			m_addActionGroup->setEnabled(false);
	}
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

void VirtualConsole::slotAddButton()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCButton* button = new VCButton(parent);
		Q_ASSERT(button != NULL);
		button->show();

		if (parent->buttonBehaviour() == VCFrame::Exclusive)
			button->setExclusive(true);
		else
			button->setExclusive(false);

		button->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotAddSlider()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCSlider* slider = new VCSlider(parent);
		Q_ASSERT(slider != NULL);
		slider->show();

		slider->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotAddXYPad()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCXYPad* xypad = new VCXYPad(parent);
		Q_ASSERT(xypad != NULL);
		xypad->show();

		xypad->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotAddCueList()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCCueList* cuelist = new VCCueList(parent);
		Q_ASSERT(cuelist != NULL);
		cuelist->show();

		cuelist->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotAddFrame()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCFrame* frame = new VCFrame(parent);
		Q_ASSERT(frame != NULL);
		frame->show();

		frame->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotAddLabel()
{
	VCFrame* parent;

	/* Either add to the draw area or the latest selected widget (but only
	   if it's a VCFrame). */
	if (m_selectedWidgets.isEmpty() == true)
		parent = m_drawArea;
	else
		parent = qobject_cast<VCFrame*> (m_selectedWidgets.last());

	if (parent != NULL)
	{
		VCLabel* label = new VCLabel(parent);
		Q_ASSERT(label != NULL);
		label->show();

		label->move(parent->lastClickPoint());

		_app->doc()->setModified();
	}
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
	/* Make the edit action valid only if there's something to cut */
	if (m_selectedWidgets.count() == 0)
	{
		m_editAction = EditNone;
		m_clipboard.clear();
		m_editPasteAction->setEnabled(false);
	}
	else
	{
		m_editAction = EditCut;
		m_clipboard = m_selectedWidgets;
		m_editPasteAction->setEnabled(true);
	}
}

void VirtualConsole::slotEditCopy()
{
	/* Make the edit action valid only if there's something to copy */
	if (m_selectedWidgets.count() == 0)
	{
		m_editAction = EditNone;
		m_clipboard.clear();
		m_editPasteAction->setEnabled(false);
	}
	else
	{
		m_editAction = EditCopy;
		m_clipboard = m_selectedWidgets;
		m_editPasteAction->setEnabled(true);
	}
}

void VirtualConsole::slotEditPaste()
{
	if (m_clipboard.count() == 0)
	{
		/* Invalidate the edit action if there's nothing to paste */
		m_editAction = EditNone;
		m_editPasteAction->setEnabled(false);
	}
	else if (m_editAction == EditCut)
	{
		VCWidget* parent;

		/* Select the parent that gets the cut clipboard contents */
		if (m_selectedWidgets.count() == 0)
			parent = _app->virtualConsole()->drawArea();
		else
			parent = m_selectedWidgets.last();

		/* Move each widget to the new parent */
		QMutableListIterator <VCWidget*> it(m_clipboard);
		while (it.hasNext() == true)
		{
			VCWidget* widget = it.next();
			Q_ASSERT(widget != NULL);
			widget->setParent(parent);
			widget->show();
		}

		/* Clear clipboard after pasting stuff that was CUT */
		m_clipboard.clear();
		m_editPasteAction->setEnabled(false);
	}
	else if (m_editAction == EditCopy)
	{
		VCWidget* parent;

		/* Select the parent that gets the copied clipboard contents */
		if (m_selectedWidgets.count() == 0)
			parent = _app->virtualConsole()->drawArea();
		else
			parent = m_selectedWidgets.last();

		/* Copy each widget to the parent */
		QMutableListIterator <VCWidget*> it(m_clipboard);
		while (it.hasNext() == true)
		{
			VCWidget* widget = it.next()->createCopy(parent);
			Q_ASSERT(widget != NULL);
			widget->show();
		}
	}
}

void VirtualConsole::slotEditDelete()
{
	QString msg(tr("Do you wish to delete the selected widgets?"));
	QString title(tr("Delete widgets"));
	int result = QMessageBox::question(this, title, msg,
					   QMessageBox::Yes,
					   QMessageBox::No);
	if (result == QMessageBox::Yes)
	{
		while (m_selectedWidgets.isEmpty() == false)
		{
			/* Consume the selected list until it is empty and
			   delete each widget. */
			VCWidget* widget = m_selectedWidgets.takeFirst();
			widget->deleteLater();

			/* Remove the widget from clipboard as well so that
			   deleted widgets won't be pasted anymore anywhere */
			m_clipboard.removeAll(widget);
			m_editPasteAction->setEnabled(false);
		}
	}
}

void VirtualConsole::slotEditProperties()
{
	if (m_selectedWidgets.isEmpty() == true)
		return;

	VCWidget* widget = m_selectedWidgets.last();
	if (widget != NULL)
		widget->editProperties();
}

void VirtualConsole::slotEditRename()
{
	if (m_selectedWidgets.isEmpty() == true)
		return;

	bool ok = false;
	QString text(m_selectedWidgets.last()->caption());
	text = QInputDialog::getText(this, tr("Rename widgets"), tr("Caption:"),
				     QLineEdit::Normal, text, &ok);
	if (ok == true)
	{
		VCWidget* widget;
		foreach(widget, m_selectedWidgets)
			widget->setCaption(text);
	}
}

/*********************************************************************
 * Background menu callbacks
 *********************************************************************/

void VirtualConsole::slotBackgroundColor()
{
	QColor color;

	if (m_selectedWidgets.isEmpty() == true)
		color = m_drawArea->backgroundColor();
	else
		color = m_selectedWidgets.last()->backgroundColor();

	color = QColorDialog::getColor(color);
	if (color.isValid() == true)
	{
		if (m_selectedWidgets.isEmpty() == true)
		{
			m_drawArea->setBackgroundColor(color);
		}
		else
		{
			VCWidget* widget;
			foreach(widget, m_selectedWidgets)
				widget->setBackgroundColor(color);
		}
	}
}

void VirtualConsole::slotBackgroundImage()
{
	QString path;

	if (m_selectedWidgets.isEmpty() == true)
		path = m_drawArea->backgroundImage();
	else
		path = m_selectedWidgets.last()->backgroundImage();

	path = QFileDialog::getOpenFileName(this,
					    tr("Select background image"),
					    path,
					    "Images (*.png *.xpm *.jpg *.gif)");
	if (path.isEmpty() == false)
	{
		if (m_selectedWidgets.isEmpty() == true)
		{
			m_drawArea->setBackgroundImage(path);
		}
		else
		{
			VCWidget* widget;
			foreach(widget, m_selectedWidgets)
				widget->setBackgroundImage(path);
		}
	}
}

void VirtualConsole::slotBackgroundNone()
{
	if (m_selectedWidgets.isEmpty() == true)
	{
		m_drawArea->resetBackgroundColor();
	}
	else
	{
		VCWidget* widget;
		foreach(widget, m_selectedWidgets)
			widget->resetBackgroundColor();
	}
}

/*********************************************************************
 * Foreground menu callbacks
 *********************************************************************/

void VirtualConsole::slotForegroundColor()
{
	if (m_selectedWidgets.isEmpty() == true)
		return;

	QColor color(m_selectedWidgets.last()->foregroundColor());
	color = QColorDialog::getColor(color);
	if (color.isValid() == true)
	{
		VCWidget* widget;
		foreach(widget, m_selectedWidgets)
			widget->setForegroundColor(color);
	}
}

void VirtualConsole::slotForegroundNone()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->resetForegroundColor();
}

/*********************************************************************
 * Font menu callbacks
 *********************************************************************/

void VirtualConsole::slotFont()
{
	if (m_selectedWidgets.isEmpty() == true)
		return;

	bool ok = false;
	QFont font(m_selectedWidgets.last()->font());
	font = QFontDialog::getFont(&ok, font);
	if (ok == true)
	{
		VCWidget* widget;
		foreach(widget, m_selectedWidgets)
			widget->setFont(font);
	}
}

void VirtualConsole::slotResetFont()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->resetFont();
}

/*********************************************************************
 * Stacking menu callbacks
 *********************************************************************/

void VirtualConsole::slotStackingRaise()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->raise();
}

void VirtualConsole::slotStackingLower()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->lower();
}

/*********************************************************************
 * Frame menu callbacks
 *********************************************************************/

void VirtualConsole::slotFrameSunken()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->setFrameStyle(KVCFrameStyleSunken);
}

void VirtualConsole::slotFrameRaised()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->setFrameStyle(KVCFrameStyleRaised);
}

void VirtualConsole::slotFrameNone()
{
	VCWidget* widget;
	foreach(widget, m_selectedWidgets)
		widget->setFrameStyle(KVCFrameStyleNone);
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
		// Don't allow editing or adding in operate mode
		m_toolsSettingsAction->setEnabled(false);
		m_editActionGroup->setEnabled(false);
		m_addActionGroup->setEnabled(false);
		m_bgActionGroup->setEnabled(false);
		m_fgActionGroup->setEnabled(false);
		m_fontActionGroup->setEnabled(false);
		m_frameActionGroup->setEnabled(false);
		m_stackingActionGroup->setEnabled(false);
	}
	else
	{
		// Allow editing and adding in design mode
		m_toolsSettingsAction->setEnabled(true);
		m_editActionGroup->setEnabled(true);
		m_addActionGroup->setEnabled(true);
		m_bgActionGroup->setEnabled(true);
		m_fgActionGroup->setEnabled(true);
		m_fontActionGroup->setEnabled(true);
		m_frameActionGroup->setEnabled(true);
		m_stackingActionGroup->setEnabled(true);
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
