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

#include "virtualconsole.h"
#include "app.h"
#include "doc.h"
#include "settings.h"
#include "dmxslider.h"
#include "dmxbutton.h"
#include "dmxwidget.h"
#include "speedslider.h"
#include <qlayout.h>
#include <qobjcoll.h>

extern App* _app;

VirtualConsole::VirtualConsole(QWidget* parent, const char* name) 
  : QWidget(parent, name)
{
  m_designMode = true;
  m_objectPointMode = false;

  m_defaultSpeedSlider = NULL;

  m_drawArea = NULL;

  m_sourceObject = NULL;
  m_targetObject = NULL;
}

VirtualConsole::~VirtualConsole()
{
}

bool VirtualConsole::isDesignMode(void)
{ 
  return m_designMode;
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
	        m_drawArea = new DMXWidget(this, "Bottom Frame");
		m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
		connect(m_drawArea, SIGNAL(removed(DMXWidget*)), this, SLOT(slotDrawAreaRemoved(DMXWidget*)));
		m_layout->addWidget(m_drawArea, 1);
		m_drawArea->createContents(list);
		setGeometry(m_drawArea->rect());
		m_drawArea->show();
	    }
	  else
	    {
	      DMXWidget* w = new DMXWidget(m_drawArea);
	      w->createContents(list);
	    } 
	}
      else if (*s == QString("Button"))
	{
	  DMXButton* w = new DMXButton(m_drawArea);
	  w->createContents(list);
	}
      else if (*s == QString("SpeedSlider"))
	{
	  SpeedSlider* w = new SpeedSlider(m_drawArea);
	  w->createContents(list);
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}

void VirtualConsole::saveToFile(QFile& file)
{
  if (m_drawArea != NULL)
    {
      m_drawArea->saveFramesToFile(file);
      m_drawArea->saveChildrenToFile(file);
    }
}

void VirtualConsole::setDefaultSpeedSlider(SpeedSlider* s)
{
  if (m_defaultSpeedSlider != NULL)
    {
      switch(QMessageBox::warning(this, "QLC", "Another slider has already been assigned as the default speed slider.\nAre you sure wish to use this new one instead?", "OK", "Cancel", 0, 1))
	{
	case 0:
	  m_defaultSpeedSlider = s;
	  break;
	default:
	  break;
	}
    }
  else
    {
      m_defaultSpeedSlider = s;
    }
}

bool VirtualConsole::isObjectPointMode(void)
{
  return m_objectPointMode;
}

void VirtualConsole::initView(void)
{
  setCaption("Virtual Console");
  setIcon(_app->settings()->getPixmapPath() + QString("virtualconsole.xpm"));

  m_layout = new QVBoxLayout(this);

  m_menuBar = new QMenuBar(this);
  m_layout->addWidget(m_menuBar, 0);
  m_menuBar->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  connect(m_menuBar, SIGNAL(activated(int)), this, SLOT(slotMenuItemActivated(int)));

  // addBottomFrame();

  QPopupMenu* modeMenu;
  modeMenu = new QPopupMenu();
  modeMenu->setCheckable(true);
  modeMenu->insertItem("&Operate", ID_VC_MODE_OPERATE);
  modeMenu->insertItem("&Design", ID_VC_MODE_DESIGN);

  QPopupMenu* addMenu;
  addMenu = new QPopupMenu();
  addMenu->insertItem("&Button", ID_VC_ADD_BUTTON);
  addMenu->insertItem("&Slider", ID_VC_ADD_SLIDER);
  addMenu->insertItem("S&peed slider", ID_VC_ADD_SPEEDSLIDER);
  addMenu->insertItem("&Monitor", ID_VC_ADD_MONITOR);
  addMenu->insertItem("&Frame", ID_VC_ADD_FRAME);

  m_menuBar->insertItem("&Mode", modeMenu, ID_VC_MODE);
  m_menuBar->insertItem("&Add", addMenu, ID_VC_ADD);

  m_menuBar->setItemChecked(ID_VC_MODE_DESIGN, true);
  m_menuBar->setItemChecked(ID_VC_MODE_OPERATE, false);
  m_menuBar->setItemEnabled(ID_VC_ADD, true);
}

void VirtualConsole::newDocument()
{
  if (m_drawArea != NULL)
    {
      slotDrawAreaRemoved(m_drawArea);
    }
}

void VirtualConsole::slotDrawAreaRemoved(DMXWidget* widget)
{
  if (widget == m_drawArea)
    {
      disconnect(m_drawArea);
      delete m_drawArea;
      m_drawArea = NULL;
    }
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
      m_designMode = false;
      m_menuBar->setItemChecked(ID_VC_MODE_DESIGN, false);
      m_menuBar->setItemChecked(ID_VC_MODE_OPERATE, true);
      m_menuBar->setItemEnabled(ID_VC_ADD, false);
      setCaption("Virtual Console");
      emit modeChange(Operate);
      break;

    case ID_VC_MODE_DESIGN:
      m_designMode = true;
      m_menuBar->setItemChecked(ID_VC_MODE_DESIGN, true);
      m_menuBar->setItemChecked(ID_VC_MODE_OPERATE, false);
      m_menuBar->setItemEnabled(ID_VC_ADD, true);
      setCaption("Virtual Console - Design Mode");
      emit modeChange(Design);
      break;

    case ID_VC_ADD_BUTTON:
      {
	if (m_drawArea == NULL)
	  {
	    addBottomFrame();
	  }
	DMXButton* b;
	b = new DMXButton(m_drawArea);
	b->show();
      }
      break;

    case ID_VC_ADD_SLIDER:
      {
	if (m_drawArea == NULL)
	  {
	    addBottomFrame();
	  }
	DMXSlider* s;
	s = new DMXSlider(m_drawArea);
	s->resize(20, 120);
	s->show();
      }
      break;

    case ID_VC_ADD_SPEEDSLIDER:
      if (m_drawArea == NULL)
	{
	  addBottomFrame();
	}

      if (m_drawArea->getSpeedSlider() != NULL)
	{
	  MSG_INFO("This frame already has a default speed slider. Unable to add new one.");
	}
      else
	{
	  SpeedSlider* p = NULL;
	  p = new SpeedSlider(m_drawArea);
	  m_drawArea->setSpeedSlider(p);
	  connect(p, SIGNAL(destroyed()), m_drawArea, SLOT(slotSpeedSliderDestroyed()));
	  p->show();
	}
      break;

    case ID_VC_ADD_MONITOR:
      break;

    case ID_VC_ADD_FRAME:
      if (m_drawArea == NULL)
	{
	  addBottomFrame();
	}
      else
	{
	  DMXWidget* w;
	  w = new DMXWidget(m_drawArea);
	  w->resize(120, 120);
	  w->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	  w->show();
	}
      break;

    default:
      break;
    }
}

void VirtualConsole::addBottomFrame()
{
  m_drawArea = new DMXWidget(this, "Bottom Frame");
  m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  connect(m_drawArea, SIGNAL(removed(DMXWidget*)), this, SLOT(slotDrawAreaRemoved(DMXWidget*)));
  m_layout->addWidget(m_drawArea, 1);
  m_drawArea->show();
}

void VirtualConsole::keyPressEvent(QKeyEvent* e)
{
  KeyBind* b = NULL;

  for (unsigned int i = 0; i < m_bindings.count(); i++)
    {
      b = m_bindings.at(i);
      //      if (e->key() == b->key)
      //	{
      //	  b->receiver->keyPress(e);
      //	}
    }
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* e)
{
  KeyBind* b = NULL;

  for (unsigned int i = 0; i < m_bindings.count(); i++)
    {
      b = m_bindings.at(i);
      //      if (e->key() == b->key)
      //	{
      //	  b->receiver->keyRelease(e);
      //	}
    }
}

void VirtualConsole::registerKeyBind(KeyBind* kb)
{
  if (kb == NULL)
    {
      return;
    }

  for (unsigned int i = 0; i < m_bindings.count(); i++)
    {
    }
}
