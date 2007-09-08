/*
  Q Light Controller
  vclabel.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include "vclabel.h"
#include "app.h"
#include "doc.h"
#include "floatingedit.h"
#include "virtualconsole.h"

#include "common/qlcimagepreview.h"
#include "common/minmax.h"
#include "common/filehandler.h"

#include <qlineedit.h>
#include <qevent.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qpainter.h>
#include <qfont.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

extern App* _app;

VCLabel::VCLabel(QWidget* parent)
	: QLabel(parent, "Label"),
	  m_hasCustomForegroundColor ( false ),
	  m_hasCustomBackgroundColor ( false ),
	  m_hasCustomFont ( false ),
	  m_resizeMode ( false )
{
}

VCLabel::~VCLabel()
{
}

void VCLabel::init()
{
	setMinimumSize(20, 20);

	setCaption("Label");
	setAlignment(WordBreak | AlignCenter);

	setFrameStyle(KFrameStyleSunken);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

void VCLabel::destroy()
{
	int result = QMessageBox::warning(this,
					  QString(caption()),
					  QString("Remove selected label?"),
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
void VCLabel::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setIconText(path);
	setPaletteBackgroundPixmap(QPixmap(path));
	_app->doc()->setModified();
}

const QString& VCLabel::backgroundImage()
{
	return m_backgroundImage;
}

void VCLabel::chooseBackgroundImage()
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
void VCLabel::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
	_app->doc()->setModified();
}

void VCLabel::resetBackgroundColor()
{
	m_hasCustomBackgroundColor = false;
	/* TODO */
	_app->doc()->setModified();
}

void VCLabel::chooseBackgroundColor()
{
	QColor color;
	color = QColorDialog::getColor(backgroundColor());
	if (color.isValid())
		setBackgroundColor(color);
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCLabel::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
	_app->doc()->setModified();
}

void VCLabel::resetForegroundColor()
{
	m_hasCustomForegroundColor = false;
	/* TODO */
	_app->doc()->setModified();
}

void VCLabel::chooseForegroundColor()
{
	QColor color;
	color = QColorDialog::getColor(foregroundColor());
	if (color.isValid())
		setForegroundColor(color);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCLabel::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
	_app->doc()->setModified();
}

void VCLabel::resetFont()
{
	m_hasCustomFont = false;
	/* TODO */
	_app->doc()->setModified();
}

void VCLabel::chooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCLabel::setCaption(const QString& text)
{
	setText(text);
	QWidget::setCaption(text);
	_app->doc()->setModified();
}

void VCLabel::rename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText("Rename label",
				     "Set label caption:", QLineEdit::Normal,
				     QString::null, &ok, this );
	if (ok == true && text.isEmpty() == false)
		setCaption(text);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool VCLabel::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCLabel* label = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCLabel)
	{
		qWarning("Label node not found!");
		return false;
	}

	/* Create a new label into its parent */
	label = new VCLabel(parent);
	label->init();
	label->show();

	/* Continue loading */
	return label->loadXML(doc, root);
}

bool VCLabel::loadXML(QDomDocument* doc, QDomElement* root)
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

	if (root->tagName() != KXMLQLCVCLabel)
	{
		qWarning("Label node not found!");
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
		else
		{
			qWarning("Unknown label tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCLabel::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
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
				QColor color(tag.text().toUInt());
				setForegroundColor(color);
			}
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
				setBackgroundColor(QColor(tag.text().toUInt()));
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

bool VCLabel::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCLabel);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Window state */
	FileHandler::saveXMLWindowState(doc, &root, this);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	return true;
}

bool VCLabel::saveXMLAppearance(QDomDocument* doc, QDomElement* label_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(label_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	label_root->appendChild(root);

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

void VCLabel::slotModeChanged()
{
	repaint();
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCLabel::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (e->button() & LeftButton)
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
			invokeMenu(mapToGlobal(e->pos()));
		}
	}

}

void VCLabel::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}

}

void VCLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
	slotMenuCallback(KVCMenuEditRename);
}

void VCLabel::paintEvent(QPaintEvent* e)
{
	QLabel::paintEvent(e);

	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		QPainter p(this);

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

void VCLabel::customEvent(QCustomEvent* e)
{
	if (e->type() == KVCMenuEvent)
	{
		// parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}
}

void VCLabel::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_resizeMode == true)
		{
			QPoint p(QCursor::pos());
			resizeTo(mapFromGlobal(p));
			_app->doc()->setModified();
		}
		else if (e->state() & LeftButton || e->state() & MidButton)
		{
			QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
			p.setX(p.x() - m_mousePressPoint.x());
			p.setY(p.y() - m_mousePressPoint.y());

			moveTo(p);
			_app->doc()->setModified();
		}
	}
	else
	{
		QLabel::mouseMoveEvent(e);
	}
}

/*****************************************************************************
 * Widget move / resize
 *****************************************************************************/

void VCLabel::resizeTo(QPoint p)
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
	resize(p.x(), p.y());
}

void VCLabel::moveTo(QPoint p)
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
	move(p);
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCLabel::invokeMenu(QPoint point)
{
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

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editclear.png")),
			       "&Rename...", KVCMenuEditRename);

	editMenu->setItemEnabled(KVCMenuEditCut, false);
	editMenu->setItemEnabled(KVCMenuEditCopy, false);
	editMenu->setItemEnabled(KVCMenuEditPaste, false);

	editMenu->insertSeparator();

	editMenu->insertItem("Background", bgMenu, 0);
	editMenu->insertItem("Foreground", fgMenu, 0);
	editMenu->insertItem("Font", fontMenu, 0);
	editMenu->insertItem("Frame", frameMenu, 0);
	editMenu->insertItem("Stacking Order", stackMenu, 0);

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
	
	editMenu->exec(point);
	delete editMenu;
	delete bgMenu;
	delete fgMenu;
	delete fontMenu;
	delete frameMenu;
	delete stackMenu;
}

void VCLabel::slotMenuCallback(int item)
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
		destroy();
		break;

	case KVCMenuEditRename:
		rename();
		break;

	case KVCMenuForegroundColor:
		chooseForegroundColor();
		break;

	case KVCMenuForegroundDefault:
		resetBackgroundColor();
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
