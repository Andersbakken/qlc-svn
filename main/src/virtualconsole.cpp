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

#include <qmenubar.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qslider.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qcursor.h>
#include <assert.h>
#include <qobjectlist.h>
#include <qdom.h>

#include "common/inputplugin.h"
#include "common/qlcimagepreview.h"
#include "common/filehandler.h"

#include "virtualconsole.h"
#include "virtualconsoleproperties.h"
#include "app.h"
#include "doc.h"
#include "keybind.h"
#include "vcframe.h"
#include "vcbutton.h"
#include "vcdockarea.h"
#include "vcdockslider.h"

#include <X11/Xlib.h>

extern App* _app;
extern QApplication* _qapp;

VirtualConsole::VirtualConsole(QWidget* parent) 
	: QWidget(parent, "Virtual Console")
{
	m_dockArea = NULL;
	m_drawArea = NULL;

	m_gridEnabled = true;
	m_gridX = 10;
	m_gridY = 10;

	m_keyRepeatOff = true;
	m_grabKeyboard = true;

	m_selectedWidget = NULL;
	m_editMenu = NULL;
}

VirtualConsole::~VirtualConsole()
{
}


//
// Init the whole view
//
void VirtualConsole::initView(void)
{
	setCaption("Virtual Console");
	resize(300, 400);

	// Init top menu bar
	initMenuBar();

	// Init left dock area
	initDockArea();

	// Init right drawing area
	setDrawArea(new VCFrame(this));

	// Update this according to current mode
	slotModeChanged(_app->mode());

	// Connect to catch mode change events
	connect(_app, SIGNAL(modeChanged(App::Mode)),
		this, SLOT(slotModeChanged(App::Mode)));
}

void VirtualConsole::initMenuBar()
{
	setIcon(QString(PIXMAPS) + QString("/virtualconsole.png"));

	m_layout = new QHBoxLayout(this);
	m_layout->setAutoAdd(false);

	m_menuBar = new QMenuBar(this);
	m_layout->setMenuBar(m_menuBar);

	//
	// Add menu
	//
	m_addMenu = new QPopupMenu();
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/button.png")),
			      "&Button", this, SLOT(slotAddButton()),
			      0, KVCMenuAddButton);
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/slider.png")),
			      "&Slider", this, SLOT(slotAddSlider()),
			      0, KVCMenuAddSlider);
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frame.png")),
			      "&Frame", this, SLOT(slotAddFrame()),
			      0, KVCMenuAddFrame);
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/xypad.png")),
			      "&XY-Pad", this, SLOT(slotAddXYPad()),
			      0, KVCMenuAddXYPad);
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/label.png")),
			      "L&abel", this, SLOT(slotAddLabel()),
			      0, KVCMenuAddLabel);

	//
	// Tools menu
	//
	m_toolsMenu = new QPopupMenu();
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
				"&Settings...", this, SLOT(slotToolsSettings()),
				0, KVCMenuToolsSettings);
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/slider.png")),
				"&Default Sliders", this, SLOT(slotToolsSliders()),
				0, KVCMenuToolsSliders);
	m_toolsMenu->insertSeparator();
	m_toolsMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/panic.png")),
				"&Panic!", this, SLOT(slotToolsPanic()),
				0, KVCMenuToolsPanic);


	//
	// Foreground menu
	//
	QPopupMenu* fgMenu = new QPopupMenu();
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", this, SLOT(slotForegroundColor()),
			   0, KVCMenuForegroundColor);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
			   "&Font...", this, SLOT(slotForegroundFont()),
			   0, KVCMenuForegroundFont);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", this, SLOT(slotForegroundNone()),
			   0, KVCMenuForegroundNone);

	//
	// Background Menu
	//
	QPopupMenu* bgMenu = new QPopupMenu();
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", this, SLOT(slotBackgroundColor()),
			   0, KVCMenuBackgroundColor);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			   "&Image...", this, SLOT(slotBackgroundImage()),
			   0, KVCMenuBackgroundPixmap);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", this, SLOT(slotBackgroundNone()),
			   0, KVCMenuBackgroundNone);
	bgMenu->insertSeparator();
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frame.png")),
			   "Toggle &Frame", this, SLOT(slotBackgroundFrame()),
			   0, KVCMenuBackgroundFrame);

	//
	// Stacking order menu
	//
	QPopupMenu* stackMenu = new QPopupMenu();
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/up.png")),
			      "Bring to &Front", this, SLOT(slotStackingRaise()),
			      0, KVCMenuStackingRaise);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/down.png")),
			      "Send to &Back", this, SLOT(slotStackingLower()),
			      0, KVCMenuStackingLower);

	//
	// Edit menu
	//
	m_editMenu = new QPopupMenu();
	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcut.png")),
			       "Cut", this, SLOT(slotEditCut()),
			       0, KVCMenuEditCut);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			       "Copy", this, SLOT(slotEditCopy()),
			       0, KVCMenuEditCopy);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editpaste.png")),
			       "Paste", this, SLOT(slotEditPaste()),
			       0, KVCMenuEditPaste);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			       "Delete", this, SLOT(slotEditDelete()),
			       0, KVCMenuEditDelete);

	m_editMenu->insertSeparator();

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			       "&Properties...", this, SLOT(slotEditProperties()),
			       0, KVCMenuEditProperties);
	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editclear.png")),
			       "&Rename...", this, SLOT(slotEditRename()),
			       0, KVCMenuEditRename);

	m_editMenu->setItemEnabled(KVCMenuEditCut, false);
	m_editMenu->setItemEnabled(KVCMenuEditCopy, false);
	m_editMenu->setItemEnabled(KVCMenuEditPaste, false);

	m_editMenu->insertSeparator();

	m_editMenu->insertItem("Foreground", fgMenu, KVCMenuForegroundMin);
	m_editMenu->insertItem("Background", bgMenu, KVCMenuBackgroundMin);
	m_editMenu->insertItem("Stacking Order", stackMenu, KVCMenuStackingMin);

	m_menuBar->insertItem("&Tools", m_toolsMenu);
	m_menuBar->insertItem("&Add", m_addMenu);
	m_menuBar->insertItem("&Edit", m_editMenu);
}

void VirtualConsole::initDockArea()
{
	if (m_dockArea != NULL)
		delete m_dockArea;

	m_dockArea = new VCDockArea(this);
	connect(m_dockArea, SIGNAL(visibilityChanged(bool)),
		this, SLOT(slotDockAreaVisibilityChanged(bool)));
	m_dockArea->init();

	// Add the dock area into the master horizontal layout
	m_layout->addWidget(m_dockArea, 0);
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VirtualConsole::loader(QDomDocument* doc, QDomElement* vc_root)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* TODO: Make sure that VC is really created anew */

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
		qWarning("Virtual Console node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
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
			qDebug("Unknown virtual console tag: %s",
			       (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

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
	FileHandler::saveXMLWindowState(doc, &root, this);

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

void VirtualConsole::setSelectedWidget(QWidget* w)
{
	if (m_selectedWidget != NULL)
	{
		QWidget* old = m_selectedWidget;
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
}

/*****************************************************************************
 * Draw area
 *****************************************************************************/

void VirtualConsole::setDrawArea(VCFrame* drawArea)
{
	Q_ASSERT(drawArea != NULL);

	if (m_drawArea != NULL)
		delete m_drawArea;
	m_drawArea = drawArea;

	/* Add the draw area into the master horizontal layout */
	m_layout->addWidget(m_drawArea, 1);
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

void VirtualConsole::slotAddButton()
{
	m_drawArea->addButton();
}

void VirtualConsole::slotAddSlider()
{
	m_drawArea->addSlider();
}

void VirtualConsole::slotAddFrame()
{
	m_drawArea->addFrame();
}

void VirtualConsole::slotAddXYPad()
{
	m_drawArea->addXYPad();
}

void VirtualConsole::slotAddLabel()
{
	m_drawArea->addLabel();
}

/*********************************************************************
 * Tools menu callbacks
 *********************************************************************/

void VirtualConsole::slotToolsSettings()
{
	VirtualConsoleProperties prop(this);
	t_bus_value lo = 0;
	t_bus_value hi = 0;

	Q_ASSERT(m_dockArea != NULL);

	prop.setGridEnabled(m_gridEnabled);
	prop.setGridX(m_gridX);
	prop.setGridY(m_gridY);
	prop.setKeyRepeatOff(m_keyRepeatOff);
	prop.setGrabKeyboard(m_grabKeyboard);
	m_dockArea->defaultFadeSlider()->busRange(lo, hi);
	prop.setFadeLimits(lo, hi);
	m_dockArea->defaultHoldSlider()->busRange(lo, hi);
	prop.setHoldLimits(lo, hi);

	prop.init();
	if (prop.exec() == QDialog::Accepted)
	{
		setGridEnabled(prop.isGridEnabled());
		setGridX(prop.gridX());
		setGridY(prop.gridY());
		
		setKeyRepeatOff(prop.isKeyRepeatOff());
		setGrabKeyboard(prop.isGrabKeyboard());

		prop.fadeLimits(lo, hi);
		m_dockArea->defaultFadeSlider()->setBusRange(lo, hi);

		prop.holdLimits(lo, hi);
		m_dockArea->defaultHoldSlider()->setBusRange(lo, hi);
	}
}

void VirtualConsole::slotToolsSliders()
{
	if (m_dockArea->isHidden())
	{
		m_dockArea->show();
	}
	else
	{
		m_dockArea->hide();
	}

	_app->doc()->setModified();
}

void VirtualConsole::slotToolsPanic()
{
	// Panic button pressed: stop all running functions
	_app->slotPanic();
}

/*********************************************************************
 * Edit menu callbacks
 *********************************************************************/

void VirtualConsole::slotEditCut()
{
	qDebug("Not implemented");
}

void VirtualConsole::slotEditCopy()
{
	qDebug("Not implemented");
}

void VirtualConsole::slotEditPaste()
{
	qDebug("Not implemented");
}

void VirtualConsole::slotEditDelete()
{
	if (m_selectedWidget != NULL)
	{
		if (m_selectedWidget->inherits("VCWidget") == true)
		{
			static_cast<VCWidget*> 
				(m_selectedWidget)->scram();
		}
		else if (m_selectedWidget->isA("VCButton") == true)
		{
			static_cast<VCButton*> 
				(m_selectedWidget)->scram();
		}
	}
}

void VirtualConsole::slotEditProperties()
{
	if (m_selectedWidget != NULL)
	{
		if (m_selectedWidget->inherits("VCWidget") == true)
		{
			static_cast<VCWidget*> 
				(m_selectedWidget)->editProperties();
		}
		else if (m_selectedWidget->isA("VCButton") == true)
		{
			static_cast<VCButton*> 
				(m_selectedWidget)->editProperties();
		}
	}
}

void VirtualConsole::slotEditRename()
{
	if (m_selectedWidget != NULL)
	{
		if (m_selectedWidget->inherits("VCWidget") == true)
		{
			static_cast<VCWidget*> 
				(m_selectedWidget)->rename();
		}
		else if (m_selectedWidget->isA("VCButton") == true)
		{
			static_cast<VCButton*> 
				(m_selectedWidget)->rename();
		}
	}
}

/*********************************************************************
 * Foreground menu callbacks
 *********************************************************************/

void VirtualConsole::slotForegroundFont()
{
	if (m_selectedWidget)
	{
		m_selectedWidget->setFont(QFontDialog::
					  getFont(0, m_selectedWidget->font()));
		_app->doc()->setModified();
	}
}

void VirtualConsole::slotForegroundColor()
{
	if (m_selectedWidget)
	{
		QColor color =
			QColorDialog::getColor(m_selectedWidget->paletteForegroundColor(),
					       this);

		if (color.isValid())
		{
			_app->doc()->setModified();
			m_selectedWidget->setPaletteForegroundColor(color);
		}

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotForegroundNone()
{
	if (m_selectedWidget)
	{
		// Save the background color (note that we are resetting
		// the foreground here!)
		QColor bgc = m_selectedWidget->paletteBackgroundColor();

		// Reset palette and font
		m_selectedWidget->unsetPalette();
		m_selectedWidget->unsetFont();

		// Now set the backround color again because we only reset
		// foreground options.
		m_selectedWidget->setPaletteBackgroundColor(bgc);

		_app->doc()->setModified();
	}
}

/*********************************************************************
 * Background menu callbacks
 *********************************************************************/

void VirtualConsole::slotBackgroundColor()
{
	if (m_selectedWidget)
	{
		QColor newcolor = QColorDialog::
			getColor(m_selectedWidget->paletteBackgroundColor(), this);

		if (newcolor.isValid() == true)
		{
			m_selectedWidget->setPaletteBackgroundColor(newcolor);
			_app->doc()->setModified();
		}
	}
}

void VirtualConsole::slotBackgroundImage()
{
	if (m_selectedWidget)
	{
		QString fileName;

		QLCImagePreview* preview = new QLCImagePreview();

		QFileDialog* fd = new QFileDialog(this);
		fd->setContentsPreviewEnabled(true);
		fd->setContentsPreview(preview, preview);
		fd->setPreviewMode(QFileDialog::Contents);
		fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
		fd->setSelection(m_selectedWidget->iconText());

		if (fd->exec() == QDialog::Accepted)
		{
			fileName = fd->selectedFile();
		}

		if (fileName.isEmpty() == false)
		{
			// Set the selected pixmap as bg image and icon.
			// Some QT styles don't know how to display bg pixmaps, hence icon.
			QPixmap pm(fileName);
			m_selectedWidget->setPaletteBackgroundPixmap(pm);
			m_selectedWidget->setIconText(fileName);
			_app->doc()->setModified();
		}

		delete preview;
		delete fd;
	}
}

void VirtualConsole::slotBackgroundNone()
{
	if (m_selectedWidget)
	{
		// Save the foreground color (note that we are resetting
		// the background here!)
		QColor fgc(m_selectedWidget->paletteForegroundColor());

		// Reset palette
		m_selectedWidget->unsetPalette();

		// Now set the foreground color again
		m_selectedWidget->setPaletteForegroundColor(fgc);

		_app->doc()->setModified();
	}
}

void VirtualConsole::slotBackgroundFrame()
{
	if (m_selectedWidget)
	{
		QApplication::sendEvent(m_selectedWidget,
					new VCMenuEvent(KVCMenuBackgroundFrame));
	}
}

/*********************************************************************
 * Stacking menu callbacks
 *********************************************************************/

void VirtualConsole::slotStackingRaise()
{
	if (m_selectedWidget)
	{
		m_selectedWidget->raise();
		_app->doc()->setModified();
	}
}

void VirtualConsole::slotStackingLower()
{
	if (m_selectedWidget)
	{
		m_selectedWidget->lower();
		_app->doc()->setModified();
	}
}

/*********************************************************************
 * Misc callbacks
 *********************************************************************/

void VirtualConsole::slotDockAreaVisibilityChanged(bool isVisible)
{
	m_menuBar->setItemChecked(KVCMenuToolsSliders, isVisible);
}


void VirtualConsole::slotModeChanged(App::Mode mode)
{
	QString config;

	/* Key repeat */
	if (isKeyRepeatOff() == true)
	{
		Display* display;
		display = XOpenDisplay(NULL);
		ASSERT(display != NULL);

		if (mode == App::Design)
			XAutoRepeatOn(display);
		else
			XAutoRepeatOff(display);
		
		XCloseDisplay(display);
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
		// Don't allow editing in operate mode
		m_editMenu->setEnabled(false);
		m_addMenu->setEnabled(false);
		m_toolsMenu->setItemEnabled(KVCMenuToolsSettings, false);
	}
	else
	{
		// Allow editing in design mode
		m_editMenu->setEnabled(true);
		m_addMenu->setEnabled(true);
		m_toolsMenu->setItemEnabled(KVCMenuToolsSettings, true);
	}
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

void VirtualConsole::customEvent(QCustomEvent* e)
{
	// There is something the InputPlugin wants to tell
	if ((e->type() == KInputEvent) && (_app->mode() != App::Design))
	{
		InputEvent* ie = (InputEvent*)e;
		emit InpEvent(ie->id(), ie->channel(), ie->value());
	}
}
