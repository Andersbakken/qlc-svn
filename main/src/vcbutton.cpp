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
#include <qpopupmenu.h>
#include <qfile.h>
#include <limits.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qbrush.h>
#include <assert.h>

extern App* _app;

const int KColorMask      ( 0xff ); // Produces opposite colors with XOR
const int KFlashReadyTime ( 1000 ); // 1 second
const int KMoveThreshold  (    5 ); // Pixels

const int KMenuTitle            (  0 );
const int KMenuRename           (  1 );
const int KMenuProperties       (  2 );
const int KMenuBackgroundColor  (  3 );
const int KMenuBackgroundPixmap (  4 );
const int KMenuBackgroundNone   (  5 );
const int KMenuAttach           (  6 );
const int KMenuDetach           (  7 );
const int KMenuCopy             (  8 );
const int KMenuRemove           (  9 );
const int KMenuStackRaise       ( 10 );
const int KMenuStackLower       ( 11 );

VCButton::VCButton(QWidget* parent) : QPushButton(parent, "VCButton")
{
  m_functionID = KNoID;
  m_resizeMode = false;
  m_renameEdit = NULL;
  m_bgColor = NULL;
  m_keyBind = NULL;
  m_bgPixmapFileName = QString::null;
  m_bgPixmap = NULL;
}


void VCButton::init()
{
  setToggleButton(true);

  if (m_keyBind) delete m_keyBind;
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
  m_renameEdit = NULL;

  assert(button->keyBind());
  if (m_keyBind) delete m_keyBind;
  m_keyBind = new KeyBind(button->keyBind());
  connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
  connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));

  setToggleButton(true);

  setText(button->text());

  m_bgColor = new QColor(*button->bgColor());
  if (m_bgColor != NULL)
    setPaletteBackgroundColor(*m_bgColor);

  reparent(button->parentWidget(), 0, QPoint(0, 0), true);

  setGeometry(button->geometry());

  move(button->x() + button->width(), button->y());
}


VCButton::~VCButton()
{
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
	  setText(*(list.next()));
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
      else if (*s == QString("Pixmap"))
	{
	  QString t;
	  t = *(list.next());

	  m_bgPixmap = new QPixmap(t);
	  if (m_bgPixmap->isNull() == false)
	    {
	      m_bgPixmapFileName = t;
	      setPixmap(*m_bgPixmap);
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
	  setPaletteBackgroundColor(*m_bgColor);

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

  if (m_keyBind) delete m_keyBind;
  m_keyBind = new KeyBind(kb);
  
  connect(m_keyBind, SIGNAL(pressed()), this, SLOT(pressFunction()));
  connect(m_keyBind, SIGNAL(released()), this, SLOT(releaseFunction()));
}


void VCButton::mousePressEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
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
	  // Main context menu
	  //
	  QPopupMenu* menu;
	  menu = new QPopupMenu;
	  menu->insertItem("Button", KMenuTitle);
	  menu->setItemEnabled(KMenuTitle, false);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			   "&Rename...", KMenuRename);
	  menu->insertItem(QPixmap(dir + QString("/settings.xpm")),
			   "&Properties...", KMenuProperties);
	  menu->insertSeparator();
	  menu->insertItem("Background", bgmenu);
	  menu->insertItem("Stacking order", stackmenu);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/attach.xpm")),
			   "&Attach function...", KMenuAttach);
	  menu->insertItem(QPixmap(dir + QString("/detach.xpm")),
			   "&Detach current function", KMenuDetach);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/editcopy.xpm")),
			   "Create copy", KMenuCopy);
	  menu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			   "Re&move", KMenuRemove);

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
  else
    {
      pressFunction();
    }
}


void VCButton::slotMenuCallback(int item)
{
  switch (item)
    {
    case KMenuRename:
      {
	m_renameEdit = new FloatingEdit(parentWidget());
	connect(m_renameEdit, SIGNAL(returnPressed()),
		this, SLOT(slotRenameReturnPressed()));
	m_renameEdit->setMinimumSize(60, 25);
	m_renameEdit->setGeometry(x() + 3, y() + 3, width() - 6, height() - 6);
	m_renameEdit->setText(text());
	m_renameEdit->setSelection(0, text().length());
	m_renameEdit->show();
	m_renameEdit->setFocus();
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
	    setPaletteBackgroundColor(*m_bgColor);
	    show();

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
	    setPixmap(*m_bgPixmap);
            show();
	    _app->doc()->setModified(true);
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

	setPalette(_app->palette());

	_app->doc()->setModified(true);
      }
      break;

    case KMenuStackRaise:
      raise();
      break;

    case KMenuStackLower:
      lower();
      break;

    case KMenuProperties:
      {
	VCButtonProperties* p = NULL;
	p = new VCButtonProperties(this);
	p->exec();
	delete p;
      }
      break;

    case KMenuRemove:
      {
	if (QMessageBox::warning(this, "Remove Button", "Are you sure?",
				 QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	  {
	    _app->doc()->setModified(true);
	    delete this;
	  }
      }
      break;

    case KMenuAttach:
      {
	FunctionTree* ft = new FunctionTree(this);
	if (ft->exec() == QDialog::Accepted)
	  {
	    attachFunction(ft->functionID());
	  }
	delete ft;
      }
      break;

    case KMenuDetach:
      {
	attachFunction(KNoID);
      }
      break;

    case KMenuCopy:
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
      setCursor(QCursor(ArrowCursor));
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
	  QPoint p(QCursor::pos());
	  moveTo(parentWidget()->mapFromGlobal(p));
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
      slotMenuCallback(KMenuProperties);
    }
  else
    {
      mousePressEvent(e);
    }
}

void VCButton::paintEvent(QPaintEvent* e)
{
  QPushButton::paintEvent(e);

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

void VCButton::slotModeChanged()
{
  repaint();
}

void VCButton::pressFunction()
{
  assert(m_keyBind);

  qDebug("pressed");

  if (m_keyBind->pressAction() == KeyBind::PressNothing || 
      m_functionID == KNoID)
    {
      return;
    }
  else if (m_keyBind->pressAction() == KeyBind::PressStart)
    {
      Function* f = _app->doc()->function(m_functionID);
      if (f)
	{
	  qDebug("engage");
	  f->engage(static_cast<QObject*> (this));
	  setOn(true);
	}
      else
	{
	  qDebug("Function has been deleted!");
	  attachFunction(KNoID);
	}
    }
  else if (m_keyBind->pressAction() == KeyBind::PressToggle)
    {
      Function* f = _app->doc()->function(m_functionID);
      if (f)
	{
	  if (isOn())
	    {
	      qDebug("stop");
	      f->stop();
	      setOn(false);
	    }
	  else
	    {
	      f->engage(static_cast<QObject*> (this));
	      setOn(true);
	    }
	}
      else
	{
	  qDebug("Function has been deleted!");
	  attachFunction(KNoID);
	}
    }
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
}

void VCButton::releaseFunction()
{
  assert(m_keyBind);
  qDebug("released");
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

void VCButton::slotRenameReturnPressed()
{
  assert(m_renameEdit);
  setText(m_renameEdit->text());
  disconnect(m_renameEdit);
  delete m_renameEdit;
  m_renameEdit = NULL;
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
