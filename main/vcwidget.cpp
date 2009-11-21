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
#include <QMetaObject>
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
#include "vcproperties.h"
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

	setMinimumSize(QSize(20, 20));
	
	m_backgroundImage = QString::null;
	m_hasCustomBackgroundColor = false;
	m_hasCustomForegroundColor = false;
	m_hasCustomFont = false;
	m_frameStyle = KVCFrameStyleNone;

	m_resizeMode = false;

	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);
	setEnabled(true);

	m_inputUniverse = KInputUniverseInvalid;
	m_inputChannel = KInputChannelInvalid;

	connect(_app, SIGNAL(modeChanged(App::Mode)), 
		this, SLOT(slotModeChanged(App::Mode)));

	/* Listen to parent's (only VCWidget-kind) key signals */
	if (parent->inherits(metaObject()->className()) == true)
	{
		connect(parent, SIGNAL(keyPressed(const QKeySequence&)),
			this, SLOT(slotKeyPressed(const QKeySequence&)));
		connect(parent,	SIGNAL(keyReleased(const QKeySequence&)),
			this, SLOT(slotKeyReleased(const QKeySequence&)));
	}
}

VCWidget::~VCWidget()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

bool VCWidget::copyFrom(const VCWidget* widget)
{
	if (widget == NULL)
		return false;

	m_backgroundImage = widget->m_backgroundImage;

	m_hasCustomBackgroundColor = widget->m_hasCustomBackgroundColor;
	if (m_hasCustomBackgroundColor == true)
		setBackgroundColor(widget->backgroundColor());

	m_hasCustomForegroundColor = widget->m_hasCustomForegroundColor;
	if (m_hasCustomForegroundColor == true)
		setForegroundColor(widget->foregroundColor());

	m_hasCustomFont = widget->m_hasCustomFont;
	if (m_hasCustomFont == true)
		setFont(widget->font());

	m_frameStyle = widget->m_frameStyle;

	setGeometry(widget->geometry());
	setCaption(widget->caption());

	m_inputUniverse = widget->m_inputUniverse;
	m_inputChannel = widget->m_inputChannel;

	return true;
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

/*****************************************************************************
 * Background color
 *****************************************************************************/

void VCWidget::setBackgroundColor(const QColor& color)
{
	QPalette pal = palette();

	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;

	pal.setColor(QPalette::Window, color);
	setPalette(pal);

	_app->doc()->setModified();
}

void VCWidget::resetBackgroundColor()
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

void VCWidget::resetForegroundColor()
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

void VCWidget::resetFont()
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

/*****************************************************************************
 * Frame style
 *****************************************************************************/

void VCWidget::setFrameStyle(int style)
{
	m_frameStyle = style;
	update();
	_app->doc()->setModified();
}

void VCWidget::resetFrameStyle()
{
	setFrameStyle(KVCFrameStyleNone);
}

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

void VCWidget::editProperties()
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
 * Key sequence handler
 *****************************************************************************/

void VCWidget::slotKeyPressed(const QKeySequence& keySequence)
{
	emit keyPressed(keySequence);
}

void VCWidget::slotKeyReleased(const QKeySequence& keySequence)
{
	emit keyReleased(keySequence);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCWidget::loadXMLAppearance(const QDomElement* root)
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

bool VCWidget::loadXMLInput(const QDomElement* root)
{
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
	/* Reset mouse cursor */
	unsetCursor();
	
	/* Force an update to get rid of selection markers */
	update();

	/* Patch the signal thru to all children */
	emit modeChanged(mode);
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCWidget::invokeMenu(const QPoint& point)
{
	/* No point coming here if there is no VC instance */
	VirtualConsole* vc = VirtualConsole::instance();
	if (vc == NULL)
		return;

	QMenu* menu = vc->editMenu();
	Q_ASSERT(menu != NULL);
	menu->exec(point);
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCWidget::customMenu(QMenu* parentMenu)
{
	Q_UNUSED(parentMenu);
	return NULL;
}

/*****************************************************************************
 * Widget move & resize
 *****************************************************************************/
void VCWidget::resize(const QSize& size)
{
	QSize sz(size);

	// Force grid settings, if applicable
	if (VirtualConsole::properties().isGridEnabled() == true)
	{
		sz.setWidth(size.width() -
			(size.width() % VirtualConsole::properties().gridX()));
		sz.setHeight(size.height() -
			(size.height() % VirtualConsole::properties().gridY()));
	}

	// Resize
	QWidget::resize(sz);
}

void VCWidget::move(const QPoint& point)
{
	QPoint pt(point);

	// Force grid settings, if applicable
	if (VirtualConsole::properties().isGridEnabled() == true)
	{
		pt.setX(point.x() - 
			(point.x() % VirtualConsole::properties().gridX()));
		pt.setY(point.y() - 
			(point.y() % VirtualConsole::properties().gridY()));
	}

	// Don't move beyond left or right
	if (pt.x() < 0)
		pt.setX(0);
	else if (pt.x() + rect().width() > parentWidget()->width())
		pt.setX(parentWidget()->width() - rect().width());

	// Don't move beyond top or bottom
	if (pt.y() < 0)
		pt.setY(0);
	else if (pt.y() + rect().height() > parentWidget()->height())
		pt.setY(parentWidget()->height() - rect().height());

	// Move
	QWidget::move(pt);
}


/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCWidget::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);

	/* No point coming here if there is no VC instance */
	VirtualConsole* vc = VirtualConsole::instance();
	if (vc == NULL)
		return;

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

	/* Draw a frame border if such is specified for this widget */
	if (option.state != QStyle::State_None)
	{
		style()->drawPrimitive(QStyle::PE_Frame, &option,
				       &painter, this);
	}

	QWidget::paintEvent(e);

	/* Draw selection frame */
	if (_app->mode() == App::Design && vc->isWidgetSelected(this) == true)
	{
		/* Draw a dotted line around the widget */
		QPen pen(Qt::DotLine);
		pen.setCapStyle(Qt::RoundCap);
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
	Q_ASSERT(e != NULL);

	if (_app->mode() == App::Operate)
	{
		QWidget::mousePressEvent(e);
		return;
	}

	/* Perform widget de/selection in virtualconsole's selection buffer */
	handleWidgetSelection(e);

	/* Resize mode */
	if (m_resizeMode == true)
	{
		setMouseTracking(false);
		m_resizeMode = false;
	}

	/* Move, resize or context menu invocation */
	if (e->button() & Qt::LeftButton || e->button() & Qt::MidButton)
	{
		/* Start moving or resizing based on where the click landed */
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
		/* Menu invocation */
		m_mousePressPoint = QPoint(e->x(), e->y());
		invokeMenu(mapToGlobal(e->pos()));
	}
}

void VCWidget::handleWidgetSelection(QMouseEvent* e)
{
	/* No point coming here if there is no VC */
	VirtualConsole* vc = VirtualConsole::instance();
	if (vc == NULL)
		return;

	/* Widget selection logic (like in Qt Designer) */
	if (e->button() == Qt::LeftButton)
	{
		if (e->modifiers() & Qt::ShiftModifier)
		{
			/* Toggle selection with LMB when shift is pressed */
			bool selected = vc->isWidgetSelected(this);
			vc->setWidgetSelected(this, !selected);
		}
		else
		{
			if (vc->isWidgetSelected(this) == false)
			{
				/* Select only this */
				vc->clearWidgetSelection();
				vc->setWidgetSelected(this, true);
			}
		}
	}
	else if (e->button() == Qt::RightButton)
	{
		if (vc->isWidgetSelected(this) == false)
		{
			/* Select only this */
			vc->clearWidgetSelection();
			vc->setWidgetSelected(this, true);
		}
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

void VCWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
		editProperties();
	else
		QWidget::mouseDoubleClickEvent(e);
}

void VCWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_resizeMode == true)
		{
			QPoint p(mapFromGlobal(QCursor::pos()));
			resize(QSize(p.x(), p.y()));
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
