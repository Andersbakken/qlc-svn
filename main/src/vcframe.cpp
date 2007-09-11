/*
  Q Light Controller
  vcframe.cpp

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

#include "vcframe.h"
#include "vcbutton.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "vcframeproperties.h"
#include "common/filehandler.h"
#include "common/qlcimagepreview.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qbuttongroup.h>
#include <qobjectlist.h>
#include <assert.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qinputdialog.h>

extern App* _app;

VCFrame::VCFrame(QWidget* parent)
	: QFrame(parent, "VCFrame"),
	  m_backgroundImage  ( QString::null ),
	  m_hasCustomBackgroundColor ( false ),
	  m_hasCustomForegroundColor ( false ),
	  m_hasCustomFont ( false ),
	  m_xpos ( 0 ),
	  m_ypos ( 0 ),
	  m_resizeMode ( false ),
	  m_buttonBehaviour ( Normal )
{
}

VCFrame::~VCFrame()
{
}

void VCFrame::init(bool bottomFrame)
{
	if (bottomFrame == false)
	{
		setFrameStyle(KFrameStyleSunken);
		setMinimumSize(20, 20);
		resize(QPoint(120, 120));
	}

	/* Listen to mode changes (operate/design) so menus can be en/disabled */
	disconnect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

void VCFrame::scram()
{
	int result = QMessageBox::warning(this,
					  QString(caption()),
					  QString("Remove the selected frame " \
						  "along with all of its widgets?"),
					  QMessageBox::Yes,
					  QMessageBox::No);

	if (result == QMessageBox::Yes)
	{
		_app->virtualConsole()->setSelectedWidget(NULL);
		_app->doc()->setModified();
		deleteLater();
	}
}

bool VCFrame::isBottomFrame()
{
	if (parentWidget() != NULL && parentWidget()->className() != "VCFrame")
		return true;
	else
		return false;
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCFrame::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCFrame::backgroundImage()
{
	return m_backgroundImage;
}

void VCFrame::chooseBackgroundImage()
{
	QLCImagePreview* preview = new QLCImagePreview();
	QFileDialog* fd = new QFileDialog(this);
	fd->setContentsPreviewEnabled(true);
	fd->setContentsPreview(preview, preview);
	fd->setPreviewMode(QFileDialog::Contents);
	fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
	fd->setSelection(backgroundImage());
	
	if (fd->exec() == QDialog::Accepted)
		setBackgroundImage(fd->selectedFile());
	
	delete preview;
	delete fd;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCFrame::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCFrame::resetBackgroundColor()
{
	QColor fg;

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = QString::null;

	/* Store foreground color */
	if (m_hasCustomForegroundColor == true)
		fg = paletteForegroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (fg.isValid() == true)
		setPaletteForegroundColor(fg);

	_app->doc()->setModified();
}

void VCFrame::chooseBackgroundColor()
{
	QColor color;
	color = QColorDialog::getColor(backgroundColor());
	if (color.isValid())
		setBackgroundColor(color);
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCFrame::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCFrame::resetForegroundColor()
{
	QColor bg;

	m_hasCustomForegroundColor = false;

	/* Store background color */
	if (m_hasCustomBackgroundColor == true)
		bg = paletteBackgroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (bg.isValid() == true)
		setPaletteBackgroundColor(bg);
	else if (m_backgroundImage.isEmpty() == false)
		setPaletteBackgroundPixmap(QPixmap(m_backgroundImage));

	_app->doc()->setModified();
}

void VCFrame::chooseForegroundColor()
{
	QColor color;
	color = QColorDialog::getColor(foregroundColor());
	if (color.isValid())
		setForegroundColor(color);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCFrame::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCFrame::resetFont()
{
	m_hasCustomFont = false;
	unsetFont();
	_app->doc()->setModified();
}

void VCFrame::chooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCFrame::setCaption(const QString& text)
{
	QFrame::setCaption(text);
	_app->doc()->setModified();
}

void VCFrame::rename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText("Rename frame",
				     "Set frame caption:", QLineEdit::Normal,
				     QString::null, &ok, this );
	if (ok == true && text.isEmpty() == false)
		setCaption(text);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/
void VCFrame::editProperties()
{
	VCFrameProperties* prop = new VCFrameProperties(this);
	prop->init();
	if (prop->exec() == QDialog::Accepted)
		_app->doc()->setModified();

	delete prop;
}

/*****************************************************************************
 * Button Behaviour
 *****************************************************************************/

void VCFrame::setButtonBehaviour(ButtonBehaviour b)
{
	m_buttonBehaviour = b;

	if (buttonBehaviour() == VCFrame::Exclusive)
	{
		if (_app->virtualConsole()->selectedWidget())
		{
			QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
			QObjectListIt it(*l);
			QObject *obj;
			while ((obj = it.current()) != 0)
			{
				++it;
				((VCButton*)obj)->setExclusive(true);
			}

			delete l;
			setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
			setLineWidth(2);
		}
		else
		{
			setFrameStyle(QFrame::GroupBoxPanel | QFrame::Sunken);
			setLineWidth(2);
		}
	}
	else
	{
		if (_app->virtualConsole()->selectedWidget())
		{
			QObjectList* l = _app->virtualConsole()->selectedWidget()->queryList("VCButton");
			QObjectListIt it(*l);
			QObject *obj;
			while ((obj = it.current()) != 0)
			{
				++it;
				((VCButton*)obj)->setExclusive(false);
			}

			delete l;
			setFrameStyle(KFrameStyleSunken);
			setLineWidth(1);
		}
		else
		{
			setFrameStyle(KFrameStyleSunken);
			setLineWidth(1);
		}
	}
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCFrame::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCFrame* frame = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCFrame)
	{
		qWarning("Frame node not found!");
		return false;
	}

	/* Create a new frame into its parent */
	frame = new VCFrame(parent);
	frame->show();

	/* If the current parent widget is anything else than VCFrame,
	   the currently loaded VCFrame is the parent of all VC widgets */
	if (parent->className() != "VCFrame")
		_app->virtualConsole()->setDrawArea(frame);
	else
		frame->init();

	/* Continue loading */
	return frame->loadXML(doc, root);
}

bool VCFrame::loadXML(QDomDocument* doc, QDomElement* root)
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

	if (root->tagName() != KXMLQLCVCFrame)
	{
		qWarning("Frame node not found!");
		return false;
	}

	/* Caption */
	setCaption(root->attribute(KXMLQLCVCCaption));

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
			setGeometry(x, y, w, h);
		}
		else if (tag.tagName() == KXMLQLCVCAppearance)
		{
			loadXMLAppearance(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCFrame)
		{
			VCFrame::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCLabel)
		{
			VCLabel::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCButton)
		{
			VCButton::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCXYPad)
		{
			VCXYPad::loader(doc, &tag, this);
		}
		else if (tag.tagName() == KXMLQLCVCDockSlider)
		{
			VCDockSlider::loader(doc, &tag, this);
		}
		else
		{
			qWarning("Unknown frame tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCFrame::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	const QObjectList* objectList = NULL;
	QObject* child = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Frame entry */
	root = doc->createElement(KXMLQLCVCFrame);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Button Behaviour */
	str.setNum(buttonBehaviour());
	root.setAttribute(KXMLQLCVCFrameButtonBehaviour, str);

	/* Save appearance */
	saveXMLAppearance(doc, &root);

	/* Save widget proportions only for child frames */
	if (isBottomFrame() == false)
		FileHandler::saveXMLWindowState(doc, &root, this);

	/* Save children */
	objectList = children();
	if (objectList != NULL)
	{
		QObjectListIterator it(*objectList);
		while ( (child = it.current()) != NULL )
		{
			if (child->className() == "VCFrame")
			{
				static_cast<VCFrame*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCButton")
			{
				static_cast<VCButton*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCDockSlider")
			{
				static_cast<VCDockSlider*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCLabel")
			{
				static_cast<VCLabel*> (child)->saveXML(doc, &root);
			}
			else if (child->className() == "VCXYPad")
			{
				static_cast<VCXYPad*> (child)->saveXML(doc, &root);
			}
			else
			{
				qWarning("Unknown VC widget class: %s",
					 child->className());
			}
			
			++it;
		}
	}

	return true;
}

bool VCFrame::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCAppearance)
	{
		qWarning("Appearance node not found!");
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCFrameStyle)
		{
			int style = 0;
			style = VirtualConsole::stringToFrameStyle(tag.text());
			setFrameStyle(style);
		}
		else if (tag.tagName() == KXMLQLCVCForegroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
			{
				QColor color(tag.text().toInt());
				setForegroundColor(color);
			}
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
				setBackgroundColor(QColor(tag.text().toInt()));
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundImage)
		{
			if (tag.text() != KXMLQLCVCBackgroundImageNone)
				setBackgroundImage(tag.text());
		}
		else if (tag.tagName() == KXMLQLCVCFont)
		{
			if (tag.text() != KXMLQLCVCFontDefault)
			{
				QFont font;
				font.fromString(tag.text());
				setFont(font);
			}
		}
		else
		{
			qWarning("Unknown Appearance tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}
}

bool VCFrame::saveXMLAppearance(QDomDocument* doc, QDomElement* frame_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(frame_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	frame_root->appendChild(root);

	/* Frame style */
	tag = doc->createElement(KXMLQLCVCFrameStyle);
	root.appendChild(tag);
	text = doc->createTextNode(VirtualConsole::frameStyleToString(frameStyle()));
	tag.appendChild(text);

	/* Foreground color */
	tag = doc->createElement(KXMLQLCVCForegroundColor);
	root.appendChild(tag);
	if (hasCustomForegroundColor() == true)
		str.setNum(paletteForegroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background color */
	tag = doc->createElement(KXMLQLCVCBackgroundColor);
	root.appendChild(tag);
	if (hasCustomBackgroundColor() == true)
		str.setNum(paletteBackgroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background image */
	tag = doc->createElement(KXMLQLCVCBackgroundImage);
	root.appendChild(tag);
	if (backgroundImage() != QString::null)
		str = m_backgroundImage;
	else
		str = KXMLQLCVCBackgroundImageNone;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Font */
	tag = doc->createElement(KXMLQLCVCFont);
	root.appendChild(tag);
	if (hasCustomFont() == true)
		str = font().toString();
	else
		str = KXMLQLCVCFontDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);	

	return true;
}

/*****************************************************************************
 * QLC Mode change
 *****************************************************************************/
void VCFrame::slotModeChanged()
{
	repaint();
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCFrame::invokeMenu(QPoint point)
{
	// Add menu
	QPopupMenu* addMenu = new QPopupMenu();
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/button.png")),
			    "&Button", KVCMenuAddButton);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/slider.png")),
			    "&Slider", KVCMenuAddSlider);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frame.png")),
			    "&Frame", KVCMenuAddFrame);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/xypad.png")),
			    "&XY-Pad", KVCMenuAddXYPad);
	addMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/label.png")),
			    "L&abel", KVCMenuAddLabel);

	// Foreground menu
	QPopupMenu* fgMenu = new QPopupMenu();
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuForegroundColor);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuForegroundDefault);

	// Background menu
	QPopupMenu* bgMenu = new QPopupMenu();
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuBackgroundColor);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			   "&Image...", KVCMenuBackgroundImage);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuBackgroundDefault);

	// Font menu
	QPopupMenu* fontMenu = new QPopupMenu();
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
			     "&Font...", KVCMenuFont);
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuFontDefault);

	// Frame menu
	QPopupMenu* frameMenu = new QPopupMenu();
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framesunken.png")),
			      "&Sunken", KVCMenuFrameSunken);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frameraised.png")),
			      "&Raised", KVCMenuFrameRaised);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framenone.png")),
			      "&None", KVCMenuFrameNone);

	// Stacking order menu
	QPopupMenu* stackMenu = new QPopupMenu();
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/up.png")),
			      "Bring to &Front", KVCMenuStackingRaise);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/down.png")),
			      "Send to &Back", KVCMenuStackingLower);

	// Edit menu
	QPopupMenu* editMenu = new QPopupMenu();
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcut.png")),
			     "Cut", KVCMenuEditCut);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			     "Copy", KVCMenuEditCopy);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editpaste.png")),
			     "Paste", KVCMenuEditPaste);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			     "Delete", KVCMenuEditDelete);

	editMenu->insertSeparator();

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			       "&Properties...", KVCMenuEditProperties);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editclear.png")),
			       "&Rename...", KVCMenuEditRename);

	editMenu->setItemEnabled(KVCMenuEditCut, false);
	editMenu->setItemEnabled(KVCMenuEditCopy, false);
	editMenu->setItemEnabled(KVCMenuEditPaste, false);

	if (isBottomFrame() == true)
		editMenu->setItemEnabled(KVCMenuEditDelete, false);

	editMenu->insertSeparator();

	editMenu->insertItem("Add", addMenu);
	editMenu->insertItem("Background", bgMenu);
	editMenu->insertItem("Foreground", fgMenu);
	editMenu->insertItem("Font", fontMenu);
	editMenu->insertItem("Frame", frameMenu);
	editMenu->insertItem("Stacking Order", stackMenu);

	connect(editMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(addMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(bgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fontMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(frameMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(stackMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	
	editMenu->exec(point);

	delete editMenu;
	delete addMenu;
	delete bgMenu;
	delete fgMenu;
	delete fontMenu;
	delete frameMenu;
	delete stackMenu;
}

void VCFrame::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuAddButton:
		addButton(m_mousePressPoint);
		break;

	case KVCMenuAddSlider:
		addSlider(m_mousePressPoint);
		break;

	case KVCMenuAddFrame:
		addFrame(m_mousePressPoint);
		break;

	case KVCMenuAddXYPad:
		addXYPad(m_mousePressPoint);
		break;

	case KVCMenuAddLabel:
		addLabel(m_mousePressPoint);
		break;

	case KVCMenuEditCut:
		break;
	case KVCMenuEditCopy:
		break;
	case KVCMenuEditPaste:
		break;
	case KVCMenuEditDelete:
		scram();
		break;

	case KVCMenuEditProperties:
		editProperties();
		break;

	case KVCMenuEditRename:
		rename();
		break;

	case KVCMenuForegroundColor:
		chooseForegroundColor();
		break;

	case KVCMenuForegroundDefault:
		resetForegroundColor();
		break;

	case KVCMenuBackgroundColor:
		chooseBackgroundColor();
		break;
	
	case KVCMenuBackgroundImage:
		chooseBackgroundImage();
		break;
	
	case KVCMenuBackgroundDefault:
		resetBackgroundColor();
		break;

	case KVCMenuFont:
		chooseFont();
		break;

	case KVCMenuFontDefault:
		resetFont();
		break;

	case KVCMenuFrameSunken:
		setFrameStyle(KFrameStyleSunken);
		break;

	case KVCMenuFrameRaised:
		setFrameStyle(KFrameStyleRaised);
		break;

	case KVCMenuFrameNone:
		setFrameStyle(KFrameStyleNone);
		break;

	case KVCMenuStackingRaise:
		raise();
		break;

	case KVCMenuStackingLower:
		lower();
		break;

	default:
		break;
	}
}

void VCFrame::addButton(QPoint at)
{
	VCButton* button = new VCButton(this);
	Q_ASSERT(button != NULL);
	button->init();
	button->show();

	if (this->buttonBehaviour() == VCFrame::Exclusive)
		button->setExclusive(true);
	else
		button->setExclusive(false);

	if (at.isNull() == false)
		button->move(at);
	else
		button->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addSlider(QPoint at)
{
	VCDockSlider* slider = new VCDockSlider(this);
	Q_ASSERT(slider != NULL);
	slider->setBusID(KBusIDDefaultFade);
	slider->init();
	slider->resize(QPoint(55, 200));
	slider->show();

	if (at.isNull() == false)
		slider->move(at);
	else
		slider->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addFrame(QPoint at)
{
	VCFrame* frame = new VCFrame(this);
	Q_ASSERT(frame != NULL);
	frame->init();
	frame->show();

	if (at.isNull() == false)
		frame->move(at);
	else
		frame->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addXYPad(QPoint at)
{
	VCXYPad* xypad = new VCXYPad(this);
	Q_ASSERT(xypad != NULL);
	xypad->init();
	xypad->show();

	if (at.isNull() == false)
		xypad->move(at);
	else
		xypad->move(m_mousePressPoint);

	_app->doc()->setModified();
}

void VCFrame::addLabel(QPoint at)
{
	VCLabel* label = new VCLabel(this);
	Q_ASSERT(label != NULL);
	label->init();
	label->show();

	if (at.isNull() == false)
		label->move(at);
	else
		label->move(m_mousePressPoint);

	_app->doc()->setModified();
}

/*****************************************************************************
 * Widget move & resize
 *****************************************************************************/

void VCFrame::resize(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Map to parent coordinates so that they can be compared
	p = mapToParent(p);

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() > parentWidget()->width())
	{
		p.setX(parentWidget()->width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() > parentWidget()->height())
	{
		p.setY(parentWidget()->height());
	}

	// Map back so that this can be resized
	p = mapFromParent(p);

	// Do the resize
	QFrame::resize(p.x(), p.y());
}


void VCFrame::move(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() + rect().width() > parentWidget()->width())
	{
		p.setX(parentWidget()->width() - rect().width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() + rect().height() > parentWidget()->height())
	{
		p.setY(parentWidget()->height() - rect().height());
	}

	// Do the move
	QFrame::move(p);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCFrame::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);

	QPainter p(this);

	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		// Draw a dotted line around the widget
		QPen pen(DotLine);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
	}
}

void VCFrame::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (m_resizeMode == true && isBottomFrame() == false)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if ((e->button() & LeftButton || e->button() & MidButton)
		    && isBottomFrame() == false)
		{
			if (e->x() > rect().width() - 10 &&
			    e->y() > rect().height() - 10)
			{
				m_resizeMode = true;
				setMouseTracking(true);
				setCursor(QCursor(SizeFDiagCursor));
			}
			else
			{
				m_mousePressPoint = QPoint(e->x(), e->y());
				setCursor(QCursor(SizeAllCursor));
			}
		}
		else if (e->button() & RightButton)
		{
			m_mousePressPoint = QPoint(e->x(), e->y());
			invokeMenu(mapToGlobal(e->pos()));
		}
	}
	else
	{
		QFrame::mousePressEvent(e);
	}
}

void VCFrame::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}
	else
	{
		QFrame::mouseReleaseEvent(e);
	}
}

void VCFrame::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		slotMenuCallback(KVCMenuEditProperties);
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_resizeMode == true)
		{
			QPoint p(QCursor::pos());
			resize(mapFromGlobal(p));
			_app->doc()->setModified();
		}
		else if (e->state() & LeftButton || e->state() & MidButton)
		{
			QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
			p.setX(p.x() - m_mousePressPoint.x());
			p.setY(p.y() - m_mousePressPoint.y());

			move(p);
			_app->doc()->setModified();
		}
	}
	else
	{
		QFrame::mouseMoveEvent(e);
	}
}
