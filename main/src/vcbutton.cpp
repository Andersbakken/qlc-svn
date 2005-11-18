/*
  Q Light Controller
  vcbutton.cpp

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

#include "vcbutton.h"
#include "app.h"
#include "doc.h"
#include "functiontree.h"
#include "function.h"
#include "floatingedit.h"
#include "vcbuttonproperties.h"
#include "virtualconsole.h"
#include "keybind.h"
#include "devicemanagerview.h"
#include "settings.h"
#include "configkeys.h"
#include "../../libs/common/minmax.h"

#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qevent.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <limits.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <assert.h>

extern App* _app;

const int KColorMask      ( 0xff ); // Produces opposite colors with XOR
const int KFlashReadyTime (  200 ); // 1 second

VCButton::VCButton(QWidget* parent) : QPushButton(parent, "VCButton")
{
  m_functionID = KNoID;
  m_resizeMode = false;
  m_keyBind = NULL;
}


void VCButton::init()
{
  setToggleButton(true);

  assert(m_keyBind == NULL);
  m_keyBind = new KeyBind();

  connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
  connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

  QToolTip::add(this, "No function");

  setMinimumSize(20, 20);
  resize(30, 30);

  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}


void VCButton::copyFrom(VCButton* button)
{
  attachFunction(button->m_functionID);

  m_resizeMode = false;

  assert(button->keyBind());
  if (m_keyBind)
    {
      disconnect(m_keyBind);
      delete m_keyBind;
    }

  m_keyBind = new KeyBind(button->keyBind());
  connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
  connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

  setToggleButton(true);

  setCaption(button->caption());

  if (button->ownFont())
    {
      setFont(button->font());
    }

  if (button->ownPalette())
    {
      setPaletteForegroundColor(button->paletteForegroundColor());
      setPaletteBackgroundColor(button->paletteBackgroundColor());
    }

  if (button->paletteBackgroundPixmap())
    {
      setPaletteBackgroundPixmap(*button->paletteBackgroundPixmap());
    }

  reparent(button->parentWidget(), 0, QPoint(0, 0), true);

  setGeometry(button->geometry());

  move(button->x() + button->width(), button->y());
}


VCButton::~VCButton()
{
}


void VCButton::setCaption(const QString& text)
{
  setText(text);
  QWidget::setCaption(text);
}


void VCButton::saveToFile(QFile& file, unsigned int parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Button Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Button") + QString("\n");
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

  // Function
  s.sprintf("Function = %d\n", m_functionID);
  file.writeBlock((const char*) s, s.length());

  // Key binding
  assert(m_keyBind);

  s.sprintf("BindKey = %d\n", m_keyBind->key());
  file.writeBlock((const char*) s, s.length());

  s.sprintf("BindMod = %d\n", m_keyBind->mod());
  file.writeBlock((const char*) s, s.length());

  s.sprintf("BindPress = %d\n", m_keyBind->pressAction());
  file.writeBlock((const char*) s, s.length());
  
  s.sprintf("BindRelease = %d\n", m_keyBind->releaseAction());
  file.writeBlock((const char*) s, s.length());
}


void VCButton::createContents(QPtrList <QString> &list)
{
  QRect rect(30, 30, 30, 30);

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
	      reparent((QWidget*)parent, 0, QPoint(0, 0), true);
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
	  // Backwards compatibility for button background color
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
	      setPaletteBackgroundPixmap(pm);
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
	}
      else if (*s == QString("Function"))
	{
	  attachFunction(list.next()->toInt());
	}
      else if (*s == QString("BindKey"))
	{
	  assert(m_keyBind);
	  QString t = *(list.next());
	  m_keyBind->setKey(t.toInt());
	}
      else if (*s == QString("BindMod"))
	{
	  assert(m_keyBind);
	  QString t = *(list.next());
	  m_keyBind->setMod(t.toInt());
	}
      else if (*s == QString("BindPress"))
	{
	  assert(m_keyBind);
	  QString t = *(list.next());
	  m_keyBind->setPressAction((KeyBind::PressAction) t.toInt());
	}
      else if (*s == QString("BindRelease"))
	{
	  assert(m_keyBind);
	  QString t = *(list.next());
	  m_keyBind->setReleaseAction((KeyBind::ReleaseAction) t.toInt());
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setGeometry(rect);
}


void VCButton::setKeyBind(const KeyBind* kb)
{
  assert(kb);

  if (m_keyBind) 
    {
      disconnect(m_keyBind);
      delete m_keyBind;
    }

  m_keyBind = new KeyBind(kb);
  
  connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
  connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}


void VCButton::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      _app->virtualConsole()->setSelectedWidget(this);

      if (m_resizeMode == true)
	{
	  setMouseTracking(false);
	  m_resizeMode = false;
	}

      if (e->button() & MidButton || e->button() & LeftButton)
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
      pressFunction();
    }
}


void VCButton::invokeMenu(QPoint point)
{
  _app->virtualConsole()->editMenu()->exec(point);
}

void VCButton::parseWidgetMenu(int item)
{
  switch (item)
    {
    case KVCMenuEditProperties:
      {
	VCButtonProperties* p = NULL;
	p = new VCButtonProperties(this);
	p->exec();
	delete p;
      }
      break;

    case KVCMenuEditCopy:
      {
	VCButton* bt = NULL;
	bt = new VCButton(parentWidget());
	bt->init();
	bt->copyFrom(this);
	bt->show();

	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
    }
}

void VCButton::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      unsetCursor();
      m_resizeMode = false;
      setMouseTracking(false);
    }
  else
    {
      releaseFunction();
    }
}

void VCButton::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      if (m_resizeMode == true)
	{	  
	  QPoint p(QCursor::pos());
	  resizeTo(mapFromGlobal(p));
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
	  p.setX(p.x() - m_mousePressPoint.x());
	  p.setY(p.y() - m_mousePressPoint.y());
		
	  moveTo(p);
	  _app->doc()->setModified(true);
	}
    }
  else
    {
      QPushButton::mouseMoveEvent(e);
    }
}

void VCButton::resizeTo(QPoint p)
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


void VCButton::moveTo(QPoint p)
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


void VCButton::mouseDoubleClickEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      //slotMenuCallback(KVCMenuWidgetProperties);
    }
  else
    {
      mousePressEvent(e);
    }
}

void VCButton::paintEvent(QPaintEvent* e)
{
  QPushButton::paintEvent(e);

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

void VCButton::slotModeChanged()
{
  repaint();
}

void VCButton::pressFunction()
{
  assert(m_keyBind);

  if (/*m_keyBind->pressAction() == KeyBind::PressNothing || */
      m_functionID == KNoID)
    {
      return;
    }
  /*
  else if (m_keyBind->pressAction() == KeyBind::PressStart)
    {
      Function* f = _app->doc()->function(m_functionID);
      if (f)
	{
	  if (f->engage(static_cast<QObject*> (this)))
	    {
	      setOn(true);
	    }
	}
      else
	{
	  qDebug("Function has been deleted!");
	  attachFunction(KNoID);
	}
    }
  */
  else //if (m_keyBind->pressAction() == KeyBind::PressToggle)
    {
      Function* f = _app->doc()->function(m_functionID);
      if (f)
	{
	  if (isOn())
	    {
	      f->stop();
	      //setOn(false);
	    }
	  else
	    {
	      if (f->engage(static_cast<QObject*> (this)))
		{
		  setOn(true);
		}
	    }
	}
      else
	{
	  qDebug("Function has been deleted!");
	  attachFunction(KNoID);
	}
    }
  /*
  else if (m_keyBind->pressAction() == KeyBind::PressStepForward)
    {
      //
      // TODO: Implement a bus for stepping
      //
    }
  else if (m_keyBind->pressAction() == KeyBind::PressStepBackward)
    {
      //
      // TODO: Implement a bus for stepping
      //
    }
  */
}

void VCButton::releaseFunction()
{

}

void VCButton::attachFunction(t_function_id id)
{
  m_functionID = id;

  Function* f = _app->doc()->function(id);
  if (f)
    {
      QToolTip::add(this, f->name());
    }
  else
    {
      QToolTip::add(this, "No function");
    }

  _app->doc()->setModified(true);
}

void VCButton::customEvent(QCustomEvent* e)
{
  if (e->type() == KFunctionStopEvent && 
      ((FunctionStopEvent*)e)->functionID() == m_functionID)
    {
      setOn(false);
      slotFlashReady();
      QTimer::singleShot(KFlashReadyTime, this, SLOT(slotFlashReady()));
    }
  else if (e->type() == KVCMenuEvent)
    {
      parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
    }
}

void VCButton::slotFlashReady()
{
  //
  // This function is called twice with same XOR mask,
  // thus creating a brief opposite-color-normal-color flash
  //
  QColor c(backgroundColor());
  c.setRgb(c.red() ^ KColorMask,
	   c.green() ^ KColorMask,
	   c.blue() ^ KColorMask);
  setPaletteBackgroundColor(c);
}
