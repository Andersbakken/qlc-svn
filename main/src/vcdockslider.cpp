/*
  Q Light Controller
  vcslider.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qfile.h>

#include "vcdockslider.h"
#include "vcframe.h"
#include "virtualconsole.h"
#include "types.h"
#include "bus.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"

#include "../../libs/common/outputplugin.h"
#include "../../libs/common/minmax.h"

extern App* _app;

const int KColorMask          ( 0xff ); // Produces opposite colors with XOR
const int KMoveThreshold      (    5 ); // Pixels

//
// Constructor
//
VCDockSlider::VCDockSlider(QWidget* parent, bool isStatic, const char* name)
  : UI_VCDockSlider(parent, name)
{
  m_busID = KBusIDInvalid;
  m_busMenu = NULL;
  m_static = isStatic;
  m_updateOnly = false;
  m_mode = Normal;
}


//
// Destructor
//
VCDockSlider::~VCDockSlider()
{
}


//
// Init something
//
void VCDockSlider::init()
{
  updateBusMenu();
  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}


//
// Create this slider's contents from list
//
void VCDockSlider::createContents(QPtrList <QString> &list)
{
  QRect rect(0, 0, 60, 200);

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Parent"))
	{
	  VCFrame* parent =
	    _app->virtualConsole()->getFrame(list.next()->toInt());

	  if (parent != NULL)
	    {
	      reparent((QFrame*)parent, 0, QPoint(0, 0), true);
	    }
	}
      else if (*s == QString("X"))
	{
	  rect.setX(list.next()->toInt());
	}
      else if (*s == QString("Y"))
	{
	  rect.setY(list.next()->toInt());
	}
      else if (*s == QString("Width"))
	{
	  rect.setWidth(list.next()->toInt());
	}
      else if (*s == QString("Height"))
	{
	  rect.setHeight(list.next()->toInt());
	}
      else if (*s == QString("Mode"))
	{
	  QString t = *(list.next());
	  if (t == modeString(Normal))
	    {
	      m_mode = Normal;
	    }
	  else if (t == modeString(Speed))
	    {
	      m_mode = Speed;
	    }
	  else
	    {
	      m_mode = Master;
	    }
	}
      else if (*s == QString("Bus"))
	{
	  QString t = *(list.next());
	  t_bus_value value;
	  if (Bus::value(t.toInt(), value))
	    {
	      setBusID(t.toInt());
	    }
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setGeometry(rect);
  updateBusMenu();
}


//
// Save settings to file
//
void VCDockSlider::saveToFile(QFile &file, t_vc_id parentID)
{
  QString s;
  QString t;
  
  // Comment
  s = QString("# Virtual Console Slider Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Slider") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Parent ID
  t.setNum(parentID);
  s = QString("Parent = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

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

  // Mode
  s = QString("Mode = ") + modeString(m_mode) + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus
  t.setNum(m_busID);
  s = QString("Bus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
}


//
// Return mode as a string
//
QString VCDockSlider::modeString(Mode mode)
{
  switch(mode)
    {
    default:
    case Normal:
      return QString("Normal");
    case Speed:
      return QString("Speed");
    case Master:
      return QString("Master");
    }
}

//
// Slider has been moved
//
void VCDockSlider::slotSliderValueChanged(int value)
{
  if (m_mode == Normal)
    {
    }
  else if (m_mode == Speed)
    {
      if (!m_updateOnly)
	{
	  if (!Bus::setValue(m_busID, m_slider->value()))
	    {
	      m_valueLabel->setText("ERROR");
	      return;
	    }
	}

      QString num;
      num.sprintf("%.2fs", ((float) value / (float) KFrequency));
      m_valueLabel->setText(num);
    }
  else if (m_mode == Master)
    {
    }
  else
    {
      m_valueLabel->setText("ERROR");
    }
}


//
// Set behaviour to speed slider and assign a bus
//
bool VCDockSlider::setBusID(t_bus_id id)
{
  t_bus_value value;
  if (Bus::value(id, value))
    {
      m_busID = id;
      m_mode = Speed;

      //
      // Set name label
      //
      QString name = Bus::name(m_busID);
      if (name == QString::null)
	{
	  name.sprintf("%.2d", id);
	}
      m_nameLabel->setText(name);

      m_slider->setValue(value);

      connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
	      this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

      connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
	      this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

      _app->doc()->setModified(true);

      return true;
    }
  else
    {
      return false;
    }

  updateBusMenu();
}


// 
// Bus has been selected from menu
// 
void VCDockSlider::slotBusMenuActivated(int id)
{ 
  setBusID(id);
}


//
// Bus name has been changed by an outside entity
//
void VCDockSlider::slotBusNameChanged(t_bus_id id, const QString &name)
{
  if (id == m_busID)
    {
      m_nameLabel->setText(name);
    }
}


//
// Bus value has been changed by an outside entity
//
void VCDockSlider::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
  if (id == m_busID)
    {
      m_updateOnly = true;
      m_slider->setValue(value);
      m_updateOnly = false;
    }
}

//
//
//
void VCDockSlider::updateBusMenu()
{
  //
  // Create bus menu to bus tool button
  //
  if (m_busMenu) delete m_busMenu;
  m_busMenu = new QPopupMenu();

  QString name;

  for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
    {
      name.sprintf("%.2d: ", i);
      name += Bus::name(i);
      
      m_busMenu->insertItem(name, i);
      if (m_busID == i)
	{
	  m_busMenu->setItemChecked(i, true);
	}
    }

  m_busButton->setPopup(m_busMenu);
  connect(m_busMenu, SIGNAL(activated(int)),
	  this, SLOT(slotBusMenuActivated(int)));
}

//
// Mouse button pressed inside the widget
//
void VCDockSlider::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design && m_static == false)
    {
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
	      _app->doc()->setModified(true);
	    }
	  else
	    {
	      m_origX = e->globalX();
	      m_origY = e->globalY();
	      setCursor(QCursor(SizeAllCursor));
	      _app->doc()->setModified(true);
	    }
	}
      else if (e->button() & RightButton)
	{
	}
    }
  else
    {
      QFrame::mousePressEvent(e);
    }
}


//
// Mouse button released inside the widget
//
void VCDockSlider::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design && m_static == false)
    {
      setCursor(QCursor(ArrowCursor));
      m_resizeMode = false;
      setMouseTracking(false);
    }
  else
    {
      QFrame::mouseReleaseEvent(e);
    }
}


//
// Mouse is moved inside the widget
//
void VCDockSlider::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design && m_static == false)
    {
      if (m_resizeMode == true)
	{
	  QPoint point = mapFromGlobal(QPoint(e->globalX(), e->globalY()));
	  resize(point.x() + 2, point.y() + 2);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  if (moveThreshold(e->globalX(), e->globalY()) == true)
	    {
	      moveTo(e->globalX(), e->globalY());
	    }
	}
    }
  else
    {
      QFrame::mouseMoveEvent(e);
    }
}


void VCDockSlider::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  if (_app->mode() == App::Design && m_static == false)
    {
      QPainter p(this);
      
      QColor c(backgroundColor());
      c.setRgb(c.red() ^ KColorMask,
	       c.green() ^ KColorMask,
	       c.blue() ^ KColorMask);
      
      QBrush b(c, Dense4Pattern);
      p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
      p.drawRect(rect().width() - 10, rect().height() - 10, 10, 10);
    }
}

void VCDockSlider::slotModeChanged()
{
  repaint();

  if (_app->mode() == App::Design)
    {
      m_functionButton->setEnabled(true);
      m_busButton->setEnabled(true);
      m_propertiesButton->setEnabled(true);
    }
  else
    {
      m_functionButton->setEnabled(false);
      m_busButton->setEnabled(false);
      m_propertiesButton->setEnabled(false);
    }
}

//
// Check whether this slider has been moved at least by threshold values
//
bool VCDockSlider::moveThreshold(int x, int y)
{
  int dx = 0;
  int dy = 0;

  dx = abs(m_origX - x);
  dy = abs(m_origY - y);

  if (dx >= KMoveThreshold || dy >= KMoveThreshold)
    return true;
  else
    return false;
}


//
// Center this slider into given coordinates
//
void VCDockSlider::moveTo(int x, int y)
{
  int centerx = rect().width() / 2;
  int centery = rect().height() / 2;

  QPoint point(parentWidget()->mapFromGlobal(QPoint(x - centerx, 
						    y - centery)));

  /* Don't move over right or left */
  if (point.x() < parentWidget()->rect().left())
    {
      point.setX(parentWidget()->rect().left());
    }
  else if (point.x() + rect().width() > parentWidget()->rect().right())
    {
      point.setX(parentWidget()->rect().right() - rect().width());
    }
  
  /* Don't move over top or bottom */
  if (point.y() < parentWidget()->rect().top())
    {
      point.setY(parentWidget()->rect().top());
    }
  else if (point.y() + rect().height() > parentWidget()->rect().bottom())
    {
      point.setY(parentWidget()->rect().bottom() - rect().height());
    }

  move(point);
}
