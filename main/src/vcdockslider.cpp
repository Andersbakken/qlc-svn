/*
  Q Light Controller
  vcdockslider.cpp

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
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>

#include "vcdockslider.h"
#include "vcdocksliderproperties.h"
#include "vcframe.h"
#include "virtualconsole.h"
#include "types.h"
#include "bus.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "settings.h"
#include "configkeys.h"

#include "../../libs/common/outputplugin.h"
#include "../../libs/common/minmax.h"

extern App* _app;

const int KFrameStyle         ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask          ( 0xff ); // Produces opposite colors with XOR
const int KMoveThreshold      (    5 ); // Pixels

//
// Constructor
//
VCDockSlider::VCDockSlider(QWidget* parent, bool isStatic, const char* name)
  : UI_VCDockSlider(parent, name),
    m_mode           (             Speed ),
    m_busID          ( KBusIDDefaultFade ),
    m_busLowLimit    (                 0 ),
    m_busHighLimit   (                 5 ),
    m_levelLowLimit  (                 0 ),
    m_levelHighLimit (               255 ),
    m_static         (          isStatic ),
    m_updateOnly     (             false )
{
}


//
// Destructor
//
VCDockSlider::~VCDockSlider()
{
  //
  // If mode is submaster, resign couplings before delete
  //
  if (m_mode == Submaster)
    {
      assignSubmasters(false);
      
      // Reset submasters to 100% if they are not occupied
      // anymore by another slider
      _app->resetSubmasters();
    }
}


//
// Init something
//
void VCDockSlider::init()
{
  m_valueLabel->setBackgroundOrigin(ParentOrigin);
  m_slider->setBackgroundOrigin(ParentOrigin);

  setCaption("No Name");
  setMode(Speed);

  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}


void VCDockSlider::setCaption(const QString& text)
{
  m_infoLabel->setText(text);
  UI_VCDockSlider::setCaption(text);
}


void VCDockSlider::setMode(Mode m)
{
  m_mode = m;

  switch (m)
    {
    case Speed:
      {
	t_bus_value value;
	Bus::value(m_busID, value);
	//
	// Set name label
	//
	QString name = Bus::name(m_busID);
	if (name == QString::null)
	  {
	    name.sprintf("%.2d", m_busID + 1);
	  }
	
	m_tapInButton->setText(name);
	
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	
	connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));
	
	m_slider->setRange(m_busLowLimit * KFrequency, 
			   m_busHighLimit * KFrequency);

	m_infoLabel->hide();
	m_tapInButton->show();

	m_time.start();

	slotBusValueChanged(m_busID, value);
      }
      break;

    case Level:
      {
	disconnect(Bus::emitter(),SIGNAL(nameChanged(t_bus_id,const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	
	disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		   this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	m_slider->setRange(m_levelLowLimit, m_levelHighLimit);
	slotSliderValueChanged(m_slider->value());

	m_infoLabel->show();
	m_tapInButton->hide();
      }
      break;

    case Submaster:
      {
	disconnect(Bus::emitter(),SIGNAL(nameChanged(t_bus_id,const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	
	disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		   this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	m_slider->setRange(0, 100);
	slotSliderValueChanged(m_slider->value());

	m_infoLabel->show();
	m_tapInButton->hide();
      }
      break;
    }
}


//
// Assign or resign submasters
//
void VCDockSlider::assignSubmasters(bool assign)
{
  QValueList<t_channel>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
    {
      if (assign == true)
	{
	  _app->assignSubmaster(*it);
	}
      else
	{
	  _app->resignSubmaster(*it);
	}
    }
}


//
// Set level value range
//
void VCDockSlider::setLevelRange(t_value lo, t_value hi)
{
  m_levelLowLimit = lo;
  m_levelHighLimit = hi;
}


//
// Return level slider range
//
void VCDockSlider::levelRange(t_value &lo, t_value &hi)
{
  lo = m_levelLowLimit;
  hi = m_levelHighLimit;
}


//
// Return mode as a string
//
QString VCDockSlider::modeString(Mode mode)
{
  switch(mode)
    {
    default:
    case Speed:
      return QString("Speed");
    case Level:
      return QString("Level");
    case Submaster:
      return QString("Submaster");
    }
}


//
// Set behaviour to speed slider and assign a bus
//
bool VCDockSlider::setBusID(t_bus_id id)
{
  if (id >= KBusIDMin && id < KBusCount)
    {
      m_busID = id;

      return true;
    }
  else
    {
      return false;
    }
}


//
// Set bus value range
//
void VCDockSlider::setBusRange(t_bus_value lo, t_bus_value hi)
{
  m_busLowLimit = lo;
  m_busHighLimit = hi;
}


void VCDockSlider::busRange(t_bus_value &lo, t_bus_value &hi)
{
  lo = m_busLowLimit;
  hi = m_busHighLimit;
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
      else if (*s == QString("Name"))
	{
	  setCaption(*(list.next()));
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
      else if (*s == QString("Textcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteForegroundColor(qc);
	}
      else if (*s == QString("Backgroundcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Color"))
	{
	  // Backwards compatibility for slider background color
	  QString t = *(list.next());
	  int i = t.find(QString(","));
	  int r = t.left(i).toInt();
	  int j = t.find(QString(","), i + 1);
	  int g = t.mid(i+1, j-i-1).toInt();
	  int b = t.mid(j+1).toInt();
	  QColor qc(r, g, b);
	  setPaletteBackgroundColor(qc);
	}
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());
	  
	  QPixmap pm(t);
	  if (pm.isNull() == false)
	    {
	      setIconText(t);
	      unsetPalette();
	      setPaletteBackgroundPixmap(pm);
	      m_valueLabel->setBackgroundOrigin(ParentOrigin);
	      m_slider->setBackgroundOrigin(ParentOrigin);
	    }
	}
      else if (*s == QString("Frame"))
	{
	  if (*(list.next()) == Settings::trueValue())
	    {
	      setFrameStyle(KFrameStyle);
	    }
	  else
	    {
	      setFrameStyle(NoFrame);
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
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
      else if (*s == QString("BusLowLimit"))
	{
	  m_busLowLimit = list.next()->toInt();
	}
      else if (*s == QString("BusHighLimit"))
	{
	  m_busHighLimit = list.next()->toInt();
	}
      else if (*s == QString("Channels"))
	{
	  QString t;

	  unsigned int i = 0;
	  int j = 0;

	  s = list.next();

	  while (i < s->length())
	    {
	      j = s->find(QChar(' '), i, false);
	      if (j == -1)
		{
		  j = s->length();
		}

	      t = s->mid(i, j-i);
	      
	      // Check for duplicates
	      if (m_channels.find(t.toInt()) == m_channels.end())
		{
		  m_channels.append(t.toInt());
		}
	      
	      i = j + 1;
	    }
	}
      else if (*s == QString("LevelLowLimit"))
	{
	  m_levelLowLimit = list.next()->toInt();
	}
      else if (*s == QString("LevelHighLimit"))
	{
	  m_levelHighLimit = list.next()->toInt();
	}
      else if (*s == QString("Mode"))
	{
	  QString t = *list.next();
	  if (t == modeString(Speed))
	    {
	      m_mode = Speed;
	    }
	  else if (t == modeString(Level))
	    {
	      m_mode = Level;
	    }
	  else
	    {
	      m_mode = Submaster;
	    }
	}
      else if (*s == QString("Value"))
	{
	  m_slider->setValue(list.next()->toInt());
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setLevelRange(m_levelLowLimit, m_levelHighLimit);
  setBusRange(m_busLowLimit, m_busHighLimit);
  setGeometry(rect);

  if (m_mode == Submaster)
    {
      assignSubmasters(true);
    }

  setMode(m_mode);
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

  // Name
  s = QString("Name = ") + caption() + QString("\n");
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

  // Text color
  if (ownPalette())
    {
      t.setNum(qRgb(paletteForegroundColor().red(),
		    paletteForegroundColor().green(),
		    paletteForegroundColor().blue()));
      s = QString("Textcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
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

  // Frame
  if (frameStyle() & KFrameStyle)
    {
      s = QString("Frame = ") + Settings::trueValue() + QString("\n");
    }
  else
    {
      s = QString("Frame = ") + Settings::falseValue() + QString("\n");
    }
  file.writeBlock((const char*) s, s.length());

  // Font
  s = QString("Font = ") + font().toString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus
  t.setNum(m_busID);
  s = QString("Bus = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus Lo
  t.setNum(m_busLowLimit);
  s = QString("BusLowLimit = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Bus Hi
  t.setNum(m_busHighLimit);
  s = QString("BusHighLimit = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Channels
  if (m_channels.count())
    {
      s = QString("Channels = ");
      QValueList<t_channel>::Iterator it;
      for (it = m_channels.begin(); it != m_channels.end(); ++it)
	{
	  t.sprintf("%.3d", *it);
	  s += t + QString(" ");
	}

      s += QString("\n");
      file.writeBlock((const char*) s, s.length());      
    }

  // Level Lo
  t.setNum(m_levelLowLimit);
  s = QString("LevelLowLimit = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Level Hi
  t.setNum(m_levelHighLimit);
  s = QString("LevelHighLimit = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Mode (must be written after bus & channel settings)
  s = QString("Mode = ") + modeString(m_mode) + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Value
  t.setNum(m_slider->value());
  s = QString("Value = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());
}


//
// Slider has been moved
//
void VCDockSlider::slotSliderValueChanged(int value)
{
  if (m_mode == Speed)
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
  else if (m_mode == Level)
    {
      QString num;
      num.sprintf("%.3d", m_levelHighLimit - value + m_levelLowLimit);
      m_valueLabel->setText(num);

      QValueList<t_channel>::iterator it;
      for(it = m_channels.begin(); it != m_channels.end(); ++it)
	{
	  _app->setValue(*it, m_levelHighLimit - value + m_levelLowLimit);
	}
    }
  else
    {
      QString num;
      num.sprintf("%d%%", 100 - value);
      m_valueLabel->setText(num);

      QValueList<t_channel>::iterator it;
      for(it = m_channels.begin(); it != m_channels.end(); ++it)
	{
	  _app->setSubmasterValue(*it, 100 - value);
	}
    }
}


//
// Bus name has been changed by an outside entity
//
void VCDockSlider::slotBusNameChanged(t_bus_id id, const QString &name)
{
  if (id == m_busID)
    {
      m_tapInButton->setText(name);
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
// Calculate speed from button tap intervals
//
void VCDockSlider::slotTapInButtonClicked()
{
  int t;
  t = m_time.elapsed();
  m_slider->setValue(static_cast<int> (t * 0.001 * KFrequency));
  m_time.restart();
}


//
// Mouse button pressed inside the widget
//
void VCDockSlider::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      _app->virtualConsole()->setSelectedWidget(this);

      if (m_resizeMode == true)
	{
	  setMouseTracking(false);
	  m_resizeMode = false;
	}

      if ((e->button() & LeftButton || e->button() & MidButton) &&
	  m_static == false)
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
	  invokeMenu(mapToGlobal(e->pos()));
	}
    }
  else
    {
      QFrame::mousePressEvent(e);
    }
}


//
// QSlider passes this event thru, so grab it also
//
void VCDockSlider::contextMenuEvent(QContextMenuEvent* e)
{
  if (_app->mode() == App::Design && m_static == false)
    {
      invokeMenu(mapToGlobal(e->pos()));
    }
}


//
// Invoke a menu at given point
//
void VCDockSlider::invokeMenu(QPoint point)
{
  if (m_static)
    {
      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + PIXMAPPATH;

      QPopupMenu menu;
      menu.insertItem(QPixmap(dir + "/settings.xpm"),
		     QString("&Properties..."), KVCMenuEditProperties);

      if (menu.exec(point) == KVCMenuEditProperties)
	{
	  bool ok = false;
	  QString current;
	  current.sprintf("%d-%d", m_busLowLimit, m_busHighLimit);

	  QString text = 
	    QInputDialog::getText(KApplicationNameShort,
				  "Slider value range (e.g. 0-10) in seconds:",
				  QLineEdit::Normal, current, &ok, this);
	  
	  if (ok && !text.isEmpty())
	    {
	      int dash = text.find('-');
	      QString min = text.left(dash);
	      QString max = text.mid(dash + 1);
	      
	      if (min.toInt() >= max.toInt())
		{
		  QMessageBox::warning(this, KApplicationNameShort,
       "Minimum value cannot be bigger than or equal to the maximum value");
		}
	      else
		{
		  setBusRange(min.toInt(), max.toInt());
		  setMode(Speed);

		  if (m_busID == KBusIDDefaultFade)
		    {
		      _app->settings()->set(KEY_DEFAULT_FADE_MIN, min.toInt());
		      _app->settings()->set(KEY_DEFAULT_FADE_MAX, max.toInt());
		    }
		  else
		    {
		      _app->settings()->set(KEY_DEFAULT_HOLD_MIN, min.toInt());
		      _app->settings()->set(KEY_DEFAULT_HOLD_MAX, max.toInt());
		    }
		}
	    }
	}
    }
  else
    {
      _app->virtualConsole()->editMenu()->exec(point);
    }

}


void VCDockSlider::parseWidgetMenu(int item)
{
  switch (item)
    {
    case KVCMenuEditProperties:
      {
	VCDockSliderProperties* sp = new VCDockSliderProperties(this);
	sp->init();
	if (sp->exec() == QDialog::Accepted)
	  {
	    _app->doc()->setModified(true);
	  }
	
	delete sp;
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
	
	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
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


void VCDockSlider::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

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


void VCDockSlider::slotModeChanged()
{
  repaint();
}


void VCDockSlider::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design && m_static == false)
    {
      if (m_resizeMode == true)
	{	  
	  QPoint p(QCursor::pos());
	  resizeTo(mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  QPoint p(QCursor::pos());
	  moveTo(parentWidget()->mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
    }
  else
    {
      QFrame::mouseMoveEvent(e);
    }
}

void VCDockSlider::customEvent(QCustomEvent* e)
{
  if (e->type() == KVCMenuEvent)
    {
      parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
    }
}

void VCDockSlider::resizeTo(QPoint p)
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


void VCDockSlider::moveTo(QPoint p)
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
