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
#include <qapplication.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qcursor.h>
#include <assert.h>

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
#include "floatingedit.h"

#include <X11/Xlib.h>

extern App* _app;

VirtualConsole::VirtualConsole(QWidget* parent, const char* name) 
  : QWidget(parent, name)
{
  m_dockArea = NULL;
  m_drawArea = NULL;
  m_gridEnabled = false;
  m_gridX = 0;
  m_gridY = 0;
  m_selectedWidget = NULL;
  m_renameEdit = NULL;
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

  initMenuBar();

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


void VirtualConsole::initMenuBar()
{
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
  m_addMenu->insertItem(QPixmap(dir + "/button.xpm"), 
			"&Button", KVCMenuAddButton);
  m_addMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			"&Slider", KVCMenuAddSlider);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"), 
			"&Frame", KVCMenuAddFrame);
  m_addMenu->insertItem(QPixmap(dir + "/rename.xpm"), 
			"L&abel", KVCMenuAddLabel);

  //
  // Tools menu
  //
  QPopupMenu* toolsMenu = new QPopupMenu();
  toolsMenu->insertItem(QPixmap(dir + "/settings.xpm"),
			"&Settings...", KVCMenuToolsSettings);
  toolsMenu->insertItem(QPixmap(dir + "/slider.xpm"),
			"&Default Sliders", KVCMenuToolsDefaultSliders);
  toolsMenu->insertSeparator();
  toolsMenu->insertItem(QPixmap(dir + "/panic.xpm"), 
			"&Panic!", KVCMenuToolsPanic);

  //
  // Function menu
  //
  QPopupMenu* functionMenu = new QPopupMenu();
  functionMenu->insertItem(QPixmap(dir + QString("/attach.xpm")),
			   "&Attach function...", KVCMenuWidgetFunctionAttach);
  functionMenu->insertItem(QPixmap(dir + QString("/detach.xpm")),
			   "&Detach current function", 
			   KVCMenuWidgetFunctionDetach);

  //
  // Foreground menu
  //
  QPopupMenu* fgMenu = new QPopupMenu();
  fgMenu->insertItem(QPixmap(dir + QString("/rename.xpm")),
		     "&Font...", KVCMenuWidgetFont);
  fgMenu->insertSeparator();
  fgMenu->insertItem(QPixmap(dir + QString("/color.xpm")),
		     "&Color...", KVCMenuWidgetForegroundColor);
  fgMenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
		     "&Reset to Defaults", KVCMenuWidgetForegroundNone);

  //
  // Background Menu
  //
  QPopupMenu* bgMenu = new QPopupMenu();
  bgMenu->insertItem(QPixmap(dir + QString("/frame.xpm")),
		     "Toggle &Frame", KVCMenuWidgetDrawFrame);
  bgMenu->insertSeparator();
  bgMenu->insertItem(QPixmap(dir + QString("/color.xpm")),
		     "&Color...", KVCMenuWidgetBackgroundColor);
  bgMenu->insertItem(QPixmap(dir + QString("/image.xpm")),
		     "&Image...", KVCMenuWidgetBackgroundPixmap);
  bgMenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
		     "&Reset to Defaults", KVCMenuWidgetBackgroundNone);

  //
  // Stacking order menu
  //
  QPopupMenu* stackMenu = new QPopupMenu();
  stackMenu->insertItem(QPixmap(dir + QString("/up.xpm")),
			  "Bring to Front", KVCMenuWidgetStackRaise);
  stackMenu->insertItem(QPixmap(dir + QString("/down.xpm")),
			  "Send to Back", KVCMenuWidgetStackLower);
  
  //
  // Widget menu
  //
  m_widgetMenu = new QPopupMenu();
  m_widgetMenu->insertItem(QPixmap(dir + "/settings.xpm"),
			   "&Properties...", KVCMenuWidgetProperties);
  m_widgetMenu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			   "&Rename...", KVCMenuWidgetRename);

  m_widgetMenu->insertSeparator();

  m_widgetMenu->insertItem("Function", functionMenu);

  m_widgetMenu->insertSeparator();

  m_widgetMenu->insertItem("Foreground", fgMenu);
  m_widgetMenu->insertItem("Background", bgMenu);
  m_widgetMenu->insertItem("Stacking Order", stackMenu, KVCMenuWidgetStack);
  m_widgetMenu->insertSeparator();

  m_widgetMenu->insertItem(QPixmap(dir + QString("/remove.xpm")),
			   "Remove", KVCMenuWidgetRemove);
  
  m_menuBar->insertItem("&Add", m_addMenu);
  m_menuBar->insertItem("&Tools", toolsMenu);
  m_menuBar->insertItem("&Widget", m_widgetMenu);

  connect(m_menuBar, SIGNAL(activated(int)), 
	  this, SLOT(slotMenuItemActivated(int)));
}

//
// Menu callback
//
void VirtualConsole::slotMenuItemActivated(int item)
{
  if (item >= KVCMenuAddMin && item <= KVCMenuAddMax)
    {
      parseAddMenu(item);
    }
  else if (item >= KVCMenuToolsMin && item <= KVCMenuToolsMax)
    {
      parseToolsMenu(item);
    }
  else if (item >= KVCMenuWidgetMin && item <= KVCMenuWidgetMax)
    {
      parseWidgetMenu(item);
    }
}

void VirtualConsole::parseAddMenu(int item)
{
  QWidget* parent = NULL;

  if (m_selectedWidget && 
      QString(m_selectedWidget->className()) == QString("VCFrame"))
    {
      parent = m_selectedWidget;
    }
  else
    {
      parent = m_drawArea;
    }

  switch(item)
    {
    case KVCMenuAddButton:
      {
	VCButton* b;
	b = new VCButton(parent);
	b->init();
	b->show();
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuAddSlider:
      {
	VCDockSlider* vcd = new VCDockSlider(parent);
	vcd->setBusID(KBusIDDefaultFade);
	vcd->init();
	vcd->resize(55, 200);
	vcd->show();
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuAddFrame:
      {
	VCFrame* w;
	w = new VCFrame(parent);
	w->init();
	w->show();
	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuAddLabel:
      {
 	VCLabel* p = NULL;
	p = new VCLabel(parent);
	p->init();
	p->show();
	_app->doc()->setModified(true);
      }
      break;

    default:
      break;
    }
}

void VirtualConsole::parseToolsMenu(int item)
{
  switch(item)
    {
    case KVCMenuToolsSettings:
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

    case KVCMenuToolsDefaultSliders:
      {
	if (m_dockArea->isHidden())
	  {
	    m_dockArea->show();
	  }
	else
	  {
	    m_dockArea->hide();
	  }

	_app->doc()->setModified(true);
      }
      break;

    case KVCMenuToolsPanic:
      {
	_app->slotPanic();
      }
      break;

    default:
      break;
    }
}

void VirtualConsole::parseWidgetMenu(int item)
{
  if (m_selectedWidget)
    {
      switch(item)
	{
	case KVCMenuWidgetRename:
	  {
	    m_renameEdit = new FloatingEdit(m_selectedWidget->parentWidget());
	    connect(m_renameEdit, SIGNAL(returnPressed()),
		    this, SLOT(slotRenameReturnPressed()));
	    m_renameEdit->setMinimumSize(60, 25);
	    m_renameEdit->setGeometry(m_selectedWidget->x() + 3, 
				      m_selectedWidget->y() + 3, 
				      m_selectedWidget->width() - 6, 
				      m_selectedWidget->height() - 6);
	    m_renameEdit->setText(m_selectedWidget->caption());
	    m_renameEdit->setFont(m_selectedWidget->font());
	    m_renameEdit->setSelection(0,m_selectedWidget->caption().length());
	    m_renameEdit->show();
	    m_renameEdit->setFocus();
	  }
	  break;

	case KVCMenuWidgetFont:
	  {
	    m_selectedWidget->setFont(QFontDialog::
				      getFont(0, m_selectedWidget->font()));
	    _app->doc()->setModified(true);
	  }
	  break;


	case KVCMenuWidgetForegroundColor:
	  {
	    QColor color = QColorDialog::
	      getColor(m_selectedWidget->paletteBackgroundColor(), this);
	    if (color.isValid())
	      {
		_app->doc()->setModified(true);
		m_selectedWidget->setPaletteForegroundColor(color);
	      }
	    
	    _app->doc()->setModified(true);
	  }
	  break;
	  
	case KVCMenuWidgetForegroundNone:
	  {
	    QColor bgc = m_selectedWidget->paletteBackgroundColor();

	    m_selectedWidget->unsetPalette();
	    m_selectedWidget->unsetFont();

	    m_selectedWidget->setPaletteBackgroundColor(bgc);

	    _app->doc()->setModified(true);
	  }
	  break;

	case KVCMenuWidgetBackgroundColor:
	  {
	    QColor newcolor = QColorDialog::
	      getColor(m_selectedWidget->paletteBackgroundColor(), this);
	    
	    if (newcolor.isValid() == true)
	      {
		m_selectedWidget->setPaletteBackgroundColor(newcolor);
		_app->doc()->setModified(true);
	      }
	  }
	  break;

	case KVCMenuWidgetBackgroundPixmap:
	  {
	    QString fileName = 
	      QFileDialog::getOpenFileName(m_selectedWidget->iconText(), 
					   QString("*.jpg *.png *.xpm *.gif"), 
					   this);
	    if (fileName.isEmpty() == false)
	      {
		QPixmap pm(fileName);
		m_selectedWidget->setPaletteBackgroundPixmap(pm);
		m_selectedWidget->setIconText(fileName);
		_app->doc()->setModified(true);
	      }
	  }
	  break;

	case KVCMenuWidgetBackgroundNone:
	  {
	    m_selectedWidget->unsetPalette();
	    _app->doc()->setModified(true);
	  }
	  break;
	  
	case KVCMenuWidgetStackRaise:
	  m_selectedWidget->raise();
	  break;
	  
	case KVCMenuWidgetStackLower:
	  m_selectedWidget->lower();
	  break;
	  
	case KVCMenuWidgetRemove:
	  {
	    if (QMessageBox::warning(this, "Remove Selected Widget",
				     "Are you sure?",
				     QMessageBox::Yes, QMessageBox::No)
		== QMessageBox::Yes)
	      {
		_app->doc()->setModified(true);
		delete m_selectedWidget;
		m_selectedWidget = NULL;
	      }
	  }
	  break;

	default:
	  QApplication::sendEvent(m_selectedWidget, new VCMenuEvent(item));
	  break;
	}
    }
}

void VirtualConsole::slotRenameReturnPressed()
{
  assert(m_selectedWidget);

  m_selectedWidget->setCaption(m_renameEdit->text());

  delete m_renameEdit;
  m_renameEdit = NULL;
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
      m_menuBar->setItemChecked(KVCMenuToolsDefaultSliders, false);
    }
  else
    {
      m_menuBar->setItemChecked(KVCMenuToolsDefaultSliders, true);
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
	      m_drawArea->init();
	      m_drawArea->setBottomFrame(true);
	      m_drawArea->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	      
	      m_layout->addWidget(m_drawArea, 1);
	      
	      m_drawArea->createContents(list);

	      m_drawArea->show();
	    }
	  else
	    {
	      VCFrame* w = new VCFrame(m_drawArea);
	      w->init();
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
      else if (*s == QString("DefaultSliders"))
	{
	  if (*list.next() == Settings::trueValue())
	    {
	      m_dockArea->show();
	    }
	  else
	    {
	      m_dockArea->hide();
	    }
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

  // Default sliders
  if (m_dockArea->isHidden())
    {
      s = QString("DefaultSliders = ") + Settings::falseValue() + QString("\n");
    }
  else
    {
      s = QString("DefaultSliders = ") + Settings::trueValue() + QString("\n");
    }
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


void VirtualConsole::setSelectedWidget(QWidget* w)
{
  if (m_selectedWidget)
    {
      QWidget* old = m_selectedWidget;
      m_selectedWidget = w;
      old->update();
    }
  else
    {
      m_selectedWidget = w;
    }

  if (m_selectedWidget)
    {
      m_selectedWidget->update();
    }
  else
    {
    }
}
