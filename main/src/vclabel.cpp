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
#include "vcwidgetbase.h"
#include "vcwidget.h"
#include "bus.h"
#include "settings.h"
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

extern App* _app;

VCLabel::VCLabel(VCWidget* parent) : QLabel(parent, "VCLabel")
{
  m_renameEdit = NULL;
  m_parent = parent;
  m_noBackground = false;
}

VCLabel::~VCLabel()
{
  _app->virtualConsole()->unRegisterKeyReceiver(this);
}

void VCLabel::init()
{
  m_name = QString("Label");
  setText("New label");
  adjustSize();

  connect((VCWidget*) m_parent, SIGNAL(backgroundChanged()),
	  this, SLOT(slotBackgroundChanged()));
}


void VCLabel::slotBackgroundChanged()
{
  if (m_noBackground)
    {
      setTransparent();
    }
}


void VCLabel::setTransparent()
{
  m_noBackground = true;
  
  if (((VCWidget*) m_parent)->bgPixmap() != NULL)
    {
      setBackgroundOrigin(QWidget::ParentOrigin);
      setPaletteBackgroundPixmap(*((VCWidget*) m_parent)->bgPixmap());
      hide();
      show();
    }
  else
    {
      setEraseColor(((VCWidget*) m_parent)->eraseColor());
    }
}


void VCLabel::saveToFile(QFile& file, unsigned int parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Button Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Label") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + VCWidgetBase::name() + QString("\n");
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

  // text color
  t.setNum(qRgb(paletteForegroundColor().red(),
                paletteForegroundColor().green(),
		paletteForegroundColor().blue()));
  s = QString("Textcolor = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // background color
  t.setNum(qRgb(eraseColor().red(),
                eraseColor().green(),
		eraseColor().blue()));
  s = QString("Backgroundcolor = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // background transparency
  if (m_noBackground)
    {
      s = QString("Transparent = ") + Settings::trueValue() + QString("\n");
    }
  else
    {
      s = QString("Transparent = ") + Settings::falseValue() + QString("\n");
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
	  m_name = *(list.next());
	  setText(m_name);
	}
      else if (*s == QString("Parent"))
	{
	  VCWidget* parent = 
	    _app->virtualConsole()->getFrame(list.next()->toInt());

	  if (parent != NULL)
	    {
	      reparent(parent, 0, QPoint(0, 0), true);
	      m_parent = parent;
	      connect((VCWidget*) m_parent, SIGNAL(backgroundChanged()),
		      this, SLOT(slotBackgroundChanged()));
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
	  setEraseColor(qc);
	}
      else if (*s == QString("Transparent"))
	{
	  QString q = *(list.next());
	  if (q == Settings::trueValue())
	    {
	      m_noBackground = true;
	    }
	}
      else if (*s == QString("Font"))
	{
	  QFont f = font();
	  QString q = *(list.next());
	  f.fromString(q);
	  setFont(f);
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setGeometry(rect);

  if (m_noBackground == true)
    {
      setTransparent();
    }
}


void VCLabel::mousePressEvent(QMouseEvent* e)
{
  if (_app->virtualConsole()->isDesignMode() == true)
    {
      if (e->button() & LeftButton)
	{
	  m_origX = e->globalX();
	  m_origY = e->globalY();
	  setCursor(QCursor(SizeAllCursor));
	}

      else if (e->button() & RightButton)
	{
	  QPopupMenu* bgmenu = new QPopupMenu();
	  bgmenu->setCheckable(true);
	  bgmenu->insertItem("Color...", VCWIDGET_MENU_BACKGROUND_COLOR);
	  bgmenu->insertItem("Transparent", VCWIDGET_MENU_BACKGROUND_NONE);
	  if ( m_noBackground == FALSE )
	    {
	      bgmenu->setItemChecked(VCWIDGET_MENU_BACKGROUND_NONE, false);
	      bgmenu->setItemChecked(VCWIDGET_MENU_BACKGROUND_COLOR, true);
	    }
	    else
	    {
	      bgmenu->setItemChecked(VCWIDGET_MENU_BACKGROUND_NONE, true);
	      bgmenu->setItemChecked(VCWIDGET_MENU_BACKGROUND_COLOR, false);
	    }

	  QPopupMenu* menu;
	  menu = new QPopupMenu();
	  menu->insertItem("Modify &text...", VCWIDGET_MENU_RENAME);
	  menu->insertItem("&Font...", VCLABEL_MENU_FONT);
	  menu->insertItem("&Color...", VCLABEL_MENU_F_COLOR);
	  menu->insertItem("&Background", bgmenu, VCWIDGET_MENU_BACKGROUND);
	  menu->insertSeparator();

	  menu->insertItem("Re&move", VCWIDGET_MENU_REMOVE);
          
	  connect(bgmenu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));
	  connect(menu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));

	  menu->exec(mapToGlobal(e->pos()));
	  delete menu;
	}
    }

}

void VCLabel::slotMenuCallback(int item)
{
  switch (item)
    {

    case VCWIDGET_MENU_RENAME:
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

    case VCWIDGET_MENU_REMOVE:
      _app->doc()->setModified(true);
      delete this;
      break;

    case VCLABEL_MENU_FONT:
      _app->doc()->setModified(true);
      setFont( QFontDialog::getFont( 0, font() ) );
      adjustSize();
      break;

    case VCLABEL_MENU_F_COLOR:
      _app->doc()->setModified(true);
      setPaletteForegroundColor( QColorDialog::getColor( paletteForegroundColor(), this ));
      break;

    case VCWIDGET_MENU_BACKGROUND_COLOR:
      _app->doc()->setModified(true);
      setEraseColor( QColorDialog::getColor( eraseColor(), this ));
      m_noBackground = FALSE;
      break;

    case VCWIDGET_MENU_BACKGROUND_NONE:
      _app->doc()->setModified(true);
      m_noBackground = TRUE;
      setTransparent();
      break;

    default:
      break;
    }
}

void VCLabel::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->virtualConsole()->isDesignMode() == true)
    {
      setCursor(QCursor(ArrowCursor));

      setMouseTracking(false);
    }

}

void VCLabel::mouseDoubleClickEvent(QMouseEvent* e)
{
   slotMenuCallback(VCWIDGET_MENU_RENAME);
}


void VCLabel::mouseMoveEvent(QMouseEvent* e)
{
  if (_app->virtualConsole()->isDesignMode() == true)
    {
      if (e->state() & LeftButton)
	{
	  if (moveThreshold(e->globalX(), e->globalY()) == true )
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

  dx = max(m_origX, x) - min(m_origX, x);
  dy = max(m_origY, y) - min(m_origY, y);

  if (dx >= 5 || dy >= 5)
    {
      return true;
    }
  else
    {
      return false;
    }
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
  m_name = m_renameEdit->text();
  setText(m_name);
  disconnect(m_renameEdit);
  delete m_renameEdit;
  m_renameEdit = NULL;
  adjustSize();
}

