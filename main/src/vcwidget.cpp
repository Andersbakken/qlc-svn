/*
  Q Light Controller
  vcwidget.cpp

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
#include <math.h>

#include "common/filehandler.h"
#include "common/qlcimagepreview.h"

#include "vcwidget.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"

extern App* _app;

VCWidget::VCWidget(QWidget* parent, const char* name) : QFrame(parent, name)
{
	m_backgroundImage = QString::null;
	m_hasCustomBackgroundColor = false;
	m_hasCustomForegroundColor = false;
	m_hasCustomFont = false;

	m_xpos = 0;
	m_ypos = 0;
	m_resizeMode = false;

	connect(_app, SIGNAL(modeChanged(App::Mode)), 
		this, SLOT(slotModeChanged(App::Mode)));

	setCaption("Widget");
	setFrameStyle(KVCWidgetFrameStyleSunken);
	setMinimumSize(20, 20);
	resize(QPoint(120, 120));
}

VCWidget::~VCWidget()
{
}

void VCWidget::scram()
{
	QString msg;

	msg = "Do you wish to delete this widget?\n" + caption();
	int result = QMessageBox::question(this, "Delete", msg,
					   QMessageBox::Yes,
					   QMessageBox::No);
	
	if (result == QMessageBox::Yes)
	{
		_app->virtualConsole()->setSelectedWidget(NULL);
		_app->doc()->setModified();
		deleteLater();
	}
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCWidget::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCWidget::backgroundImage()
{
	return m_backgroundImage;
}

void VCWidget::chooseBackgroundImage()
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
void VCWidget::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCWidget::resetBackgroundColor()
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

void VCWidget::chooseBackgroundColor()
{
	QColor color;
	color = QColorDialog::getColor(backgroundColor());
	if (color.isValid())
		setBackgroundColor(color);
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCWidget::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCWidget::resetForegroundColor()
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

void VCWidget::chooseForegroundColor()
{
	QColor color;
	color = QColorDialog::getColor(foregroundColor());
	if (color.isValid())
		setForegroundColor(color);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCWidget::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCWidget::resetFont()
{
	m_hasCustomFont = false;
	unsetFont();
	_app->doc()->setModified();
}

void VCWidget::chooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCWidget::setCaption(const QString& text)
{
	QFrame::setCaption(text);
	repaint();
	_app->doc()->setModified();
}

void VCWidget::rename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText("Rename widget",
				     "Set widget caption:",
				     QLineEdit::Normal,
				     caption(),
				     &ok,
				     this);

	if (ok == true)
		setCaption(text);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCWidget::editProperties()
{
	QMessageBox::information(_app, className(),
				 "This widget has no properties");
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCWidget::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
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
		if (tag.tagName() == KXMLQLCVCWidgetFrameStyle)
		{
			int style = 0;
			style = stringToFrameStyle(tag.text());
			setFrameStyle(style);
		}
		else if (tag.tagName() == KXMLQLCVCWidgetForegroundColor)
		{
			if (tag.text() != KXMLQLCVCWidgetColorDefault)
			{
				QColor color(tag.text().toInt());
				setForegroundColor(color);
			}
		}
		else if (tag.tagName() == KXMLQLCVCWidgetBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCWidgetColorDefault)
				setBackgroundColor(QColor(tag.text().toInt()));
		}
		else if (tag.tagName() == KXMLQLCVCWidgetBackgroundImage)
		{
			if (tag.text() != KXMLQLCVCWidgetBackgroundImageNone)
				setBackgroundImage(tag.text());
		}
		else if (tag.tagName() == KXMLQLCVCWidgetFont)
		{
			if (tag.text() != KXMLQLCVCWidgetFontDefault)
			{
				QFont font;
				font.fromString(tag.text());
				setFont(font);
			}
		}
		else
		{
			qWarning("Unknown appearance tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}
}

bool VCWidget::saveXMLAppearance(QDomDocument* doc, QDomElement* frame_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(frame_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCWidgetAppearance);
	frame_root->appendChild(root);

	/* Frame style */
	tag = doc->createElement(KXMLQLCVCWidgetFrameStyle);
	root.appendChild(tag);
	text = doc->createTextNode(frameStyleToString(frameStyle()));
	tag.appendChild(text);

	/* Foreground color */
	tag = doc->createElement(KXMLQLCVCWidgetForegroundColor);
	root.appendChild(tag);
	if (hasCustomForegroundColor() == true)
		str.setNum(paletteForegroundColor().rgb());
	else
		str = KXMLQLCVCWidgetColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background color */
	tag = doc->createElement(KXMLQLCVCWidgetBackgroundColor);
	root.appendChild(tag);
	if (hasCustomBackgroundColor() == true)
		str.setNum(paletteBackgroundColor().rgb());
	else
		str = KXMLQLCVCWidgetColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background image */
	tag = doc->createElement(KXMLQLCVCWidgetBackgroundImage);
	root.appendChild(tag);
	if (backgroundImage() != QString::null)
		str = m_backgroundImage;
	else
		str = KXMLQLCVCWidgetBackgroundImageNone;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Font */
	tag = doc->createElement(KXMLQLCVCWidgetFont);
	root.appendChild(tag);
	if (hasCustomFont() == true)
		str = font().toString();
	else
		str = KXMLQLCVCWidgetFontDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);	

	return true;
}

/*****************************************************************************
 * QLC Mode change
 *****************************************************************************/
void VCWidget::slotModeChanged(App::Mode mode)
{
	repaint();
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCWidget::invokeMenu(QPoint point)
{
	QPopupMenu* menu = createMenu();
	menu->exec(point);
	delete menu;
}

QPopupMenu* VCWidget::createMenu()
{
	// Create edit menu here so that all submenus can be its children
	/* DO NOT set this as the menu's parent object. Otherwise deleting this
	   thru the menu will crash the whole program. */
	QPopupMenu* editMenu = new QPopupMenu(NULL);

	// Foreground menu
	QPopupMenu* fgMenu = new QPopupMenu(editMenu);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuForegroundColor);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuForegroundDefault);

	// Background menu
	QPopupMenu* bgMenu = new QPopupMenu(editMenu);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuBackgroundColor);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			   "&Image...", KVCMenuBackgroundImage);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuBackgroundDefault);

	// Font menu
	QPopupMenu* fontMenu = new QPopupMenu(editMenu);
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
			     "&Font...", KVCMenuFont);
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuFontDefault);

	// Frame menu
	QPopupMenu* frameMenu = new QPopupMenu(editMenu);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framesunken.png")),
			      "&Sunken", KVCMenuFrameSunken);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frameraised.png")),
			      "&Raised", KVCMenuFrameRaised);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framenone.png")),
			      "&None", KVCMenuFrameNone);

	// Stacking order menu
	QPopupMenu* stackMenu = new QPopupMenu(editMenu);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/up.png")),
			      "Bring to &Front", KVCMenuStackingRaise);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/down.png")),
			      "Send to &Back", KVCMenuStackingLower);

	// Edit menu
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

	editMenu->insertSeparator();

	editMenu->insertItem("Background", bgMenu);
	editMenu->insertItem("Foreground", fgMenu);
	editMenu->insertItem("Font", fontMenu);
	editMenu->insertItem("Frame", frameMenu);
	editMenu->insertItem("Stacking Order", stackMenu);

	connect(editMenu, SIGNAL(activated(int)),
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

	return editMenu;
}

void VCWidget::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuEditCut:
		break;
	case KVCMenuEditCopy:
		break;
	case KVCMenuEditPaste:
		break;
	case KVCMenuEditDelete:
		scram();
		break;

	case KVCMenuEditRename:
		rename();
		break;

	case KVCMenuEditProperties:
		editProperties();
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
		setFrameStyle(KVCWidgetFrameStyleSunken);
		break;

	case KVCMenuFrameRaised:
		setFrameStyle(KVCWidgetFrameStyleRaised);
		break;

	case KVCMenuFrameNone:
		setFrameStyle(KVCWidgetFrameStyleNone);
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

/*****************************************************************************
 * Widget move & resize
 *****************************************************************************/

void VCWidget::resize(QPoint p)
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
		p.setX(0);
	else if (p.x() > parentWidget()->width())
		p.setX(parentWidget()->width());

	// Don't move beyond top or bottom
	if (p.y() < 0)
		p.setY(0);
	else if (p.y() > parentWidget()->height())
		p.setY(parentWidget()->height());

	// Map back so that this can be resized
	p = mapFromParent(p);

	// Do the resize
	QFrame::resize(p.x(), p.y());
}

void VCWidget::move(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Don't move beyond left or right
	if (p.x() < 0)
		p.setX(0);
	else if (p.x() + rect().width() > parentWidget()->width())
		p.setX(parentWidget()->width() - rect().width());

	// Don't move beyond top or bottom
	if (p.y() < 0)
		p.setY(0);
	else if (p.y() + rect().height() > parentWidget()->height())
		p.setY(parentWidget()->height() - rect().height());

	// Do the move
	QFrame::move(p);
}

/*****************************************************************************
 * Frame style converters
 *****************************************************************************/

QString VCWidget::frameStyleToString(const int style)
{
	if (style == KVCWidgetFrameStyleSunken)
		return "Sunken";
	else if (style == KVCWidgetFrameStyleRaised)
		return "Raised";
	else
		return "None";
}

int VCWidget::stringToFrameStyle(const QString& style)
{
	if (style == "Sunken")
		return KVCWidgetFrameStyleSunken;
	else if (style == "Raised")
		return KVCWidgetFrameStyleRaised;
	else
		return KVCWidgetFrameStyleNone;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCWidget::paintEvent(QPaintEvent* e)
{
	int caption_x = 0;
	int caption_y = 0;

	// First paint whatever QFrame wants to
	QFrame::paintEvent(e);

	// Draw the widget's caption
	QPainter painter(this);
	QFontMetrics metrics = painter.fontMetrics();
	QSize textSize = metrics.size(SingleLine, caption());
	
	caption_x = static_cast<int> (floor((width() / 2.0) -
					   (textSize.width() / 2.0)));	
	caption_y = static_cast<int> (floor((height() / 2.0) + 
			    ((textSize.height() - metrics.leading()) / 2.0)));
	
	painter.drawText(caption_x, caption_y, caption());

	/* Draw selection frame */
	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		// Draw a dotted line around the widget
		QPen pen(DotLine);
		pen.setWidth(2);
		painter.setPen(pen);
		painter.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		painter.fillRect(rect().width() - 10,
				 rect().height() - 10, 10, 10, b);
	}
}

void VCWidget::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (m_resizeMode == true)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if (e->button() & LeftButton || e->button() & MidButton)
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

void VCWidget::mouseReleaseEvent(QMouseEvent* e)
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

void VCWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		slotMenuCallback(KVCMenuEditProperties);
}

void VCWidget::mouseMoveEvent(QMouseEvent* e)
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
