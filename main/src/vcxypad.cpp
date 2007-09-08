/*
  Q Light Controller
  vcxypad.cpp

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

#include "vcxypad.h"
#include "xychannelunit.h"
#include "vcbutton.h"
#include "fixture.h"
#include "vclabel.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "common/settings.h"
#include "common/minmax.h"
#include "configkeys.h"
#include "vcxypadproperties.h"
#include "common/filehandler.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qobjectlist.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qlistview.h>

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent) 
	: QFrame(parent, "XYPad"),
	  m_hasCustomForegroundColor ( false ),
	  m_hasCustomBackgroundColor ( false ),
	  m_hasCustomFont ( false ),
	  m_xpos ( 0 ),
	  m_ypos ( 0 ),
	  m_resizeMode ( false )
{
}

VCXYPad::~VCXYPad()
{
	clearChannels();
}

void VCXYPad::init()
{
	setMinimumSize(20, 20);

	resize(120, 120);
	setFrameStyle(KFrameStyleSunken);

	setBackgroundColor(white);

	m_pixmap = QPixmap(QString(PIXMAPS) + QString("/xypad-point.png"));

	m_currentXYPosition.setX(width() / 2);
	m_currentXYPosition.setY(height() / 2);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

/*********************************************************************
 * Background image
 *********************************************************************/
void VCXYPad::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCXYPad::backgroundImage()
{
	return m_backgroundImage;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCXYPad::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCXYPad::resetBackgroundColor()
{
	m_hasCustomBackgroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCXYPad::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCXYPad::resetForegroundColor()
{
	m_hasCustomForegroundColor = false;
	/* TODO */
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCXYPad::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCXYPad::resetFont()
{
	m_hasCustomFont = false;
	/* TODO */
}

/*****************************************************************************
 * Channels
 *****************************************************************************/
void VCXYPad::clearChannels()
{
	m_channelsX.setAutoDelete(true);
	m_channelsX.clear();
	m_channelsX.setAutoDelete(false);

	m_channelsY.setAutoDelete(true);
	m_channelsY.clear();
	m_channelsY.setAutoDelete(false);
}

void VCXYPad::appendChannel(t_axis axis, t_fixture_id fixture, t_channel channel,
			    t_value lowLimit, t_value highLimit, bool reverse)
{
	XYChannelUnit* xyc = NULL;

	if (this->channel(axis, fixture, channel) == NULL)
	{
		xyc = new XYChannelUnit(fixture, channel,
					lowLimit, highLimit, reverse);
		if (axis == KAxisX)
			m_channelsX.append(xyc);
		else
			m_channelsY.append(xyc);
	}
}

void VCXYPad::removeChannel(t_axis axis, t_fixture_id fixture, t_channel channel)
{
}

XYChannelUnit* VCXYPad::channel(t_axis axis, t_fixture_id fixture,
				t_channel channel)
{
	XYChannelUnit* xyc = NULL;
	QPtrList<XYChannelUnit>* list = NULL;

	if (axis == KAxisX)
		list = &m_channelsX;
	else
		list = &m_channelsY;
		
	QPtrListIterator<XYChannelUnit> it(*list);
	while ( (xyc = it.current()) != 0 )
	{
		if (xyc->fixtureID() == fixture && xyc->channel() == channel)
			break;
		++it;
	}

	return xyc;
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/
void VCXYPad::setCurrentXYPosition(const QPoint& point)
{
	m_currentXYPosition = point;;
	repaint();
}

void VCXYPad::setCurrentXYPosition(int x, int y)
{
	setCurrentXYPosition(QPoint(x, y));
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool VCXYPad::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCXYPad* xypad = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCXYPad)
	{
		qWarning("XY Pad node not found!");
		return false;
	}

	/* Create a new xy pad into its parent */
	xypad = new VCXYPad(parent);
	xypad->init();
	xypad->show();

	/* Continue loading */
	return xypad->loadXML(doc, root);
}

bool VCXYPad::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	int xpos = 0;
	int ypos = 0;

	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCXYPad)
	{
		qWarning("XY Pad node not found!");
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
		}
		else if (tag.tagName() == KXMLQLCVCAppearance)
		{
			loadXMLAppearance(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCXYPadPosition)
		{
			str = tag.attribute(KXMLQLCVCXYPadPositionX);
			xpos = str.toInt();

			str = tag.attribute(KXMLQLCVCXYPadPositionY);
			ypos = str.toInt();
		}
		else if (tag.tagName() == KXMLQLCVCXYPadChannel)
		{
			QString axis;
			t_fixture_id fixture = KNoID;
			t_value lowLimit = 0;
			t_value highLimit = 255;
			bool reverse = false;
			t_channel channel = 0;

			/* Axis */
			axis = tag.attribute(KXMLQLCVCXYPadChannelAxis);

			/* Fixture ID */
			str = tag.attribute(KXMLQLCVCXYPadChannelFixture);
			fixture = str.toInt();

			/* Low limit */
			str = tag.attribute(KXMLQLCVCXYPadChannelLowLimit);
			lowLimit = str.toInt();

			/* High limit */
			str = tag.attribute(KXMLQLCVCXYPadChannelHighLimit);
			highLimit = str.toInt();

			/* Reverse */
			str = tag.attribute(KXMLQLCVCXYPadChannelReverse);
			reverse = (bool) str.toInt();

			/* Fixture channel number */
			channel = tag.text().toInt();

			if (axis == KXMLQLCVCXYPadChannelAxisX)
				appendChannel(KAxisX, fixture, channel,
					      lowLimit, highLimit, reverse);
			else
				appendChannel(KAxisY, fixture, channel,
					      lowLimit, highLimit, reverse);
		}
		else
		{
			qWarning("Unknown XY Pad tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}
	
	/* First set window dimensions and AFTER that set the
	   pointer's XY position */
	setGeometry(x, y, w, h);
	setCurrentXYPosition(xpos, ypos);

	return true;
}

bool VCXYPad::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	XYChannelUnit* xyc = NULL;
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC XY Pad entry */
	root = doc->createElement(KXMLQLCVCXYPad);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Current XY Position */
	tag = doc->createElement(KXMLQLCVCXYPadPosition);
	str.setNum(currentXYPosition().x());
	tag.setAttribute(KXMLQLCVCXYPadPositionX, str);
	str.setNum(currentXYPosition().x());
	tag.setAttribute(KXMLQLCVCXYPadPositionY, str);
	root.appendChild(tag);

	/* Window state */
	FileHandler::saveXMLWindowState(doc, &root, this);

	/* X Channels */
	QPtrListIterator<XYChannelUnit> xit(m_channelsX);
	while ( (xyc = xit.current()) != 0 )
	{
		tag = doc->createElement(KXMLQLCVCXYPadChannel);

		/* This is an X axis channel */
		tag.setAttribute(KXMLQLCVCXYPadChannelAxis,
				 KXMLQLCVCXYPadChannelAxisX);

		/* Fixture ID */
		str.setNum(xyc->fixtureID());
		tag.setAttribute(KXMLQLCVCXYPadChannelFixture, str);

		/* Channel low value limit */
		str.setNum(xyc->lo());
		tag.setAttribute(KXMLQLCVCXYPadChannelLowLimit, str);

		/* Channel high value limit */
		str.setNum(xyc->hi());
		tag.setAttribute(KXMLQLCVCXYPadChannelHighLimit, str);

		/* Reverse */
		str.setNum(xyc->reverse());
		tag.setAttribute(KXMLQLCVCXYPadChannelReverse, str);

		/* DMX Channel number */
		str.setNum(xyc->channel());
		text = doc->createTextNode(str);
		tag.appendChild(text);

		root.appendChild(tag);

		++xit;
	}

	/* Y Channels */
	QPtrListIterator<XYChannelUnit> yit(m_channelsY);
	while ( (xyc = yit.current()) != 0 )
	{
		tag = doc->createElement(KXMLQLCVCXYPadChannel);

		/* This is an Y axis channel */
		tag.setAttribute(KXMLQLCVCXYPadChannelAxis,
				 KXMLQLCVCXYPadChannelAxisY);

		/* Fixture ID */
		str.setNum(xyc->fixtureID());
		tag.setAttribute(KXMLQLCVCXYPadChannelFixture, str);

		/* Channel low value limit */
		str.setNum(xyc->lo());
		tag.setAttribute(KXMLQLCVCXYPadChannelLowLimit, str);

		/* Channel high value limit */
		str.setNum(xyc->hi());
		tag.setAttribute(KXMLQLCVCXYPadChannelHighLimit, str);

		/* Fixture channel number */
		str.setNum(xyc->channel());
		text = doc->createTextNode(str);
		tag.appendChild(text);

		root.appendChild(tag);

		++yit;
	}

	/* Appearance */
	saveXMLAppearance(doc, &root);

	return true;
}

bool VCXYPad::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
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

bool VCXYPad::saveXMLAppearance(QDomDocument* doc, QDomElement* xypad_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(xypad_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	xypad_root->appendChild(root);

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
 * Event handlers
 *****************************************************************************/

void VCXYPad::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
	QPainter p(this);
	QPen pen;

	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		// Draw a dotted line around the widget
		pen.setStyle(DotLine);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
	}

	// Draw crosshairs
	pen.setStyle(DotLine);
	pen.setColor(paletteForegroundColor());
	pen.setWidth(1);
	p.setPen(pen);
	p.drawLine(width() / 2, 0, width() / 2, height());
	p.drawLine(0, height() / 2, width(), height() / 2);

	p.drawPixmap(m_currentXYPosition.x() - (m_pixmap.width() / 2),
		     m_currentXYPosition.y() - (m_pixmap.height() / 2),
		     m_pixmap);
}

void VCXYPad::slotModeChanged()
{
	repaint();
}

void VCXYPad::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		_app->virtualConsole()->setSelectedWidget(this);

		if (m_resizeMode == true)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if ((e->button() & LeftButton ||
		     e->button() & MidButton))
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
	else
	{
		setCurrentXYPosition(e->x(), e->y());

		setMouseTracking(true);
		setCursor(Qt::CrossCursor);

		outputDMX(e->x(), e->y());
	}
}

void VCXYPad::invokeMenu(QPoint point)
{
	_app->virtualConsole()->editMenu()->exec(point);
}

void VCXYPad::parseWidgetMenu(int item)
{
	switch (item)
	{
	case KVCMenuEditProperties:
	{
		VCXYPadProperties* p = new VCXYPadProperties(this);
		p->init();

		if (p->exec() == QDialog::Accepted)
		{
			_app->doc()->setModified();
		}

		delete p;
	}
	break;

	case KVCMenuBackgroundFrame:
	{
		if (frameStyle() & KFrameStyleSunken)
		{
			setFrameStyle(NoFrame);
		}
		else
		{
			setFrameStyle(KFrameStyleSunken);
		}
		_app->doc()->setModified();
	}
	break;

	default:
		break;
	}
}

void VCXYPad::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}
	else
	{
		setMouseTracking(false);
		unsetCursor();
		QFrame::mouseReleaseEvent(e);
	}
}

void VCXYPad::mouseMoveEvent(QMouseEvent* e)
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
	{ // the following is NOT done by hasMouse() because that fails if
		// there are child widgets
		if (e->x() > 0 &&  e->y() > 0 &&
		    e->x() < rect().width() &&
		    e->y() < rect().height())
		{
			m_currentXYPosition = mapFromGlobal(m_currentXYPosition);
			m_currentXYPosition.setX(e->x());
			m_currentXYPosition.setY(e->y());
			repaint();

			outputDMX( e->x(), e->y());
			setCursor(Qt::CrossCursor);
		}
		else
		{
			unsetCursor();
		}
		QFrame::mouseMoveEvent(e);
	}
}


void VCXYPad::outputDMX(int x, int y)
{
	int delta;
	int xx;

	QPtrListIterator<XYChannelUnit> xit(*channelsX());
	XYChannelUnit *xyc;

	while ( (xyc = xit.current()) != 0 )
	{
		++xit;
		delta = xyc->hi() - xyc->lo();
		xx = xyc->lo() + int(delta*x/rect().width());
		if (xyc->reverse() == false)
		{
			_app->outputPlugin()->writeChannel(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->outputPlugin()->writeChannel(
				xyc->fixture()->universeAddress() +
				xyc->channel(),
				(t_value) KChannelValueMax - xx);
		}
	}

	QPtrListIterator<XYChannelUnit> yit(*channelsY());
	while ( (xyc = yit.current()) != 0 )
	{
		++yit;
		delta = xyc->hi() - xyc->lo();
		xx = xyc->lo() + int(delta*y/rect().height());
		if (xyc->reverse() == false)
		{
			_app->outputPlugin()->writeChannel(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->outputPlugin()->writeChannel(
				xyc->fixture()->universeAddress() +
				xyc->channel(),
				(t_value) KChannelValueMax - xx);
		}
	}

}


void VCXYPad::customEvent(QCustomEvent* e)
{
	if (e->type() == KVCMenuEvent)
	{
		parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}
}


void VCXYPad::resizeTo(QPoint p)
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


void VCXYPad::moveTo(QPoint p)
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
