/*
  Q Light Controller
  vclabel.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
                                 Stefan Krumm
  
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
#include "functiontree.h"
#include "function.h"
#include "floatingedit.h"
#include "virtualconsole.h"
#include "keybind.h"
#include "bus.h"
#include "settings.h"
#include "configkeys.h"
#include "../../libs/common/minmax.h"

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
const int KMoveThreshold   (    5 ); // Pixels

VCLabel::VCLabel(QWidget* parent) 
  : QLabel(parent, "VCLabel"),
  m_renameEdit ( NULL ),
  m_resizeMode ( false ),
  m_bgPixmap   ( false ),
  m_bgColor    ( false ),
  m_fgColor    ( false )
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

void VCLabel::saveToFile(QFile& file, unsigned int parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Label Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Label") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + text() + QString("\n");
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
  if (m_fgColor)
    {
      t.setNum(qRgb(paletteForegroundColor().red(),
		    paletteForegroundColor().green(),
		    paletteForegroundColor().blue()));
      s = QString("Textcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Background color
  if (m_bgColor)
    {
      t.setNum(qRgb(paletteBackgroundColor().red(),
		    paletteBackgroundColor().green(),
		    paletteBackgroundColor().blue()));
      s = QString("Backgroundcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Background pixmap
  if (m_bgPixmap)
    {
      s = QString("Pixmap = " + m_bgPixmapFileName + QString("\n"));
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
}


void VCLabel::createContents(QPtrList <QString> &list)
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
	  setText(*(list.next()));
	}
      else if (*s == QString("Parent"))
	{
	  VCFrame* parent = 
	    _app->virtualConsole()->getFrame(list.next()->toInt());

	  if (parent != NULL)
	    {
	      reparent((QWidget*) parent, 0, QPoint(0, 0), true);
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
	  m_fgColor = true;
	}
      else if (*s == QString("Backgroundcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteBackgroundColor(qc);
	  m_bgColor = true;
	  m_bgPixmap = false;
	}
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());
	  
	  QPixmap pm(t);
	  if (pm.isNull() == false)
	    {
	      m_bgPixmapFileName = t;
	      setPaletteBackgroundPixmap(pm);
	      m_bgPixmap = true;
	      m_bgColor = false;
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
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
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setGeometry(rect);
}


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
	      m_origX = e->globalX();
	      m_origY = e->globalY();
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
  _app->virtualConsole()->widgetMenu()->exec(point);
}

void VCLabel::parseWidgetMenu(int item)
{
  switch (item)
    {
    case KVCMenuWidgetRename:
      m_renameEdit = new FloatingEdit(parentWidget());
      connect(m_renameEdit, SIGNAL(returnPressed()),
	      this, SLOT(slotRenameReturnPressed()));
      m_renameEdit->setMinimumSize(60, 25);
      m_renameEdit->setGeometry(x() + 3, y() + 3, width() - 6, height() - 6);
      m_renameEdit->setText(text());
      m_renameEdit->setFont(font());
      m_renameEdit->setSelection(0, text().length());
      m_renameEdit->show();
      m_renameEdit->setFocus();
      break;

    case KVCMenuWidgetFont:
      _app->doc()->setModified(true);
      setFont(QFontDialog::getFont(0, font()));
      break;

    case KVCMenuWidgetForegroundColor:
      {
	QColor color;
	color = QColorDialog::getColor(paletteBackgroundColor(), this);
	if (color.isValid())
	  {
	    _app->doc()->setModified(true);
	    setPaletteForegroundColor(color);
	    m_fgColor = true;
	  }
	
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuWidgetForegroundNone:
      {
	if (m_bgColor)
	  {
	    QColor bgcolor = paletteBackgroundColor();
	    unsetPalette();
	    setPaletteBackgroundColor(bgcolor);
	  }
	else
	  {
	    unsetPalette();
	  }

	unsetFont();
	m_fgColor = false;
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuWidgetBackgroundColor:
      {
	QColor newcolor = 
	  QColorDialog::getColor(paletteBackgroundColor(), this);

	if (newcolor.isValid() == true)
	  {
	    setPaletteBackgroundColor(newcolor);
	    _app->doc()->setModified(true);
	  }
      }
      break;
      
      case KVCMenuWidgetBackgroundPixmap:
      {
	QString fileName = 
	  QFileDialog::getOpenFileName(m_bgPixmapFileName, 
				       QString("*.jpg *.png *.xpm *.gif"), 
				       this);
	if (fileName.isEmpty() == false)
	  {
	    m_bgPixmapFileName = fileName;
	    QPixmap pm(fileName);
	    setPaletteBackgroundPixmap(pm);
	    _app->doc()->setModified(true);
	  }
      }
      break;

    case KVCMenuWidgetBackgroundNone:
      {
	if (m_fgColor)
	  {
	    QColor fgcolor = paletteForegroundColor();
	    unsetPalette();
	    setPaletteForegroundColor(fgcolor);
	  }
	else
	  {
	    unsetPalette();
	  }

	m_bgColor = false;
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuWidgetStackRaise:
      raise();
      break;

    case KVCMenuWidgetStackLower:
      lower();
      break;

    case KVCMenuWidgetDrawFrame:
      _app->doc()->setModified(true);
      if (frameStyle() & KFrameStyle)
	setFrameStyle(NoFrame);
      else
	setFrameStyle(KFrameStyle);
      break;

    default:
      break;
    }
}

void VCLabel::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      setCursor(QCursor(ArrowCursor));
      m_resizeMode = false;
      setMouseTracking(false);
    }

}

void VCLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
  parseWidgetMenu(KVCMenuWidgetRename);
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

void VCLabel::slotRenameReturnPressed()
{
  setText(m_renameEdit->text());
  disconnect(m_renameEdit);
  delete m_renameEdit;
  m_renameEdit = NULL;
}

