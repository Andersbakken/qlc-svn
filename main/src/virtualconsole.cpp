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
#include <qptrlist.h>
#include <qstring.h>
#include <qlayout.h>
#include <qobjcoll.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qslider.h>

#include "virtualconsole.h"
#include "app.h"
#include "doc.h"
#include "settings.h"
#include "vcslider.h"
#include "vcbutton.h"
#include "vcwidget.h"
#include "vcwidgetbase.h"
#include "keybind.h"
#include "vclabel.h"
#include "configkeys.h"
#include "vcdockarea.h"

#include <X11/Xlib.h>

extern App* _app;

const QString KEY_VIRTUAL_CONSOLE_OPEN  (  "VirtualConsoleOpen" );

VirtualConsole::VirtualConsole(QWidget* parent, const char* name) 
  : QWidget(parent, name)
{
  m_dockArea = NULL;
  m_drawArea = NULL;
}

VirtualConsole::~VirtualConsole()
{
  QString config;

  //
  // Save visible status
  //
  if (isShown())
    {
      config = Settings::trueValue();
    }
  else
    {
      config = Settings::falseValue();
    }

  _app->settings()->set(KEY_VIRTUAL_CONSOLE_OPEN, config);
}


void VirtualConsole::initView(void)
{
  setCaption("Virtual Console");

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(dir + QString("/virtualconsole.xpm"));

  m_layout = new QHBoxLayout(this);
  m_layout->setAutoAdd(false);

  m_menuBar = new QMenuBar(this);
  m_layout->setMenuBar(m_menuBar);

  m_addMenu = new QPopupMenu();
  m_addMenu->setCheckable(false);
  m_addMenu->insertItem(QPixmap(dir + "/button.xpm"), 
			"&Button", ID_VC_ADD_BUTTON);
  m_addMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			"&Slider", ID_VC_ADD_SLIDER);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"), 
			"&Frame", ID_VC_ADD_FRAME);
  m_addMenu->insertItem(QPixmap(dir + "/rename.xpm"), 
			"L&abel", ID_VC_ADD_LABEL);
  connect(m_addMenu, SIGNAL(activated(int)), 
	  this, SLOT(slotMenuItemActivated(int)));

  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->setCheckable(true);
  m_toolsMenu->insertItem(QPixmap(dir + "/slider.xpm"),
			  "&Default Sliders", ID_VC_TOOLS_SLIDERS);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(dir + "/panic.xpm"), 
			  "&Panic!", ID_VC_TOOLS_PANIC);
  connect(m_toolsMenu, SIGNAL(activated(int)), 
	  this, SLOT(slotMenuItemActivated(int)));

  m_menuBar->insertItem("&Add", m_addMenu, ID_VC_ADD);
  m_menuBar->insertItem("&Tools", m_toolsMenu, ID_VC_TOOLS);

  m_menuBar->setItemEnabled(ID_VC_ADD, true);

  VCWidget::ResetID();

  initDockArea();
  initDrawArea();

  slotModeChanged();
  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));

  // Check if VC should be open
  QString config;
  _app->settings()->get(KEY_VIRTUAL_CONSOLE_OPEN, config);
  if (config == Settings::trueValue())
    {
      _app->slotViewVirtualConsole();
    }
  else
    {
      hide();
      _app->slotVirtualConsoleClosed();
    }
}


//
// Menu callback
//
void VirtualConsole::slotMenuItemActivated(int item)
{
  switch(item)
    {
    case ID_VC_ADD_BUTTON:
      {
	VCButton* b;
	b = new VCButton(m_drawArea);
	b->init();
	b->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_SLIDER:
      {
	VCSlider* s;
	s = new VCSlider(m_drawArea);
	s->init();
	s->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_LABEL:
      {
 	VCLabel* p = NULL;
	p = new VCLabel(m_drawArea);
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_ADD_MONITOR:
      _app->doc()->setModified(true);
      break;

    case ID_VC_ADD_FRAME:
      {
	VCWidget* w;
	w = new VCWidget(m_drawArea);
	w->init();
	w->show();
	_app->doc()->setModified(true);
      }
      break;

    case ID_VC_TOOLS_PANIC:
      {
	_app->slotPanic();
      }
      break;

    case ID_VC_TOOLS_SLIDERS:
      {
	if (m_dockArea->isHidden())
	  {
	    m_dockArea->show();
	  }
	else
	  {
	    m_dockArea->hide();
	  }
      }
      break;

    default:
      break;
    }
}


void VirtualConsole::initDockArea()
{
  if (m_dockArea) delete m_dockArea;
  m_dockArea = new VCDockArea(this);
  connect(m_dockArea, SIGNAL(areaHidden(bool)),
	  this, SLOT(slotDockAreaHidden(bool)));
  m_dockArea->init();

  // Add the dock area into the master (horizontal) layout
  m_layout->addWidget(m_dockArea, 0);
}


void VirtualConsole::slotDockAreaHidden(bool areaHidden)
{
  if (areaHidden == true)
    {
      m_toolsMenu->setItemChecked(ID_VC_TOOLS_SLIDERS, false);
    }
  else
    {
      m_toolsMenu->setItemChecked(ID_VC_TOOLS_SLIDERS, true);
    }
}


void VirtualConsole::initDrawArea()
{
  if (m_drawArea) delete m_drawArea;
  m_drawArea = new VCWidget(this);
  m_drawArea->setBottomFrame(true);

  // Add the draw area into the master (horizontal) layout
  m_layout->addWidget(m_drawArea, 1);
}


//
// Set the mode (Design/Operate)
//
void VirtualConsole::slotModeChanged()
{
  QString config;
  _app->settings()->get("KeyRepeatOffInOperateMode", config);

  if (_app->mode() == App::Design)
    {
      qDebug("TODO: check if there are any running functions");
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
}


// Search for a parent frame by the id number <id>
// This is a recursive function and I have the feeling that it could
// be done in a more sophisticated way. Anyway, it works now.
VCWidget* VirtualConsole::getFrame(unsigned int id, VCWidget* widget)
{
  VCWidget* w = NULL;
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
      if (QString(it.current()->className()) == QString("VCWidget"))
	{
	  w = getFrame(id, (VCWidget*) it.current());
	  if (w != NULL)
	    {
	      break;
	    }
	}
    }

  return w;
}

void VirtualConsole::createWidget(QPtrList <QString> &list)
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
	      m_drawArea = new VCWidget(this);
	      m_drawArea->setBottomFrame(true);
	      m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	      
	      m_layout->addWidget(m_drawArea, 1);
	      
	      m_drawArea->createContents(list);

	      m_drawArea->show();
	    }
	  else
	    {
	      VCWidget* w = new VCWidget(m_drawArea);
	      w->createContents(list);
	    }
	}
      else if (*s == QString("Label"))
	{
	  VCLabel* w = new VCLabel(m_drawArea);
	  w->init();
	  w->createContents(list);
	}
      else if (*s == QString("Button"))
	{
	  VCButton* w = new VCButton(m_drawArea);
	  w->init();
	  w->createContents(list);
	}
      else if (*s == QString("Slider"))
	{
	  VCSlider* w = new VCSlider(m_drawArea);
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

void VirtualConsole::createVirtualConsole(QPtrList <QString>& list)
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

void VirtualConsole::createContents(QPtrList <QString> &list)
{
  QString t;

  VCWidget::ResetID();
  
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

  // Check if VC should be open
  QString config;
  _app->settings()->get(KEY_VIRTUAL_CONSOLE_OPEN, config);
  if (config == Settings::trueValue())
    {
      _app->slotViewVirtualConsole();
    }
  else
    {
      hide();
      _app->slotVirtualConsoleClosed();
    }
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

void VirtualConsole::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}

void VirtualConsole::keyPressEvent(QKeyEvent* e)
{
  VCWidgetBase* b = NULL;

  if (_app->mode() == App::Operate)
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
  VCWidgetBase* b = NULL;

  if (_app->mode() == App::Operate)
    {
      for (unsigned int i = 0; i < m_keyReceivers.count(); i++)
	{
	  b = m_keyReceivers.at(i);
	  b->keyRelease(e);
	}
    }
}

void VirtualConsole::registerKeyReceiver(VCWidgetBase* widget)
{
  ASSERT(widget != NULL);

  m_keyReceivers.append(widget);
}

void VirtualConsole::unRegisterKeyReceiver(VCWidgetBase* widget)
{
  ASSERT(widget != NULL);

  m_keyReceivers.remove(widget);
}

VCWidgetBase* VirtualConsole::searchKeyReceiver(VCWidgetBase* widget)
{
  VCWidgetBase* w = NULL;

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
