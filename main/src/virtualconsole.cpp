/*
  Q Light Controller
  virtualconsole.cpp
  
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

#include <qmenubar.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qlayout.h>
#include <qobjcoll.h>
#include <qlabel.h>
#include <qmessagebox.h>

#include "virtualconsole.h"
#include "app.h"
#include "doc.h"
#include "settings.h"
#include "dmxslider.h"
#include "dmxbutton.h"
#include "dmxwidget.h"
#include "speedslider.h"
#include "dmxwidgetbase.h"
#include "keybind.h"
#include "dmxlabel.h"
#include "sequenceprovider.h"
#include "configkeys.h"

#include <X11/Xlib.h>

extern App* _app;

VirtualConsole::VirtualConsole(QWidget* parent, const char* name) 
  : QWidget(parent, name)
{
  m_drawArea = NULL;
  m_defaultSpeedBus = NULL;
}

VirtualConsole::~VirtualConsole()
{
}

void VirtualConsole::setDefaultSpeedBus(Bus* bus)
{
  m_defaultSpeedBus = bus;

  if (bus != NULL)
    {
      connect(bus, SIGNAL(destroyed()), this, SLOT(slotDefaultSpeedBusDestroyed()));
    }
}

void VirtualConsole::slotDefaultSpeedBusDestroyed()
{
  m_defaultSpeedBus = NULL;

  Bus* bus = _app->doc()->busList()->at(_app->doc()->busList()->count() - 1);
  if (bus != NULL)
    {
      m_defaultSpeedBus = bus;
    }
}

bool VirtualConsole::isDesignMode(void)
{ 
  return (m_mode == Design) ? true : false;
}

void VirtualConsole::setMode(Mode mode)
{
  //
  // If we're gonna change to design mode when functions are running
  //
  if (mode == Design)
    {
      QList <Feeder> *feederList = _app->sequenceProvider()->feederList();
      
      if (feederList->count() > 0)
	{
	  int result = QMessageBox::warning(this, QString("Virtual Console"),
					    QString("There are running functions. Do you want to stop them now?"),
					    QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
	  if (result == QMessageBox::Yes)
	    {
	      _app->sequenceProvider()->flush();
	    }
	  else if (result == QMessageBox::Cancel)
	    {
	      return;
	    }
	}
    }

  QString config;
  _app->settings()->get("KeyRepeatOffInOperateMode", config);

  if (mode == Design)
    {
      m_mode = Design;
      m_menuBar->setItemChecked(ID_VC_MODE_DESIGN, true);
      m_menuBar->setItemChecked(ID_VC_MODE_OPERATE, false);
      m_menuBar->setItemEnabled(ID_VC_ADD, true);
      setCaption("Virtual Console - Design Mode");

      /* Set auto repeat off when in "Operate" mode and on 
       * again when vc is put to "Design" mode.
       */
      if (config == QString("true"))
	{
	  Display* display;
	  display = XOpenDisplay(NULL);
	  ASSERT(display != NULL);
	  
	  XAutoRepeatOn(display);
	  
	  XCloseDisplay(display);
	}
    }
  else
    {
      m_mode = Operate;
      m_menuBar->setItemChecked(ID_VC_MODE_DESIGN, false);
      m_menuBar->setItemChecked(ID_VC_MODE_OPERATE, true);
      m_menuBar->setItemEnabled(ID_VC_ADD, false);
      setCaption("Virtual Console - Operate Mode");

      /* Set auto repeat off when in "Operate" mode and on 
       * again when vc is put to "Design" mode.
       */
      if (config == QString("true"))
	{
	  Display* display;
	  display = XOpenDisplay(NULL);
	  ASSERT(display != NULL);
	  
	  XAutoRepeatOff(display);
	  
	  XCloseDisplay(display);
	}
    }

  emit modeChange(m_mode);
}

// Search for a parent frame by the id number <id>
// This is a recursive function and I have the feeling that it could
// be done in a more sophisticated way. Anyway, it works now.
DMXWidget* VirtualConsole::getFrame(unsigned int id, DMXWidget* widget)
{
  DMXWidget* w = NULL;
  QObjectList* ol = NULL;

  if (widget != NULL)
    {
      if (id == widget->id())
	{
	  return widget;
	}

      if (widget->children() != NULL)
	{
	  ol = (QObjectList*) widget->children();
	}
      else
	{
	  return NULL;
	}
    }
  else
    {
      if (id == m_drawArea->id())
	{
	  return m_drawArea;
	}
      else
	{
	  ol = (QObjectList*) m_drawArea->children();
	}
    }

  for (QObjectListIt it(*ol); it.current() != NULL; ++it)
    {
      if (QString(it.current()->className()) == QString("DMXWidget"))
	{
	  w = getFrame(id, (DMXWidget*) it.current());
	  if (w != NULL)
	    {
	      break;
	    }
	}
    }

  return w;
}

void VirtualConsole::createWidget(QList<QString> &list)
{
  QString t;
  
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Frame"))
	{
	  if (m_drawArea == NULL)
	    {
	      m_drawArea = new DMXWidget(this);
	      m_drawArea->setBottomFrame(true);
	      m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	      
	      m_layout->addWidget(m_drawArea, 1);
	      
	      m_drawArea->createContents(list);

	      m_drawArea->show();
	    }
	  else
	    {
	      DMXWidget* w = new DMXWidget(m_drawArea);
	      w->createContents(list);
	    }
	}
      else if (*s == QString("Label"))
	{
	  DMXLabel* w = new DMXLabel(m_drawArea);
	  w->init();
	  w->createContents(list);
	}
      else if (*s == QString("Button"))
	{
	  DMXButton* w = new DMXButton(m_drawArea);
	  w->init();
	  w->createContents(list);
	}
      else if (*s == QString("SpeedSlider"))
	{
	  SpeedSlider* w = new SpeedSlider(m_drawArea);
	  w->createContents(list);
	}
      else if (*s == QString("Slider"))
	{
	  DMXSlider* w = new DMXSlider(m_drawArea);
	  w->init();
	  w->createContents(list);
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}

void VirtualConsole::createVirtualConsole(QList<QString>& list)
{
  QString t;
  QRect rect(10, 10, 400, 400);

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  list.prev();
	  break;
	}
      else if (*s == QString("Mode"))
	{
	  t = *(list.next());
	  if (t == QString("Design"))
	    {
	      setMode(Design);
	    }
	  else
	    {
	      setMode(Operate);
	    }
	}
      else if (*s == QString("X"))
	{
	  t = *(list.next());
	  rect.setX(t.toInt());
	}
      else if (*s == QString("Y"))
	{
	  t = *(list.next());
	  rect.setY(t.toInt());
	}
      else if (*s == QString("Width"))
	{
	  t = *(list.next());
	  rect.setWidth(t.toInt());
	}
      else if (*s == QString("Height"))
	{
	  t = *(list.next());
	  rect.setHeight(t.toInt());
	}
      else
	{
	  list.next();
	}
    }

  setGeometry(rect);
}

void VirtualConsole::createContents(QList<QString> &list)
{
  QString t;

  DMXWidget::globalDMXWidgetIDReset();
  
  if (m_drawArea != NULL)
    {
      delete m_drawArea;
      m_drawArea = NULL;
    }

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.next();

	  if (*s == QString("Virtual Console"))
	    {
	      createVirtualConsole(list);
	    }
	  else if (*s == QString("Frame"))
	    {
	      list.prev();
	      createWidget(list);
	    }
	  else if (*s == QString("Button"))
	    {
	      list.prev();
	      createWidget(list);
	    }
	  else if (*s == QString("SpeedSlider"))
	    {
	      list.prev();
	      createWidget(list);
	    }
	  else if (*s == QString("Label"))
	    {
	      list.prev();
	      createWidget(list);
	    }
	  else if (*s == QString("Slider"))
	    {
	      list.prev();
	      createWidget(list);
	    }
	  else
	    {
	      // Unknown keyword, skip
	      list.next();
	    }
	}
      else
	{
	  list.next();
	}
    }

  // Virtual console sometimes loses its parent (or vice versa)
  // when loading a new document... try to handle it with this.
  reparent((QWidget*) _app->workspace(), 0, pos(), isVisible());
}

void VirtualConsole::saveToFile(QFile& file)
{
  QString s;
  QString t;

  // Comment
  s = QString("# Virtual Console Master Entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Virtual Console") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  t = (m_mode == Design) ? QString("Design") : QString("Operate");
  s = QString("Mode = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // X
  t.setNum(rect().x());
  s = QString("X = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Y
  t.setNum(rect().y());
  s = QString("Y = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Width
  t.setNum(rect().width());
  s = QString("Width = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Height
  t.setNum(rect().height());
  s = QString("Height = ") + t + QString("\n");
  file.writeBlock((const char*) s, s.length());

  ASSERT(m_drawArea != NULL);

  m_drawArea->saveFramesToFile(file);
  m_drawArea->saveChildrenToFile(file);
}

void VirtualConsole::initView(void)
{
  setCaption("Virtual Console");

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(dir + QString("/virtualconsole.xpm"));

  m_layout = new QVBoxLayout(this);
  m_layout->setAutoAdd(false);
  
  m_menuBar = new QMenuBar(this);
  m_layout->setMenuBar(m_menuBar);

  newDocument();

  m_modeMenu = new QPopupMenu();
  m_modeMenu->setCheckable(true);
  m_modeMenu->insertItem("&Operate", ID_VC_MODE_OPERATE);
  m_modeMenu->insertItem("&Design", ID_VC_MODE_DESIGN);
  connect(m_modeMenu, SIGNAL(activated(int)), this, SLOT(slotMenuItemActivated(int)));

  m_addMenu = new QPopupMenu();
  m_addMenu->setCheckable(false);
  m_addMenu->insertItem(QPixmap(dir + "/button.xpm"), "&Button", ID_VC_ADD_BUTTON);
  m_addMenu->insertItem(QPixmap(dir + "/slider.xpm"), "&Slider", ID_VC_ADD_SLIDER);
  m_addMenu->insertItem(QPixmap(dir + "/speedslider.xpm"), "S&peed slider", ID_VC_ADD_SPEEDSLIDER);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"), "&Frame", ID_VC_ADD_FRAME);
  m_addMenu->insertItem(QPixmap(dir + "/rename.xpm"), "L&abel", ID_VC_ADD_LABEL);
  connect(m_addMenu, SIGNAL(activated(int)), this, SLOT(slotMenuItemActivated(int)));

  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->setCheckable(false);
  m_toolsMenu->insertItem(QPixmap(dir + "/panic.xpm"), "&Panic!", ID_VC_TOOLS_PANIC);
  connect(m_toolsMenu, SIGNAL(activated(int)), this, SLOT(slotMenuItemActivated(int)));

  m_menuBar->insertItem("&Mode", m_modeMenu, ID_VC_MODE);
  m_menuBar->insertItem("&Add", m_addMenu, ID_VC_ADD);
  m_menuBar->insertItem("&Tools", m_toolsMenu, ID_VC_TOOLS);

  m_menuBar->setItemEnabled(ID_VC_ADD, true);
  setMode(Design);
}

void VirtualConsole::newDocument()
{
  DMXWidget::globalDMXWidgetIDReset();
  
  if (m_drawArea != NULL)
    {
      delete m_drawArea;
    }

  m_drawArea = new DMXWidget(this);
  m_drawArea->setBottomFrame(true);
  m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  m_layout->addWidget(m_drawArea, 0);

  m_drawArea->show();
}

void VirtualConsole::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}

void VirtualConsole::slotMenuItemActivated(int item)
{
  switch(item)
    {
    case ID_VC_MODE_OPERATE:
      {
	setMode(Operate);
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_MODE_DESIGN:
      {
	setMode(Design);
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_BUTTON:
      {
	DMXButton* b;
	b = new DMXButton(m_drawArea);
	b->init();
	b->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_SLIDER:
      {
	DMXSlider* s;
	s = new DMXSlider(m_drawArea);
	s->init();
	s->setGeometry(0, 0, 20, 120);
	s->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_SPEEDSLIDER:
      {
	SpeedSlider* p = NULL;
	p = new SpeedSlider(m_drawArea);
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_LABEL:
      {
 	DMXLabel* p = NULL;
	p = new DMXLabel(m_drawArea);
	p->setText("New label");
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_MONITOR:
	_app->doc()->setModified(true);
      break;

    case ID_VC_ADD_FRAME:
      {
	DMXWidget* w;
	w = new DMXWidget(m_drawArea);
	w->resize(120, 120);
	w->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	w->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_TOOLS_PANIC:
      {
	_app->slotPanic();
      }
      break;

    default:
      break;
    }
}

void VirtualConsole::keyPressEvent(QKeyEvent* e)
{
  DMXWidgetBase* b = NULL;

  if (m_mode == Operate)
    {
      for (unsigned int i = 0; i < m_keyReceivers.count(); i++)
	{
	  b = m_keyReceivers.at(i);
	  b->keyPress(e);
	}
    }
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* e)
{
  DMXWidgetBase* b = NULL;

  if (m_mode == Operate)
    {
      for (unsigned int i = 0; i < m_keyReceivers.count(); i++)
	{
	  b = m_keyReceivers.at(i);
	  b->keyRelease(e);
	}
    }
}

void VirtualConsole::registerKeyReceiver(DMXWidgetBase* widget)
{
  ASSERT(widget != NULL);

  m_keyReceivers.append(widget);
}

void VirtualConsole::unRegisterKeyReceiver(DMXWidgetBase* widget)
{
  ASSERT(widget != NULL);

  m_keyReceivers.remove(widget);
}

DMXWidgetBase* VirtualConsole::searchKeyReceiver(DMXWidgetBase* widget)
{
  DMXWidgetBase* w = NULL;

  ASSERT(widget != NULL);

  for (uint i = 0; i < m_keyReceivers.count(); i++)
    {
      w = m_keyReceivers.at(i);
      if (w == widget)
	{
	  break;
	}
    }

  return w;
}
