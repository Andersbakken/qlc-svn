/*
  Q Light Controller
  vcframe.cpp
  
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

#include "vcframe.h"
#include "vcbutton.h"
#include "vclabel.h"
#include "vcdockslider.h"
#include "floatingedit.h"
#include "app.h"
#include "doc.h"
#include "virtualconsole.h"
#include "settings.h"
#include "../../libs/common/minmax.h"
#include "configkeys.h"

#include <qcursor.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <stdio.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qobjcoll.h>
#include <qmessagebox.h>
#include <qpainter.h>

extern App* _app;

t_vc_id VCFrame::s_nextVCID = KVCIDMin;

const int KColorMask            ( 0xff );
const int KMoveThreshold        (    5 ); // Pixels

const int KMenuTitle            ( 0  );
const int KMenuRename           ( 1  );
const int KMenuBackgroundNone   ( 2  );
const int KMenuBackgroundColor  ( 3  );
const int KMenuBackgroundPixmap ( 4  );
const int KMenuRemove           ( 5  );
const int KMenuCopy             ( 6  );

const int KMenuAddButton        ( 7  );
const int KMenuAddSlider        ( 8  );
const int KMenuAddFrame         ( 9  );
const int KMenuAddLabel         ( 10 );

const int KMenuStack            ( 11 );
const int KMenuStackRaise       ( 12 );
const int KMenuStackLower       ( 13 );

VCFrame::VCFrame(QWidget* parent) : QFrame(parent)
{
  setMinimumSize(20, 20);

  m_origX = 0;
  m_origY = 0;
  m_id = s_nextVCID++;

  m_xpos = 0;
  m_ypos = 0;

  m_bgPixmapFileName = QString::null;

  m_bgPixmap = NULL;
  m_bgColor = NULL;

  m_bottomFrame = false;

  m_resizeMode = false;

  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
}

VCFrame::~VCFrame()
{
}

void VCFrame::init()
{
  resize(120, 120);
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

void VCFrame::setBottomFrame(bool set) 
{ 
  m_bottomFrame = set;
  if (set)
    {
      setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }
  else
    {
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }
}

void VCFrame::setID(t_vc_id newid)
{
  m_id = newid;
  if (newid >= s_nextVCID)
    {
      // Set the biggest id number + 1 to be the next free id
      s_nextVCID = newid + 1;
    }
}

void VCFrame::saveFramesToFile(QFile& file, t_vc_id parentID)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Frame Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Frame") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // ID
  t.setNum(id());
  s = QString("ID = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Parent ID
  if (parentID != 0)
    {
      t.setNum(parentID);
      s = QString("Parent = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  if (m_bottomFrame == false)
    {
      // Geometry
      t.setNum(x());
      s = QString("X = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      t.setNum(y());
      s = QString("Y = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      t.setNum(width());
      s = QString("Width = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
      
      t.setNum(height());
      s = QString("Height = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  // Pixmap or color
  if (m_bgColor != NULL)
    {
      t.setNum(m_bgColor->red());
      s = QString("Color = " + t + QString(","));
      t.setNum(m_bgColor->green());
      s += t + QString(",");
      t.setNum(m_bgColor->blue());
      s += t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
  else if (m_bgPixmap != NULL)
    {
      s = QString("Pixmap = ") + m_bgPixmapFileName + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }

  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      for (; it.current() != NULL; ++it)
	{
	  if (QString(it.current()->className()) == QString("VCFrame"))
	    {
	      VCFrame* w = (VCFrame*) it.current();
	      w->saveFramesToFile(file, id());
	    }
	}
    }
}

void VCFrame::saveChildrenToFile(QFile& file)
{
  if (children() != NULL)
    {
      QObjectList* ol = (QObjectList*) children();
      QObjectListIt it(*ol);
      
      // Child frames
      while(it.current())
	{
	  if (QString(it.current()->className()) == QString("VCButton"))
	    {
	      ((VCButton*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCLabel"))
	    {
	      ((VCLabel*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className())==QString("VCDockSlider"))
	    {
	      ((VCDockSlider*) it.current())->saveToFile(file, id());
	    }
	  else if (QString(it.current()->className()) == QString("VCFrame"))
	    {
	      ((VCFrame*) it.current())->saveChildrenToFile(file);
	    }

	  ++it;
	}
    }
}

void VCFrame::createContents(QPtrList <QString> &list)
{
  QRect rect(30, 30, 30, 30);

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("ID"))
	{
	  setID(list.next()->toInt());
	}
      else if (*s == QString("Parent"))
	{
	  if (m_bottomFrame == false)
	    {
	      VCFrame* parent = 
		_app->virtualConsole()->getFrame(list.next()->toInt());

	      if (parent != NULL)
		{
		  reparent((QWidget*) parent, 0, QPoint(0, 0), true);
		}
	    }
	  else
	    {
	      list.next();
	    }
	}
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());

	  m_bgPixmap = new QPixmap(t);
	  if (m_bgPixmap->isNull() == false)
	    {
	      m_bgPixmapFileName = t;
	      setBackgroundPixmap(*m_bgPixmap);
	    }
	  else
	    {
	      delete m_bgPixmap;
	    }
	}
      else if (*s == QString("Color"))
	{
	  QString t = *(list.next());
	  int i = t.find(QString(","));
	  int r = t.left(i).toInt();
	  int j = t.find(QString(","), i + 1);
	  int g = t.mid(i+1, j-i-1).toInt();
	  int b = t.mid(j+1).toInt();
	  m_bgColor = new QColor(r, g, b);
	  setBackgroundColor(*m_bgColor);

	  emit backgroundChanged();
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
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  if (m_bottomFrame == false)
    {
      setGeometry(rect);
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }
  else
    {
      setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }
}


void VCFrame::paintEvent(QPaintEvent* e)
{
  QFrame::paintEvent(e);

  if (_app->mode() == App::Design && m_bottomFrame == false)
    {
      QPainter p(this);

      QColor c(backgroundColor());
      c.setRgb(c.red() ^ KColorMask,
	       c.green() ^ KColorMask,
	       c.blue() ^ KColorMask);

      QBrush b(Dense4Pattern);
      p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
      p.drawRect(rect().width() - 10, rect().height() - 10, 10, 10);
    }
}

void VCFrame::slotModeChanged()
{
  repaint();
}

void VCFrame::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      if (m_resizeMode == true && m_bottomFrame == false)
	{
	  setMouseTracking(false);
	  m_resizeMode = false;
	}
      
      if ((e->button() & LeftButton || e->button() & MidButton)
	  && m_bottomFrame == false)
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
	  QCursor cursor;
	  QPoint pt;
	  QPoint point;
	  pt = cursor.pos();
	  point = mapFromGlobal(pt);

	  m_xpos = point.x();
	  m_ypos = point.y();

	  QString dir;
	  _app->settings()->get(KEY_SYSTEM_DIR, dir);
	  dir += QString("/") + PIXMAPPATH;

	  //
	  // BG Menu
	  //
	  QPopupMenu* bgmenu = new QPopupMenu();
	  bgmenu->setCheckable(true);
	  bgmenu->insertItem(QPixmap(dir + QString("/color.xpm")),
			     "&Color...", KMenuBackgroundColor);
	  bgmenu->insertItem(QPixmap(dir + QString("/image.xpm")),
			     "&Image...", KMenuBackgroundPixmap);
	  bgmenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
			     "&None", KMenuBackgroundNone);
	  if (m_bgPixmap != NULL)
	    {
	      bgmenu->setItemChecked(KMenuBackgroundPixmap, true);
	    }
	  else if (m_bgColor != NULL)
	    {
	      bgmenu->setItemChecked(KMenuBackgroundColor, true);
	    }
	  else
	    {
	      bgmenu->setItemChecked(KMenuBackgroundNone, true);
	    }
	  
	  //
	  // Stacking order menu
	  //
	  QPopupMenu* stackmenu = new QPopupMenu;
	  stackmenu->insertItem(QPixmap(dir + QString("/up.xpm")),
				"Bring to Front", KMenuStackRaise);
	  stackmenu->insertItem(QPixmap(dir + QString("/down.xpm")),
				"Send to Back", KMenuStackLower);

	  //
	  // Add menu
	  //
	  QPopupMenu* addMenu = new QPopupMenu();
	  addMenu->insertItem(QPixmap(dir + "/button.xpm"), 
			      "&Button", KMenuAddButton);
	  addMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			      "&Slider", KMenuAddSlider);
	  addMenu->insertItem(QPixmap(dir + "/frame.xpm"), 
			      "&Frame", KMenuAddFrame);
	  addMenu->insertItem(QPixmap(dir + "/rename.xpm"), 
			      "&Label", KMenuAddLabel);

	  QPopupMenu* menu = new QPopupMenu();
	  menu->insertItem("Frame", KMenuTitle);
	  menu->setItemEnabled(KMenuTitle, false);
	  menu->insertSeparator();
	  menu->insertItem("Add", addMenu);
	  menu->insertItem("Background", bgmenu);
	  menu->insertItem("Stacking order", stackmenu, KMenuStack);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			   "Remove", KMenuRemove);

	  if (m_bottomFrame == true)
	    {
	      menu->setItemEnabled(KMenuRemove, false);
	      menu->setItemEnabled(KMenuStack, false);
	    }

	  connect(bgmenu, SIGNAL(activated(int)), 
		  this, SLOT(slotMenuCallback(int)));

	  connect(stackmenu, SIGNAL(activated(int)), 
		  this, SLOT(slotMenuCallback(int)));

	  connect(addMenu, SIGNAL(activated(int)), 
		  this, SLOT(slotMenuCallback(int)));

	  connect(menu, SIGNAL(activated(int)), 
		  this, SLOT(slotMenuCallback(int)));

	  menu->exec(mapToGlobal(e->pos()));

	  delete bgmenu;
	  delete stackmenu;
	  delete addMenu;
	  delete menu;
	}
    }
  else
    {
      QFrame::mousePressEvent(e);
    }
}

void VCFrame::slotMenuCallback(int item)
{
  switch (item)
    {
    case KMenuRemove:
      {
	QString message ("Remove frame and its contents permanently?");
	switch(QMessageBox::warning(this, "Frame", message,
				    "&Yes", "&No", 0, 2))
	  {
	  case 0:
	    {
	      _app->doc()->setModified(true);
	      delete this;
	    }
	    break;

	  case 2:
	    break;
	  }
      }
      break;

    case KMenuBackgroundNone:
      {
	if (m_bgPixmap != NULL)
	  {
	    m_bgPixmapFileName = QString::null;
	    delete m_bgPixmap;
	    m_bgPixmap = NULL;
	  }

	if (m_bgColor != NULL)
	  {
	    delete m_bgColor;
	    m_bgColor = NULL;
	  }

	//setPalette(_app->palette());
	setBackgroundMode(PaletteBackground);

	emit backgroundChanged();

	_app->doc()->setModified(true);
      }
      break;

    case KMenuBackgroundColor:
      {
	QColor currentcolor;
	if (m_bgColor != NULL)
	  {
	    currentcolor = *m_bgColor;
	  }

	QColor newcolor = QColorDialog::getColor(currentcolor, this);
	
	if (newcolor.isValid() == true)
	  {
	    if (m_bgPixmap != NULL)
	      {
		m_bgPixmapFileName = QString::null;
		delete m_bgPixmap;
		m_bgPixmap = NULL;
	      }
	    m_bgColor = new QColor(newcolor);
	    setBackgroundColor(*m_bgColor);

	    emit backgroundChanged();

	    _app->doc()->setModified(true);
	  }
      }
      break;

    case KMenuBackgroundPixmap:
      {
	QString fileName = 
	  QFileDialog::getOpenFileName(m_bgPixmapFileName, 
				       QString("*.jpg *.png *.xpm *.gif"),
				       this);
	if (fileName.isEmpty() == false)
	  {
	    if (m_bgColor != NULL)
	      {
		delete m_bgColor;
		m_bgColor = NULL;
	      }

	    m_bgPixmapFileName = fileName;
	    m_bgPixmap = new QPixmap(fileName);
	    setBackgroundPixmap(*m_bgPixmap);

	    emit backgroundChanged();
	    
	    _app->doc()->setModified(true);
	  }
      }
      break;

    case KMenuStackRaise:
      raise();
      break;

    case KMenuStackLower:
      lower();
      break;

    case KMenuAddButton:
      {
	VCButton* b;
	b = new VCButton(this);
	b->init();
	b->setGeometry(m_xpos, m_ypos, 30, 30);
	b->show();
	_app->doc()->setModified(true);
      }
      break;

    case KMenuAddSlider:
      {
	VCDockSlider* vcd = new VCDockSlider(this);
	vcd->setBusID(KBusIDDefaultFade);
	vcd->init();
	vcd->setGeometry(m_xpos, m_ypos, 60, 200);
	vcd->show();
	_app->doc()->setModified(true);
      }
      break;
      
    case KMenuAddFrame:
      {
	VCFrame* w = new VCFrame(this);
	w->setGeometry(m_xpos, m_ypos, 120, 120);
	w->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	w->show();
	_app->doc()->setModified(true);
      }
      break;

     case KMenuAddLabel:
      {
	VCLabel* p = NULL;
	p = new VCLabel(this);
	p->move(m_xpos, m_ypos);
	p->init();
	p->setText("New label");
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
    }
}

void VCFrame::mouseReleaseEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
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

void VCFrame::mouseMoveEvent(QMouseEvent* e)
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
      QFrame::mouseMoveEvent(e);
    }
}

void VCFrame::resizeTo(QPoint p)
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


void VCFrame::moveTo(QPoint p)
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
