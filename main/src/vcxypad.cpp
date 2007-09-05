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

const int KFrameStyle      ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask       ( 0xff );

VCXYPad::VCXYPad(QWidget* parent) : QFrame(parent, "XYPad"),
				    m_xpos             ( 0 ),
				    m_ypos             ( 0 ),
				    m_resizeMode       ( false )
{
}

VCXYPad::~VCXYPad()
{
	m_channelsX.setAutoDelete(true);
	m_channelsX.clear();

	m_channelsY.setAutoDelete(true);
	m_channelsY.clear();
}

void VCXYPad::init()
{
	setMinimumSize(20, 20);

	resize(120, 120);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);

	setBackgroundColor(white);

	m_pixmap = QPixmap(QString(PIXMAPS) + QString("/xypad-point.png"));

	m_currentXYPosition.setX(width() / 2);
	m_currentXYPosition.setY(height() / 2);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
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
		qWarning("XYPad node not found!");
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
}

bool VCXYPad::saveXML(QDomDocument* doc, QDomElement* root)
{
}

/*
  void VCXYPad::saveToFile(QFile& file, t_vc_id parentID)
  {
  QString s;
  QString t;
  XYChannelUnit *xyc = NULL;
  
  // Comment
  s = QString("# Virtual Console XYPad Entry\n");
  file.writeBlock((const char*) s, s.length());
  
  // Entry type
  s = QString("Entry = XYPad") + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Name
  s = QString("Name = ") + caption() + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Parent ID
  t.setNum(parentID);
  s = QString("Parent = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // X Channels and limits
  QPtrListIterator<XYChannelUnit> xit(m_channelsX);
  while ( (xyc = xit.current()) != 0 )
  {
  ++xit;
  
  s.sprintf("ChannelEntryX = %d,%d,%d,%d,",
  xyc->fixtureID(),
  xyc->channel(),
  xyc->lo(),
  xyc->hi());
  
  if (xyc->reverse() == true)
  {
  s += Settings::trueValue();
  }
  else
  {
  s += Settings::falseValue();
  }
  
  s += "\n";
  
  file.writeBlock((const char*) s, s.length());
  }
  
  // Y Channels and limits
  QPtrListIterator<XYChannelUnit> yit(m_channelsY);
  while ( (xyc = yit.current()) != 0 )
  {
  ++yit;
  
  s.sprintf("ChannelEntryY = %d,%d,%d,%d,",
  xyc->fixtureID(),
  xyc->channel(),
  xyc->lo(),
  xyc->hi());
  
  if (xyc->reverse() == true)
  {
  s += Settings::trueValue();
  }
  else
  {
  s += Settings::falseValue();
  }
  
  s += "\n";
  
  file.writeBlock((const char*) s, s.length());
  }
  
  // X
  t.setNum(x());
  s = QString("X = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Y
  t.setNum(y());
  s = QString("Y = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // W
  t.setNum(width());
  s = QString("Width = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // H
  t.setNum(height());
  s = QString("Height = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Palette
  if (ownPalette())
  {
  // Text color
  t.setNum(qRgb(paletteForegroundColor().red(),
  paletteForegroundColor().green(),
  paletteForegroundColor().blue()));
  
  s = QString("Textcolor = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Background color
  t.setNum(qRgb(paletteBackgroundColor().red(),
  paletteBackgroundColor().green(),
  paletteBackgroundColor().blue()));
  
  s = QString("Backgroundcolor = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
  }
  
  // Background pixmap
  if (paletteBackgroundPixmap())
  {
  s = QString("Pixmap = " + iconText() + QString("\n"));
  file.writeBlock((const char*) s, s.length());
  }
  
  // Font
  s = QString("Font = ") + font().toString() + QString("\n");
  file.writeBlock((const char*) s, s.length());
  
  // Frame
  if (frameStyle() & KFrameStyle)
  {
  s = QString("Frame = ") +
  Settings::trueValue() + QString("\n");
  }
  else
  {
  s = QString("Frame = ") +
  Settings::falseValue() + QString("\n");
  }
  file.writeBlock((const char*) s, s.length());
  }
*/

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
		m_currentXYPosition = mapFromGlobal(m_currentXYPosition);
		m_currentXYPosition.setX(e->x());
		m_currentXYPosition.setY(e->y());
		repaint();

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
