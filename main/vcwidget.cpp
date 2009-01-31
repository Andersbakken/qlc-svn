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

#include <QStyleOptionFrame>
#include <QApplication>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QCursor>
#include <QPixmap>
#include <QDebug>
#include <QBrush>
#include <QPoint>
#include <QStyle>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>

#include <cmath>
#include "common/qlcfile.h"

#include "virtualconsole.h"
#include "inputmap.h"
#include "vcwidget.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCWidget::VCWidget(QWidget* parent) : QWidget(parent)
{
	Q_ASSERT(parent != NULL);

	/* Set the class name "VCWidget" as the object name as well */
	setObjectName(VCWidget::staticMetaObject.className());

	m_backgroundImage = QString::null;
	m_hasCustomBackgroundColor = false;
	m_hasCustomForegroundColor = false;
	m_hasCustomFont = false;
	m_frameStyle = KVCFrameStyleNone;

	m_xpos = 0;
	m_ypos = 0;
	m_resizeMode = false;

	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);
	setEnabled(true);

	setMinimumSize(20, 20);

	m_inputUniverse = KInputUniverseInvalid;
	m_inputChannel = KInputChannelInvalid;

	connect(parent, SIGNAL(modeChanged(App::Mode)), 
		this, SLOT(slotModeChanged(App::Mode)));
}

VCWidget::~VCWidget()
{
}

void VCWidget::slotCut()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VCWidget::slotCopy()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VCWidget::slotPaste()
{
	QMessageBox::information(this, "TODO", "Not implemented");
}

void VCWidget::slotDelete()
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

/*****************************************************************************
 * Background image
 *****************************************************************************/

void VCWidget::setBackgroundImage(const QString& path)
{
	QPalette pal = palette();

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;

	/* setAutoFillBackground(true); */
	pal.setBrush(QPalette::Window, QBrush(QPixmap(path)));
	setPalette(pal);

	_app->doc()->setModified();
}

void VCWidget::slotChooseBackgroundImage()
{
	QString path;
	path = QFileDialog::getOpenFileName(this,
					    tr("Select background image"),
					    m_backgroundImage,
					    "Images (*.png *.xpm *.jpg *.gif)");
	if (path.isEmpty() == false)
		setBackgroundImage(path);
}

/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCWidget::setBackgroundColor(const QColor& color)
{
	QPalette pal = palette();

	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;

	/* setAutoFillBackground(true); */
	pal.setColor(QPalette::Window, color);
	setPalette(pal);

	_app->doc()->setModified();
}

void VCWidget::slotChooseBackgroundColor()
{
	QColor color;

	color = QColorDialog::getColor(palette().color(QPalette::Window));
	if (color.isValid() == true)
		setBackgroundColor(color);
}

void VCWidget::slotResetBackgroundColor()
{
	QColor fg;

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = QString::null;

	/* Store foreground color */
	if (m_hasCustomForegroundColor == true)
		fg = palette().color(QPalette::WindowText);

	/* Reset the whole palette to application palette */
	setPalette(QApplication::palette());
	/* setAutoFillBackground(false); */

	/* Restore foreground color */
	if (fg.isValid() == true)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::WindowText, fg);
		setPalette(pal);
	}

	_app->doc()->setModified();
}

/*****************************************************************************
 * Foreground color
 *****************************************************************************/

void VCWidget::setForegroundColor(const QColor& color)
{
	QPalette pal = palette();

	m_hasCustomForegroundColor = true;

	pal.setColor(QPalette::WindowText, color);
	setPalette(pal);

	_app->doc()->setModified();
}

void VCWidget::slotChooseForegroundColor()
{
	QColor color;

	color = QColorDialog::getColor(palette().color(QPalette::WindowText));
	if (color.isValid() == true)
		setForegroundColor(color);
}

void VCWidget::slotResetForegroundColor()
{
	QColor bg;

	m_hasCustomForegroundColor = false;

	/* Store background color */
	if (m_hasCustomBackgroundColor == true)
		bg = palette().color(QPalette::Window);

	/* Reset the whole palette to application palette */
	setPalette(QApplication::palette());

	/* Restore foreground color */
	if (bg.isValid() == true)
		setBackgroundColor(bg);
	else if (m_backgroundImage.isEmpty() == false)
		setBackgroundImage(m_backgroundImage);

	_app->doc()->setModified();
}

/*****************************************************************************
 * Font
 *****************************************************************************/

void VCWidget::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
	_app->doc()->setModified();
}

void VCWidget::slotChooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

void VCWidget::slotResetFont()
{
	m_hasCustomFont = false;
	setFont(QFont());
	_app->doc()->setModified();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCWidget::setCaption(const QString& text)
{
	setWindowTitle(text);
	update();
	_app->doc()->setModified();
}

void VCWidget::slotRename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText(this, tr("Rename widget"),
				     tr("Set widget caption:"),
				     QLineEdit::Normal, caption(), &ok);
	if (ok == true)
		setCaption(text);
}

/*****************************************************************************
 * Stacking
 *****************************************************************************/

void VCWidget::raise()
{
	QWidget::raise();
	_app->doc()->setModified();
}

void VCWidget::lower()
{
	QWidget::lower();
	_app->doc()->setModified();
}

/*****************************************************************************
 * Frame style
 *****************************************************************************/

void VCWidget::setFrameStyle(int style)
{
	m_frameStyle = style;
	update();
	_app->doc()->setModified();
}

void VCWidget::slotSetFrameSunken()
{
	setFrameStyle(KVCFrameStyleSunken);
}

void VCWidget::slotSetFrameRaised()
{
	setFrameStyle(KVCFrameStyleRaised);
}

void VCWidget::slotResetFrame()
{
	setFrameStyle(KVCFrameStyleNone);
}

/*****************************************************************************
 * Frame style converters
 *****************************************************************************/

QString VCWidget::frameStyleToString(int style)
{
	if (style == KVCFrameStyleSunken)
		return "Sunken";
	else if (style == KVCFrameStyleRaised)
		return "Raised";
	else
		return "None";
}

int VCWidget::stringToFrameStyle(const QString& style)
{
	if (style == "Sunken")
		return KVCFrameStyleSunken;
	else if (style == "Raised")
		return KVCFrameStyleRaised;
	else
		return KVCFrameStyleNone;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCWidget::slotProperties()
{
	QMessageBox::information(_app, staticMetaObject.className(),
				 tr("This widget has no properties"));
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCWidget::setInputSource(t_input_universe uni, t_input_channel ch)
{
	if (uni == KInputUniverseInvalid || ch == KInputChannelInvalid)
	{
		/* If either one of the new values is invalid we end up here
		   to disconnect from inputmap and setting both of the values
		   invalid. */
		m_inputUniverse = KInputUniverseInvalid;
		m_inputChannel = KInputChannelInvalid;

		/* Even though we might not be connected, it is safe to do a
		   disconnect in any case. */
		disconnect(_app->inputMap(),
			   SIGNAL(inputValueChanged(t_input_universe,
						    t_input_channel,
						    t_input_value)),
			   this, SLOT(slotInputValueChanged(t_input_universe,
							    t_input_channel,
							    t_input_value)));
	}
	else if (m_inputUniverse == KInputUniverseInvalid ||
		 m_inputChannel == KInputChannelInvalid)
	{
		/* Execution comes here only if both of the new values
		   are valid and the existing values are invalid, in which
		   case a new connection must be made. */
		m_inputUniverse = uni;
		m_inputChannel = ch;

		connect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
	else
	{
		/* Execution comes here only if the current uni & channel are
		 * valid and the new ones are valid as well. So we don't do a
		 * new connection, which would end up in duplicate values.
		 * Just update the new values and get it over with. */
		 m_inputUniverse = uni;
		 m_inputChannel = ch;
	}
}

void VCWidget::slotInputValueChanged(t_input_universe universe,
				     t_input_channel channel,
				     t_input_value value)
{
	Q_UNUSED(universe);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCWidget::loadXMLAppearance(QDomDocument*, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCAppearance)
	{
		qDebug() << "Appearance node not found!";
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCFrameStyle)
		{
			setFrameStyle(stringToFrameStyle(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCVCWidgetForegroundColor)
		{
			if (tag.text() != KXMLQLCVCWidgetColorDefault)
				setForegroundColor(QColor(tag.text().toUInt()));
		}
		else if (tag.tagName() == KXMLQLCVCWidgetBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCWidgetColorDefault)
				setBackgroundColor(QColor(tag.text().toUInt()));
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
			qDebug() << "Unknown appearance tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool VCWidget::loadXMLInput(QDomDocument* doc, QDomElement* root)
{
	Q_UNUSED(doc);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCWidgetInput)
	{
		qDebug() << "Input node not found!";
		return false;
	}
	else
	{
		t_input_universe uni;
		t_input_channel ch;

		uni = root->attribute(KXMLQLCVCWidgetInputUniverse).toInt();
		ch = root->attribute(KXMLQLCVCWidgetInputChannel).toInt();

		setInputSource(uni, ch);
	}

	return true;
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
	tag = doc->createElement(KXMLQLCVCFrameStyle);
	root.appendChild(tag);
	text = doc->createTextNode(frameStyleToString(frameStyle()));
	tag.appendChild(text);

	/* Foreground color */
	tag = doc->createElement(KXMLQLCVCWidgetForegroundColor);
	root.appendChild(tag);
	if (hasCustomForegroundColor() == true)
		str.setNum(foregroundColor().rgb());
	else
		str = KXMLQLCVCWidgetColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background color */
	tag = doc->createElement(KXMLQLCVCWidgetBackgroundColor);
	root.appendChild(tag);
	if (hasCustomBackgroundColor() == true)
		str.setNum(backgroundColor().rgb());
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

bool VCWidget::saveXMLInput(QDomDocument* doc, QDomElement* root)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (m_inputUniverse != KInputUniverseInvalid &&
	    m_inputChannel != KInputChannelInvalid)
	{
		QDomElement tag;
		QDomText text;

		tag = doc->createElement(KXMLQLCVCWidgetInput);
		root->appendChild(tag);
		tag.setAttribute(KXMLQLCVCWidgetInputUniverse,
				 QString("%1").arg(inputUniverse()));
		tag.setAttribute(KXMLQLCVCWidgetInputChannel,
				 QString("%1").arg(inputChannel()));
	}

	return true;
}

/*****************************************************************************
 * QLC Mode change
 *****************************************************************************/

void VCWidget::slotModeChanged(App::Mode mode)
{
	update();

	/* Patch the signal thru to all children */
	emit modeChanged(mode);
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCWidget::invokeMenu(QPoint point)
{
	QMenu* menu = createMenu();
	menu->exec(point);
	delete menu;
}

QMenu* VCWidget::createMenu()
{
	QAction* action;

	// Create edit menu here so that all submenus can be its children
	/* DO NOT set this as the menu's parent object. Otherwise deleting this
	   thru the menu will crash the whole program. */
	QMenu* editMenu = new QMenu(NULL);
	action = editMenu->addAction(QIcon(":/editcut.png"), "Cut",
				     this, SLOT(slotCut()));
	action->setEnabled(false);
	action = editMenu->addAction(QIcon(":/editcopy.png"), "Copy",
				     this, SLOT(slotCopy()));
	action->setEnabled(false);
	action = editMenu->addAction(QIcon(":/editpaste.png"), "Paste",
				     this, SLOT(slotPaste()));
	action->setEnabled(false);
	editMenu->addAction(QIcon(":/editdelete.png"), "Delete",
			    this, SLOT(slotDelete()));
	editMenu->addSeparator();
	editMenu->addAction(QIcon(":/configure.png"), "Properties...",
			    this, SLOT(slotProperties()));
	editMenu->addAction(QIcon(":/editclear.png"), "Rename...",
			    this, SLOT(slotRename()));
	editMenu->addSeparator();

	// Background menu
	QMenu* bgMenu = new QMenu(editMenu);
	bgMenu->setTitle("Background");
	bgMenu->addAction(QIcon(":/color.png"), "Color...",
			  this, SLOT(slotChooseBackgroundColor()));
	bgMenu->addAction(QIcon(":/image.png"), "Image...",
			  this, SLOT(slotChooseBackgroundImage()));
	bgMenu->addAction(QIcon(":/undo.png"), "Default",
			  this, SLOT(slotResetBackgroundColor()));

	// Foreground menu
	QMenu* fgMenu = new QMenu(editMenu);
	fgMenu->setTitle("Foreground");
	fgMenu->addAction(QIcon(":/color.png"), "Color...",
			  this, SLOT(slotChooseForegroundColor()));
	fgMenu->addAction(QIcon(":/undo.png"), "Default",
			  this, SLOT(slotResetForegroundColor()));

	// Font menu
	QMenu* fontMenu = new QMenu(editMenu);
	fontMenu->setTitle("Font");
	fontMenu->addAction(QIcon(":/fonts.png"), "Font...",
			   this, SLOT(slotChooseFont()));
	fontMenu->addAction(QIcon(":/undo.png"), "Default",
			    this, SLOT(slotResetFont()));

	/* Frame menu */
	QMenu* frameMenu = new QMenu(editMenu);
	frameMenu->setTitle("Frame");
	frameMenu->addAction(QIcon(":/framesunken.png"), "Sunken",
			     this, SLOT(slotSetFrameSunken()));
	frameMenu->addAction(QIcon(":/frameraised.png"), "Raised",
			     this, SLOT(slotSetFrameRaised()));
	frameMenu->addAction(QIcon(":/framenone.png"), "None",
			     this, SLOT(slotResetFrame()));

	// Stacking order menu
	QMenu* stackMenu = new QMenu(editMenu);
	stackMenu->setTitle("Stacking order");
	stackMenu->addAction(QIcon(":/up.png"), "Raise",
			     this, SLOT(raise()));
	stackMenu->addAction(QIcon(":/down.png"), "Lower",
			     this, SLOT(lower()));

	// Menu construction
	editMenu->addMenu(bgMenu);
	editMenu->addMenu(fgMenu);
	editMenu->addMenu(fontMenu);
	editMenu->addMenu(frameMenu);
	editMenu->addMenu(stackMenu);

	return editMenu;
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
	QWidget::resize(p.x(), p.y());
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
	QWidget::move(p);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCWidget::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);

	QPainter painter(this);

	/* Draw frame according to style */
	QStyleOptionFrame option;
	option.initFrom(this);
	if (frameStyle() == KVCFrameStyleSunken)
		option.state = QStyle::State_Sunken;
	else if (frameStyle() == KVCFrameStyleRaised)
		option.state = QStyle::State_Raised;
	else
		option.state = QStyle::State_None;

	if (option.state != QStyle::State_None)
		style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

	/* Draw selection frame */
	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		/* Draw a dotted line around the widget */
		QPen pen(Qt::DotLine);
		pen.setWidth(0);
		painter.setPen(pen);
		painter.drawRect(0, 0, rect().width(), rect().height());

		/* Draw a resize handle */
		QIcon icon(":/resize.png");
		painter.drawPixmap(rect().width() - 16, rect().height() - 16,
			icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
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

		if (e->button() & Qt::LeftButton || e->button() & Qt::MidButton)
		{
			if (e->x() > rect().width() - 10 &&
			    e->y() > rect().height() - 10)
			{
				m_resizeMode = true;
				setMouseTracking(true);
				setCursor(QCursor(Qt::SizeFDiagCursor));
			}
			else
			{
				m_mousePressPoint = QPoint(e->x(), e->y());
				setCursor(QCursor(Qt::SizeAllCursor));
			}
		}
		else if (e->button() & Qt::RightButton)
		{
			m_mousePressPoint = QPoint(e->x(), e->y());
			invokeMenu(mapToGlobal(e->pos()));
		}
	}
	else
	{
		QWidget::mousePressEvent(e);
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
		QWidget::mouseReleaseEvent(e);
	}
}

void VCWidget::mouseDoubleClickEvent(QMouseEvent*)
{
	if (_app->mode() == App::Design)
		slotProperties();
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
		else if (e->buttons() & Qt::LeftButton ||
			 e->buttons() & Qt::MidButton)
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
		QWidget::mouseMoveEvent(e);
	}
}
