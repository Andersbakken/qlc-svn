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

      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + PIXMAPPATH;
      p.drawPixmap(rect().width() - 10, rect().height() - 10, 
		   QPixmap(dir + "/resize.xpm"), 0, 0);
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
  adjustSize();
}

