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
#include "vcwidgetbase.h"
#include "vcwidget.h"
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

extern App* _app;

const int KReadyColorMask (0xff); // Produces opposite colors with XOR
const int KFlashReadyTime (1000); // 1 second

VCButton::VCButton(VCWidget* parent) 
  : QPushButton(parent, "VCButton")
{
  m_function = NULL;
  m_resizeMode = false;
  m_renameEdit = NULL;
  m_bgColor = NULL;
  m_keyBind = NULL;
  m_bgPixmapFileName = QString::null;
  m_bgPixmap = NULL;

  m_lock = false;
}


void VCButton::init()
{
  setToggleButton(true);

  m_keyBind = new KeyBind();

  m_name = QString("Btn");
  setText(m_name);
  QToolTip::add(this, "No function");

  resize(30, 30);
}


void VCButton::copyFrom(VCButton* button)
{
  m_function =  button->function();
  m_resizeMode = false;
  m_renameEdit = NULL;

  ASSERT(button->keyBind());
  m_keyBind = new KeyBind(button->keyBind());

  m_lock = false;

  setToggleButton(true);

  m_name = ((VCWidgetBase*) button)->name();
  setText(m_name);

  m_bgColor = button->bgColor();
  setPaletteBackgroundColor(*m_bgColor);
	  
  reparent(button->parentWidget(), 0, QPoint(0, 0), true);

  setGeometry(button->geometry());

  move(button->x() + 20, button->y() + 20);
}


VCButton::~VCButton()
{
  _app->virtualConsole()->unRegisterKeyReceiver(this);
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

   
  // Lock
  s = QString("Lock = ") + ((VCWidgetBase::locked() == true) ? 
			    QString("True\n") : QString("False\n"));
  file.writeBlock((const char*) s, s.length());

  // Function
  if (m_function != NULL)
    {
      s.sprintf("Function = %d\n", m_function->id());
      file.writeBlock((const char*) s, s.length());
    }

  // Key binding
  ASSERT (m_keyBind != NULL);

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
  int key = 0;
  int mod = 0;

  KeyBind::PressAction pressAction = KeyBind::PressStart;
  KeyBind::ReleaseAction releaseAction = KeyBind::ReleaseNothing;

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
	  attachFunction(_app->doc()->searchFunction(list.next()->toInt()));
	}
      else if (*s == QString("BindKey"))
	{
	  QString t = *(list.next());
	  key = t.toInt();
	}
      else if (*s == QString("BindMod"))
	{
	  QString t = *(list.next());
	  mod = t.toInt();
	}
      else if (*s == QString("BindPress"))
	{
	  QString t = *(list.next());
	  pressAction = (KeyBind::PressAction) t.toInt();
	}
      else if (*s == QString("BindRelease"))
	{
	  QString t = *(list.next());
	  releaseAction = (KeyBind::ReleaseAction) t.toInt();
	}
      else if (*s == QString("Lock"))
	{
	  QString t = *(list.next());
	  m_lock = (t == QString("True")) ? true : false;
	}
      else
	{
	  // Unknown keyword, ignore
	  *list.next();
	}
    }

  setGeometry(rect);

  m_keyBind = new KeyBind(key, mod);
  m_keyBind->setPressAction(pressAction);
  m_keyBind->setReleaseAction(releaseAction);

  if (m_keyBind->valid() == true)
    {
      _app->virtualConsole()->registerKeyReceiver(this);
    }

  QString str("No function");

  if (m_function)
    {
      str = m_function->name();
    }

  QToolTip::add(this, str);
}

Function* VCButton::function() const
{
  return m_function;
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

      if (e->button() & LeftButton && m_lock == false)
	{
	  m_origX = e->globalX();
	  m_origY = e->globalY();
	  setCursor(QCursor(SizeAllCursor));
	}
      else if (e->button() & MidButton && m_lock == false)
	{
	  m_resizeMode = true;
	  setMouseTracking(true);
	  setCursor(QCursor(SizeFDiagCursor));
	}
      else if (e->button() & RightButton)
	{
	  QPopupMenu* menu;
	  menu = new QPopupMenu;
	  menu->insertItem("Button", -1);
	  menu->insertSeparator();

	  QString dir;
	  _app->settings()->get(KEY_SYSTEM_DIR, dir);
	  dir += QString("/") + PIXMAPPATH;
	  
	  QPopupMenu* sizeMenu;
	  sizeMenu = new QPopupMenu();
	  sizeMenu->setCheckable(false);
	  sizeMenu->insertItem("&Tiny", VCWIDGET_SIZE_TINY);
	  sizeMenu->insertItem("&Small", VCWIDGET_SIZE_SMALL);
	  sizeMenu->insertItem("&Normal", VCWIDGET_SIZE_NORMAL);
	  sizeMenu->insertItem("&Large", VCWIDGET_SIZE_LARGE);
	  sizeMenu->insertItem("&Huge", VCWIDGET_SIZE_HUGE);
	  sizeMenu->insertSeparator();
	  sizeMenu->insertItem("&Manual", VCWIDGET_SIZE_MANUAL);

	  menu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			   "&Rename...", VCWIDGET_MENU_RENAME);
	  menu->insertItem(QPixmap(dir + QString("/settings.xpm")),
			   "&Properties...", VCWIDGET_MENU_PROPERTIES);
	  menu->insertItem(QPixmap(dir + QString("/move.xpm")),
			   "&Size", sizeMenu);
	  menu->insertItem(QPixmap(dir + QString("/color.xpm")),
			     "Color...", VCWIDGET_MENU_BACKGROUND_COLOR);
	  menu->insertItem(QPixmap(dir + QString("/image.xpm")),
			     "Pixmap...", VCWIDGET_MENU_BACKGROUND_PIXMAP);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/attach.xpm")),
			   "&Attach function...", VCBUTTON_MENU_ATTACH);
	  menu->insertItem(QPixmap(dir + QString("/detach.xpm")),
			   "&Detach current function", VCBUTTON_MENU_DETACH);
	  menu->insertSeparator();
	  menu->insertItem(QPixmap(dir + QString("/editcopy.xpm")),
			   "Duplicate", VCWIDGET_MENU_COPY);
	  menu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			   "Re&move", VCWIDGET_MENU_REMOVE);

	  connect(menu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));
	  connect(sizeMenu, SIGNAL(activated(int)),
		  this, SLOT(slotMenuCallback(int)));

	  menu->exec(mapToGlobal(e->pos()));

	  delete sizeMenu;
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
    case VCWIDGET_SIZE_TINY:
      {
	_app->doc()->setModified(true);
	resize(15, 15);
      }
      break;

    case VCWIDGET_SIZE_SMALL:
      {
	_app->doc()->setModified(true);
	resize(20, 20);
      }
      break;

    case VCWIDGET_SIZE_NORMAL:
      {
	_app->doc()->setModified(true);
	resize(30, 30);
      }
      break;

    case VCWIDGET_SIZE_LARGE:
      {
	_app->doc()->setModified(true);
	resize(40, 40);
      }
      break;

    case VCWIDGET_SIZE_HUGE:
      {
	_app->doc()->setModified(true);
	resize(50, 50);
      }
      break;

    case VCWIDGET_SIZE_MANUAL:
      {
	_app->doc()->setModified(true);
	m_resizeMode = true;
	setMouseTracking(true);
	setCursor(QCursor(SizeFDiagCursor));
      }
      break;

    case VCWIDGET_MENU_RENAME:
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

    case VCWIDGET_MENU_BACKGROUND_COLOR:
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
      
      case VCWIDGET_MENU_BACKGROUND_PIXMAP:
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
    case VCWIDGET_MENU_PROPERTIES:
      {
	VCButtonProperties* p = NULL;
	p = new VCButtonProperties(this);

	if (p->exec() == QDialog::Accepted)
	  {
	    // Name
	    m_name = (QString) p->name();
	    setText(m_name);

	    // Delete old keybind
	    ASSERT (m_keyBind != NULL);
	    ASSERT (p->keyBind() != NULL);

	    delete m_keyBind;
	    m_keyBind = NULL;
	    m_keyBind = new KeyBind((KeyBind*) p->keyBind());

	    if (m_keyBind->valid() == true)
	      {
		_app->virtualConsole()
		  ->unRegisterKeyReceiver((VCWidgetBase*) this);
		_app->virtualConsole()
		  ->registerKeyReceiver((VCWidgetBase*) this);
	      }
	    else
	      {
		_app->virtualConsole()
		  ->unRegisterKeyReceiver((VCWidgetBase*) this);
	      }

	    // Widget position lock
	    m_lock = p->lock();
	    
	    // Function
	    attachFunction(p->function());
	  }

	delete p;
      }
      break;

    case VCWIDGET_MENU_REMOVE:
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

    case VCBUTTON_MENU_ATTACH:
      {
	FunctionTree* ft = new FunctionTree(this);
	if (ft->exec() == QDialog::Accepted)
	  {
	    Function* function = NULL;
	    function = _app->doc()->searchFunction(ft->functionId());
	    ASSERT(function);

	    attachFunction(function);
	  }
	delete ft;
      }
      break;

    case VCBUTTON_MENU_DETACH:
      {
	attachFunction(NULL);
      }
      break;

    case VCWIDGET_MENU_COPY:
      {
	VCButton* bt = NULL;
	bt = new VCButton((VCWidget*) this->parentWidget());
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
      if (m_resizeMode == true && m_lock == false)
	{
	  QPoint pos(e->globalX(), e->globalY());
	  pos = mapFromGlobal(pos);
	  resize(pos.x() + 2, pos.y() + 2);
	  _app->doc()->setModified(true);
	}
      else if (e->state() & LeftButton)
	{
	  if (moveThreshold(e->globalX(), e->globalY()) == true 
	      && m_lock == false)
	    {
	      _app->doc()->setModified(true);
	      moveTo(e->globalX(), e->globalY());
	    }
	}
    }
  else
    {
      QPushButton::mouseMoveEvent(e);
    }
}

bool VCButton::moveThreshold(int x, int y)
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

void VCButton::moveTo(int x, int y)
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


void VCButton::mouseDoubleClickEvent(QMouseEvent* e)
{
  if (_app->mode() == App::Design)
    {
      slotMenuCallback(VCWIDGET_MENU_PROPERTIES);
    }
  else
    {
      mousePressEvent(e);
    }
}

void VCButton::pressFunction()
{
  ASSERT(m_keyBind != NULL);

  if (m_keyBind->pressAction() == KeyBind::PressNothing || m_function == NULL)
    {
      return;
    }
  else if (m_keyBind->pressAction() == KeyBind::PressStart)
    {
      m_function->engage(static_cast<QObject*> (this));
      setOn(true);
    }
  else if (m_keyBind->pressAction() == KeyBind::PressToggle)
    {
      if (isOn())
	{
	  m_function->stop();
	  setOn(false);
	}
      else
	{
	  m_function->engage(static_cast<QObject*> (this));
	  setOn(true);
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
  if (m_function)
    {
    }
}

void VCButton::keyPress(QKeyEvent* e)
{
  ASSERT(m_keyBind != NULL);

  if (e != NULL && m_keyBind->valid() == true && e->key() == m_keyBind->key())
    {
      pressFunction();
    }
}

void VCButton::keyRelease(QKeyEvent* e)
{
  ASSERT(m_keyBind != NULL);

  if (e != NULL && m_keyBind->valid() == true && e->key() == m_keyBind->key())
    {
      releaseFunction();
    }
}

void VCButton::attachFunction(Function* function)
{
  _app->doc()->setModified(true);

  m_function = function;

  if (m_function)
    {
      QToolTip::add(this, m_function->name());
    }
  else
    {
      QToolTip::remove(this);
    }
}

void VCButton::slotRenameReturnPressed()
{
  m_name = m_renameEdit->text();
  setText(m_name);
  disconnect(m_renameEdit);
  delete m_renameEdit;
  m_renameEdit = NULL;
  _app->doc()->setModified(true);
}

void VCButton::slotFunctionDestroyed()
{
  attachFunction(NULL);
}

void VCButton::customEvent(QCustomEvent* e)
{
  if (e->type() == KFunctionStopEvent)
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
  c.setRgb(c.red() ^ KReadyColorMask,
	   c.green() ^ KReadyColorMask,
	   c.blue() ^ KReadyColorMask);
  setPaletteBackgroundColor(c);
}
