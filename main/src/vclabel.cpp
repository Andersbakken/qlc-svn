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

extern App* _app;

const int KFrameStyle      ( QFrame::StyledPanel | QFrame::Sunken );
const int KColorMask       ( 0xff ); // Produces opposite colors with XOR
const int KMoveThreshold   (    5 ); // Pixels

const int KMenuTitle             (  0 );
const int KMenuRename            (  1 );
const int KMenuFont              (  2 );
const int KMenuFontColor         (  3 );
const int KMenuBackgroundColor   (  4 );
const int KMenuBackgroundPixmap  (  5 );
const int KMenuBackgroundNone    (  6 );
const int KMenuDrawFrame         (  7 );
const int KMenuRemove            (  8 );
const int KMenuStackRaise        (  9 );
const int KMenuStackLower        ( 10 );

VCLabel::VCLabel(QWidget* parent) : QLabel(parent, "VCLabel")
{
  m_renameEdit = NULL;
  m_background = false;
  m_resizeMode = false;
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
  t.setNum(qRgb(paletteForegroundColor().red(),
                paletteForegroundColor().green(),
		paletteForegroundColor().blue()));
  s = QString("Textcolor = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Background color
  if (m_background)
    {
      t.setNum(qRgb(paletteBackgroundColor().red(),
		    paletteBackgroundColor().green(),
		    paletteBackgroundColor().blue()));
      s = QString("Backgroundcolor = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Font
  s = QString("Font = ") + font().toString() + QString("\n");
  file.writeBlock((const char*) s, s.length());

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
	}
      else if (*s == QString("Backgroundcolor"))
	{
	  QColor qc;
	  qc.setRgb(list.next()->toUInt());
	  setPaletteBackgroundColor(qc);
	  m_background = true;
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
	  QString dir;
	  _app->settings()->get(KEY_SYSTEM_DIR, dir);
	  dir += QString("/") + PIXMAPPATH;

	  //
	  // Background menu
	  //
	  QPopupMenu* bgmenu = new QPopupMenu();
	  bgmenu->insertItem(QPixmap(dir + QString("/color.xpm")),
			     "&Color...", KMenuBackgroundColor);
	  bgmenu->insertItem(QPixmap(dir + QString("/image.xpm")),
			     "&Image...", KMenuBackgroundPixmap);
	  bgmenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
			     "&None", KMenuBackgroundNone);

	  //
	  // Stacking order menu
	  //
	  QPopupMenu* stackmenu = new QPopupMenu;
	  stackmenu->insertItem(QPixmap(dir + QString("/up.xpm")),
				"Bring to Front", KMenuStackRaise);
	  stackmenu->insertItem(QPixmap(dir + QString("/down.xpm")),
				"Send to Back", KMenuStackLower);

	  //
	  // Main context menu
	  //
	  QPopupMenu* menu = new QPopupMenu();
	  menu->insertItem("Label", KMenuTitle);
	  menu->setItemEnabled(KMenuTitle, false);
	  menu->insertSeparator();
	  menu->setCheckable(true);
	  menu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			   "&Rename...", KMenuRename);
	  menu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			   "&Font...", KMenuFont);
	  menu->insertItem(QPixmap(dir + QString("/color.xpm")),
			   "&Text Color...", KMenuFontColor);
	  menu->insertSeparator();
	  menu->insertItem("Background", bgmenu);
	  menu->insertItem("Stacking order", stackmenu);
	  menu->insertItem(QPixmap(dir + QString("/frame.xpm")),
			   "Draw &Frame", KMenuDrawFrame);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			   "Re&move", KMenuRemove);
          
	  if (frameStyle() & KFrameStyle)
	    {
	      menu->setItemChecked(KMenuDrawFrame, true);
	    }

	  connect(bgmenu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));

	  connect(stackmenu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));

	  connect(menu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));

	  menu->exec(mapToGlobal(e->pos()));

	  delete bgmenu;
	  delete stackmenu;
	  delete menu;
	}
    }

}


void VCLabel::slotMenuCallback(int item)
{
  switch (item)
    {
    case KMenuRename:
      m_renameEdit = new FloatingEdit(parentWidget());
      connect(m_renameEdit, SIGNAL(returnPressed()),
	      this, SLOT(slotRenameReturnPressed()));
      m_renameEdit->setMinimumSize(60, 25);
      m_renameEdit->setGeometry(x() + 3, y() + 3, width() - 6, height() - 6);
      m_renameEdit->setText(text());
      m_renameEdit->setSelection(0, text().length());
      m_renameEdit->show();
      m_renameEdit->setFocus();
      break;

    case KMenuRemove:
      _app->doc()->setModified(true);
      delete this;
      break;

    case KMenuFont:
      _app->doc()->setModified(true);
      setFont(QFontDialog::getFont(0, font()));
      adjustSize();
      break;

    case KMenuFontColor:
      {
	QColor color;
	color = QColorDialog::getColor(paletteBackgroundColor(), this);
	if (color.isValid())
	  {
	    _app->doc()->setModified(true);
	    setPaletteForegroundColor(color);
	  }
      }
      break;

    case KMenuBackgroundColor:
      {
	QColor color;
	color = QColorDialog::getColor(paletteBackgroundColor(), this);
	if (color.isValid())
	  {
	    _app->doc()->setModified(true);
	    setPaletteBackgroundColor(color);
	    m_background = true;
	  }
      }
      break;

    case KMenuBackgroundNone:
      _app->doc()->setModified(true);
      setPalette(_app->palette());
      m_background = false;
      break;

    case KMenuStackRaise:
      raise();
      break;

    case KMenuStackLower:
      lower();
      break;

    case KMenuDrawFrame:
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
  slotMenuCallback(KMenuRename);
}


void VCLabel::paintEvent(QPaintEvent* e)
{
  QLabel::paintEvent(e);

  if (_app->mode() == App::Design)
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
	  QPoint pos(e->globalX(), e->globalY());
	  pos = mapFromGlobal(pos);
	  resize(pos.x() + 2, pos.y() + 2);
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton || e->state() & MidButton)
	{
	  if (moveThreshold(e->globalX(), e->globalY()))
	    {
	      _app->doc()->setModified(true);
	      moveTo(e->globalX(), e->globalY());
	    }
	}
      else if (e->state() & LeftButton)
	{
	  if (moveThreshold(e->globalX(), e->globalY()))
	    {
	      _app->doc()->setModified(true);
	      moveTo(e->globalX(), e->globalY());
	      
	      show();
	    }
	}
   }
}

bool VCLabel::moveThreshold(int x, int y)
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

void VCLabel::moveTo(int x, int y)
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


void VCLabel::slotRenameReturnPressed()
{
  setText(m_renameEdit->text());
  disconnect(m_renameEdit);
  delete m_renameEdit;
  m_renameEdit = NULL;
  adjustSize();
}

