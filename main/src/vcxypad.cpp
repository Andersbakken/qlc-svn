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

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <qfiledialog.h>
#include <qobjectlist.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qlistview.h>
#include <qcolordialog.h>
#include <qfontdialog.h>
#include <qfiledialog.h>
#include <qinputdialog.h>

#include "common/filehandler.h"
#include "common/qlcimagepreview.h"

#include "app.h"
#include "doc.h"
#include "dmxmap.h"
#include "vcxypad.h"
#include "vcxypadproperties.h"
#include "xychannelunit.h"
#include "fixture.h"
#include "virtualconsole.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent) : VCWidget(parent, "XYPad")
{
	setCaption("");
	setMinimumSize(20, 20);
	
	resize(QPoint(120, 120));
	setFrameStyle(KVCWidgetFrameStyleSunken);
	
	m_xyPosPixmap = QPixmap(QString(PIXMAPS) + QString("/xypad-point.png"));
	
	/* Set initial position to center */
	m_currentXYPosition.setX(width() / 2);
	m_currentXYPosition.setY(height() / 2);
}

VCXYPad::~VCXYPad()
{
	clearChannels();
}

void VCXYPad::scram()
{
	int result = QMessageBox::warning(this,
					  QString(caption()),
					  QString("Remove selected XY pad?"),
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
 * Properties
 *****************************************************************************/

void VCXYPad::editProperties()
{
	VCXYPadProperties prop(this);
	prop.init();
	
	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
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

void VCXYPad::appendChannel(t_axis axis, t_fixture_id fixture,
			    t_channel channel, t_value lowLimit,
			    t_value highLimit, bool reverse)
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

void VCXYPad::removeChannel(t_axis axis, t_fixture_id fixture,
			    t_channel channel)
{
	XYChannelUnit* xyc = NULL;
	QPtrList<XYChannelUnit>* list = NULL;

	if (axis == KAxisX)
		list = &m_channelsX;
	else
		list = &m_channelsY;

	for (int i = 0; i < list->count(); i++)
	{
		xyc = list->at(i);
		if (xyc != NULL &&
		    xyc->fixtureID() == fixture && xyc->channel() == channel)
		{
			list->remove(i);
			delete xyc;
			break;
		}
	}
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

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCXYPad::paintEvent(QPaintEvent* e)
{
	/* Let the parent class draw its stuff first */
	VCWidget::paintEvent(e);

	QPainter p(this);
	QPen pen;

	/* Draw crosshairs to indicate the center position */
	pen.setStyle(DotLine);
	pen.setColor(paletteForegroundColor());
	pen.setWidth(1);
	p.setPen(pen);
	p.drawLine(width() / 2, 0, width() / 2, height());
	p.drawLine(0, height() / 2, width(), height() / 2);

	p.drawPixmap(m_currentXYPosition.x() - (m_xyPosPixmap.width() / 2),
		     m_currentXYPosition.y() - (m_xyPosPixmap.height() / 2),
		     m_xyPosPixmap);
}

void VCXYPad::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		/* Let the parent class take care of moving & resizing */
		VCWidget::mousePressEvent(e);
	}
	else
	{
		setCurrentXYPosition(e->x(), e->y());

		setMouseTracking(true);
		setCursor(Qt::CrossCursor);

		outputDMX(e->x(), e->y());

		VCWidget::mousePressEvent(e);
	}
}

void VCXYPad::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		/* Let the parent class take care of moving & resizing */
		VCWidget::mouseReleaseEvent(e);
	}
	else
	{
		setMouseTracking(false);
		unsetCursor();
		VCWidget::mouseReleaseEvent(e);
	}
}

void VCXYPad::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		/* Let the parent class take care of moving & resizing */
		VCWidget::mouseMoveEvent(e);
	}
	else
	{
		/* The following is not done by hasMouse() because
		   that fails if there are child widgets */
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

		VCWidget::mouseMoveEvent(e);
	}
}

/*****************************************************************************
 * DMX writer
 *****************************************************************************/

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
			_app->dmxMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->dmxMap()->setValue(
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
			_app->dmxMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->dmxMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(),
				(t_value) KChannelValueMax - xx);
		}
	}

}

