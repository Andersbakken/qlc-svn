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
#include "virtualconsoleproperties.h"
#include "app.h"
#include "doc.h"
#include "settings.h"
#include "vcdockslider.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "keybind.h"
#include "vclabel.h"
#include "configkeys.h"
#include "vcdockarea.h"
#include "vcdockslider.h"

#include <X11/Xlib.h>

extern App* _app;

const int KMenuAdd                 ( 0 );
const int KMenuAddButton           ( 1 );
const int KMenuAddSlider           ( 2 );
const int KMenuAddFrame            ( 3 );
const int KMenuAddLabel            ( 4 );
const int KMenuProperties          ( 5 );
const int KMenuDefaultSliders      ( 6 );
const int KMenuPanic               ( 7 );

VirtualConsole::VirtualConsole(QWidget* parent, const char* name) 
  : QWidget(parent, name)
{
  m_dockArea = NULL;
  m_drawArea = NULL;
  m_gridEnabled = false;
  m_gridX = 0;
  m_gridY = 0;
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
  resize(200, 400);

  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  setIcon(dir + QString("/virtualconsole.xpm"));

  m_layout = new QHBoxLayout(this);
  m_layout->setAutoAdd(false);

  m_menuBar = new QMenuBar(this);
  m_layout->setMenuBar(m_menuBar);

  //
  // Add menu
  //
  m_addMenu = new QPopupMenu();
  m_addMenu->setCheckable(false);
  m_addMenu->insertItem(QPixmap(dir + "/button.xpm"), 
			"&Button", KMenuAddButton);
  m_addMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			"&Slider", KMenuAddSlider);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"), 
			"&Frame", KMenuAddFrame);
  m_addMenu->insertItem(QPixmap(dir + "/rename.xpm"), 
			"L&abel", KMenuAddLabel);
  connect(m_addMenu, SIGNAL(activated(int)), 
	  this, SLOT(slotMenuItemActivated(int)));

  //
  // Tools menu
  //
  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->setCheckable(true);
  m_toolsMenu->insertItem(QPixmap(dir + "/settings.xpm"),
			  "&Properties...", KMenuProperties);
  m_toolsMenu->insertItem(QPixmap(dir + "/slider.xpm"),
			  "&Default Sliders", KMenuDefaultSliders);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(dir + "/panic.xpm"), 
			  "&Panic!", KMenuPanic);

  connect(m_toolsMenu, SIGNAL(activated(int)), 
	  this, SLOT(slotMenuItemActivated(int)));

  m_menuBar->insertItem("&Add", m_addMenu, KMenuAdd);
  m_menuBar->insertItem("&Tools", m_toolsMenu);
  m_menuBar->setItemEnabled(KMenuAdd, true);

  // Reset ID
  VCFrame::ResetID();

  // Dock & Draw Area
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

  // Grid
  _app->settings()->get(KEY_VIRTUAL_CONSOLE_SNAPGRID, config);
  if (config == Settings::trueValue())
    {
      m_gridEnabled = true;
    }
  else
    {
      m_gridEnabled = false;
    }

  // Grid X
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_GRIDX, config))
    {
      m_gridX = config.toInt();
    }
  else
    {
      m_gridX = 10;
    }

  // Grid Y
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_GRIDY, config))
    {
      m_gridY = config.toInt();
    }
  else
    {
      m_gridY = 10;
    }
}


//
// Menu callback
//
void VirtualConsole::slotMenuItemActivated(int item)
{
  switch(item)
    {
    case KMenuAddButton:
      {
	VCButton* b;
	b = new VCButton(m_drawArea);
	b->init();
	b->show();
	_app->doc()->setModified(true);
      }
      break;

    case KMenuAddSlider:
      {
	VCDockSlider* vcd = new VCDockSlider(m_drawArea);
	vcd->setBusID(KBusIDDefaultFade);
	vcd->init();
	vcd->resize(60, 200);
	vcd->show();
	_app->doc()->setModified(true);
      }
      break;

    case KMenuAddFrame:
      {
	VCFrame* w;
	w = new VCFrame(m_drawArea);
	w->init();
	w->show();
	_app->doc()->setModified(true);
      }
      break;

    case KMenuAddLabel:
      {
 	VCLabel* p = NULL;
	p = new VCLabel(m_drawArea);
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    case KMenuProperties:
      {
	VirtualConsoleProperties* vcp = new VirtualConsoleProperties(this);
	vcp->init();
	if (vcp->exec() == QDialog::Accepted)
	  {
	    // Cache grid values so widgets don't have to get them
	    // from settings each time (which is slow)
	    m_gridEnabled = vcp->isGridEnabled();
	    m_gridX = vcp->gridX();
	    m_gridY = vcp->gridY();
	  }
	delete vcp;
      }
      break;

    case KMenuDefaultSliders:
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

    case KMenuPanic:
      {
	_app->slotPanic();
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
      m_toolsMenu->setItemChecked(KMenuDefaultSliders, false);
    }
  else
    {
      m_toolsMenu->setItemChecked(KMenuDefaultSliders, true);
    }
}


void VirtualConsole::initDrawArea()
{
  if (m_drawArea) delete m_drawArea;
  m_drawArea = new VCFrame(this);
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
  
  //
  // Key repeat
  //
  _app->settings()->get(KEY_VIRTUAL_CONSOLE_KEYREPEAT, config);
  if (config == Settings::trueValue())
    {
      Display* display;
      display = XOpenDisplay(NULL);
      ASSERT(display != NULL);
      
      if (_app->mode() == App::Design)
	{
	  XAutoRepeatOn(display);
	}
      else
	{
	  XAutoRepeatOff(display);
	}
      
      XCloseDisplay(display);
    }

  //
  // Grab keyboard
  //
  _app->settings()->get(KEY_VIRTUAL_CONSOLE_GRABKB, config);
  if (config == Settings::trueValue())
    {
      if (_app->mode() == App::Design)
	{
	  releaseKeyboard();
	}
      else
	{
	  grabKeyboard();
	}
    }
}


// Search for a parent frame by the id number <id>
// This is a recursive function and I have the feeling that it could
// be done in a more sophisticated way. Anyway, it works now.
VCFrame* VirtualConsole::getFrame(unsigned int id, VCFrame* widget)
{
  VCFrame* w = NULL;
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
      if (QString(it.current()->className()) == QString("VCFrame"))
	{
	  w = getFrame(id, (VCFrame*) it.current());
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
	      m_drawArea = new VCFrame(this);
	      m_drawArea->setBottomFrame(true);
	      m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	      
	      m_layout->addWidget(m_drawArea, 1);
	      
	      m_drawArea->createContents(list);

	      m_drawArea->show();
	    }
	  else
	    {
	      VCFrame* w = new VCFrame(m_drawArea);
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
	  VCDockSlider* s = new VCDockSlider(m_drawArea);
	  s->init();
	  s->createContents(list);
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

  VCFrame::ResetID();
  
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
  if (_app->mode() == App::Operate)
    {
      emit keyPressed(e);
      e->accept();
    }
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* e)
{
  if (_app->mode() == App::Operate)
    {
      emit keyReleased(e);
      e->accept();
    }
}
