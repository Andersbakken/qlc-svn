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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "vcxypadproperties.h"
#include "virtualconsole.h"
#include "xychannelunit.h"
#include "outputmap.h"
#include "vcxypad.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent) : VCWidget(parent)
{
	/* Set the class name "VCXYPad" as the object name as well */
	setObjectName(VCXYPad::staticMetaObject.className());

	setFrameStyle(KVCFrameStyleSunken);
	setCaption(QString::null);
	setMinimumSize(20, 20);

	resize(QPoint(120, 120));

	m_xyPosPixmap = QPixmap(":/xypad-point.png");

	/* Set initial position to center */
	m_currentXYPosition.setX(width() / 2);
	m_currentXYPosition.setY(height() / 2);
}

VCXYPad::~VCXYPad()
{
	clearChannels();
}

void VCXYPad::slotDelete()
{
	QString msg;

	msg = "Do you wish to delete this XY-Pad?\n" + caption();
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
 * Properties
 *****************************************************************************/

void VCXYPad::slotProperties()
{
	VCXYPadProperties prop(_app, this);
	if (prop.exec() == QDialog::Accepted)
		_app->doc()->setModified();
}

/*****************************************************************************
 * Channels
 *****************************************************************************/
void VCXYPad::clearChannels()
{
	while (m_channelsX.isEmpty() == false)
		delete m_channelsX.takeFirst();

	while (m_channelsY.isEmpty() == false)
		delete m_channelsY.takeFirst();
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
	QList <XYChannelUnit*>* list;

	if (axis == KAxisX)
		list = &m_channelsX;
	else
		list = &m_channelsY;

	QMutableListIterator <XYChannelUnit*> it(*list);
	while (it.hasNext() == true)
	{
		XYChannelUnit* xyc = it.next();
		if (xyc != NULL && xyc->fixtureID() == fixture &&
		    xyc->channel() == channel)
		{
			it.remove();
			delete xyc;
			break;
		}
	}
}

XYChannelUnit* VCXYPad::channel(t_axis axis, t_fixture_id fixture,
				t_channel channel)
{
	XYChannelUnit* xyc;
	QList <XYChannelUnit*>* list;

	if (axis == KAxisX)
		list = &m_channelsX;
	else
		list = &m_channelsY;

	QListIterator <XYChannelUnit*> it(*list);
	while (it.hasNext() == true)
	{
		xyc = it.next();
		if (xyc->fixtureID() == fixture && xyc->channel() == channel)
			return xyc;
	}

	return NULL;
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
		qDebug() << "XY Pad node not found!";
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
		qDebug() << "XY Pad node not found!";
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
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
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
			qDebug() << "Unknown XY Pad tag:" << tag.tagName();
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
	QLCFile::saveXMLWindowState(doc, &root, this);

	/* X Channels */
	QListIterator <XYChannelUnit*> xit(m_channelsX);
	while (xit.hasNext() == true)
	{
		xyc = xit.next();

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
	}

	/* Y Channels */
	QListIterator <XYChannelUnit*> yit(m_channelsY);
	while (yit.hasNext() == true)
	{
		xyc = yit.next();

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
	pen.setStyle(Qt::DotLine);
	pen.setColor(palette().color(QPalette::WindowText));
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

	QListIterator <XYChannelUnit*> xit(*channelsX());
	while (xit.hasNext() == true)
	{
		XYChannelUnit* xyc = xit.next();

		delta = xyc->hi() - xyc->lo();
		xx = xyc->lo() + int((delta * x) / rect().width());

		if (xyc->reverse() == false)
		{
			_app->outputMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->outputMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(),
				(t_value) KChannelValueMax - xx);
		}
	}

	QListIterator <XYChannelUnit*> yit(*channelsY());
	while (yit.hasNext() == true)
	{
		XYChannelUnit* xyc = yit.next();

		delta = xyc->hi() - xyc->lo();
		xx = xyc->lo() + int((delta * y) / rect().height());

		if (xyc->reverse() == false)
		{
			_app->outputMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(), (t_value) xx);
		}
		else
		{
			_app->outputMap()->setValue(
				xyc->fixture()->universeAddress() +
				xyc->channel(),
				(t_value) KChannelValueMax - xx);
		}
	}

}

