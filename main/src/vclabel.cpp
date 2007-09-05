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
#include "common/settings.h"
#include "configkeys.h"
#include "common/minmax.h"
#include "common/filehandler.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qevent.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qfiledialog.h>

extern App* _app;

const int KFrameStyle      ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask       ( 0xff ); // Produces opposite colors with XOR

VCLabel::VCLabel(QWidget* parent)
	: QLabel(parent, "Label"),
	  m_resizeMode ( false )
{
}

VCLabel::~VCLabel()
{
	//_app->virtualConsole()->unRegisterKeyReceiver(this);
}

void VCLabel::init()
{
	setMinimumSize(20, 20);

	setText("Label");
	setAlignment(WordBreak | AlignCenter);

	setFrameStyle(KFrameStyle);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCLabel::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCLabel::backgroundImage()
{
	return m_backgroundImage;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCLabel::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCLabel::resetBackgroundColor()
{
	m_hasCustomBackgroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCLabel::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCLabel::resetForegroundColor()
{
	m_hasCustomForegroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCLabel::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCLabel::resetFont()
{
	m_hasCustomFont = false;
	/* TODO */
}

/*****************************************************************************
 * 
 *****************************************************************************/

void VCLabel::setCaption(const QString& text)
{
	setText(text);
	QWidget::setCaption(text);
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

	/* Frame style */
	str = root->attribute(KXMLQLCVirtualConsoleFrameStyle);
	setFrameStyle(str.toInt());

	/* Foreground Color */
	str = root->attribute(KXMLQLCVCForegroundColor);
	if (str.length() != 0 && str != KXMLQLCVCColorDefault)
		setForegroundColor(QColor(str.toUInt()));

	/* Background Color */
	str = root->attribute(KXMLQLCVCBackgroundColor);
	if (str.length() != 0 && str != KXMLQLCVCColorDefault)
		setBackgroundColor(QColor(str.toUInt()));

	/* Background Color */
	str = root->attribute(KXMLQLCVCBackgroundImage);
	if (str.length() != 0 && str != KXMLQLCVCBackgroundImageNone)
		setBackgroundImage(str);

	/* Font */
	str = root->attribute(KXMLQLCVCFont);
	if (str.length() != 0 && str != KXMLQLCVCFontDefault)
	{
		QFont font;
		font.fromString(str);
		setFont(font);
	}

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
		else
		{
			qWarning("Unknown label tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
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

	/* Frame style */
	str.setNum(frameStyle());
	root.setAttribute(KXMLQLCVirtualConsoleFrameStyle, str);

	/* Foreground color */
	if (hasCustomForegroundColor() == true)
	{
		str.setNum(paletteForegroundColor().rgb());
		root.setAttribute(KXMLQLCVCForegroundColor, str);
	}
	else
	{
		root.setAttribute(KXMLQLCVCForegroundColor,
				  KXMLQLCVCColorDefault);
	}

	/* Background color */
	if (hasCustomBackgroundColor() == true)
	{
		str.setNum(paletteBackgroundColor().rgb());
		root.setAttribute(KXMLQLCVCBackgroundColor, str);
	}
	else
	{
		root.setAttribute(KXMLQLCVCBackgroundColor,
				  KXMLQLCVCColorDefault);
	}

	/* Background image */
	if (backgroundImage() != QString::null)
	{
		root.setAttribute(KXMLQLCVCBackgroundImage, m_backgroundImage);
	}
	else
	{
		root.setAttribute(KXMLQLCVCBackgroundImage,
				  KXMLQLCVCBackgroundImageNone);
	}

	return FileHandler::saveXMLWindowState(doc, &root, this);
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

void VCLabel::invokeMenu(QPoint point)
{
	_app->virtualConsole()->editMenu()->exec(point);
}

void VCLabel::parseWidgetMenu(int item)
{
	switch (item)
	{
	case KVCMenuBackgroundFrame:
	{
		if (frameStyle() & KFrameStyle)
		{
			setFrameStyle(NoFrame);
		}
		else
		{
			setFrameStyle(KFrameStyle);
		}
		_app->doc()->setModified();
	}
	break;

	default:
		break;
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
	parseWidgetMenu(KVCMenuEditRename);
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
		parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}
}

void VCLabel::slotModeChanged()
{
	repaint();
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
