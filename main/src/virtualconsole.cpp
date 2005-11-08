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
#include "vcxypad.h"
#include "floatingedit.h"

#include <X11/Xlib.h>

extern App* _app;
extern QApplication* _qapp;

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
  m_editMenu = NULL;
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


//
// Init the whole view
//
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

//
// Init the left dock area
//
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


//
// Init the main drawing area
//
void VirtualConsole::initDrawArea()
{
  if (m_drawArea) delete m_drawArea;
  m_drawArea = new VCFrame(this);
  m_drawArea->setBottomFrame(true);

  // Add the draw area into the master (horizontal) layout
  m_layout->addWidget(m_drawArea, 1);
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
			"&Button", this, SLOT(slotAddButton()),
			0, KVCMenuAddButton);
  m_addMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			"&Slider", this, SLOT(slotAddSlider()),
			0, KVCMenuAddSlider);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"), 
			"&Frame", this, SLOT(slotAddFrame()),
			0, KVCMenuAddFrame);
  m_addMenu->insertItem(QPixmap(dir + "/frame.xpm"),
                        "&XY-Pad", this, SLOT(slotAddXYPad()),
                        0, KVCMenuAddXYPad);

  m_addMenu->setItemEnabled(KVCMenuAddXYPad, true);
  m_addMenu->insertItem(QPixmap(dir + "/rename.xpm"),
			"L&abel", this, SLOT(slotAddLabel()),
			0, KVCMenuAddLabel);

  //
  // Tools menu
  //
  m_toolsMenu = new QPopupMenu();
  m_toolsMenu->insertItem(QPixmap(dir + "/settings.xpm"), 
			  "&Settings...", this, SLOT(slotToolsSettings()),
			  0, KVCMenuToolsSettings);
  m_toolsMenu->insertItem(QPixmap(dir + "/slider.xpm"), 
			  "&Default Sliders", this, SLOT(slotToolsSliders()),
			  0, KVCMenuToolsSliders);
  m_toolsMenu->insertSeparator();
  m_toolsMenu->insertItem(QPixmap(dir + "/panic.xpm"), 
			  "&Panic!", this, SLOT(slotToolsPanic()),
			  0, KVCMenuToolsPanic);


  //
  // Foreground menu
  //
  QPopupMenu* fgMenu = new QPopupMenu();
  fgMenu->insertItem(QPixmap(dir + QString("/color.xpm")), 
		     "&Color...", this, SLOT(slotForegroundColor()),
		     0, KVCMenuForegroundColor);
  fgMenu->insertItem(QPixmap(dir + QString("/rename.xpm")), 
		     "&Font...", this, SLOT(slotForegroundFont()),
		     0, KVCMenuForegroundFont);
  fgMenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
		     "&Default", this, SLOT(slotForegroundNone()), 
		     0, KVCMenuForegroundNone);

  //
  // Background Menu
  //
  QPopupMenu* bgMenu = new QPopupMenu();
  bgMenu->insertItem(QPixmap(dir + QString("/color.xpm")), 
		     "&Color...", this, SLOT(slotBackgroundColor()),
		     0, KVCMenuBackgroundColor);
  bgMenu->insertItem(QPixmap(dir + QString("/image.xpm")), 
		     "&Image...", this, SLOT(slotBackgroundImage()),
		     0, KVCMenuBackgroundPixmap);
  bgMenu->insertItem(QPixmap(dir + QString("/fileclose.xpm")),
		     "&Default", this, SLOT(slotBackgroundNone()), 
		     0, KVCMenuBackgroundNone);
  bgMenu->insertSeparator();
  bgMenu->insertItem(QPixmap(dir + QString("/frame.xpm")),
		     "Toggle &Frame", this, SLOT(slotBackgroundFrame()),
		     0, KVCMenuBackgroundFrame);

  //
  // Stacking order menu
  //
  QPopupMenu* stackMenu = new QPopupMenu();
  stackMenu->insertItem(QPixmap(dir + QString("/up.xpm")),
			"&Raise", this, SLOT(slotStackingRaise()),
			0, KVCMenuStackingRaise);
  stackMenu->insertItem(QPixmap(dir + QString("/down.xpm")),
			"&Lower", this, SLOT(slotStackingLower()),
			0, KVCMenuStackingLower);
  
  //
  // Edit menu
  //
  m_editMenu = new QPopupMenu();
  m_editMenu->insertItem(QPixmap(dir + "/editcut.xpm"), 
			 "Cut", this, SLOT(slotEditCut()),
			 0, KVCMenuEditCut);
  
  m_editMenu->insertItem(QPixmap(dir + "/editcopy.xpm"),
			 "Copy", this, SLOT(slotEditCopy()),
			 0, KVCMenuEditCopy);
  
  m_editMenu->insertItem(QPixmap(dir + "/editpaste.xpm"),
			 "Paste", this, SLOT(slotEditPaste()),
			 0, KVCMenuEditPaste);
  
  m_editMenu->insertItem(QPixmap(dir + "/remove.xpm"), 
			 "Delete", this, SLOT(slotEditDelete()),
			 0, KVCMenuEditDelete);
  
  m_editMenu->insertSeparator();
  
  m_editMenu->insertItem(QPixmap(dir + "/settings.xpm"),
			 "&Properties...", this, SLOT(slotEditProperties()),
			 0, KVCMenuEditProperties);
  m_editMenu->insertItem(QPixmap(dir + QString("/rename.xpm")),
			 "&Rename...", this, SLOT(slotEditRename()),
			 0, KVCMenuEditRename);

  m_editMenu->setItemEnabled(KVCMenuEditCut, false);
  m_editMenu->setItemEnabled(KVCMenuEditCopy, false);
  m_editMenu->setItemEnabled(KVCMenuEditPaste, false);

  m_editMenu->insertSeparator();

  m_editMenu->insertItem("Foreground", fgMenu, KVCMenuForegroundMin);
  m_editMenu->insertItem("Background", bgMenu, KVCMenuBackgroundMin);
  m_editMenu->insertItem("Stacking Order", stackMenu, KVCMenuStackingMin);

  m_menuBar->insertItem("&Tools", m_toolsMenu);
  m_menuBar->insertItem("&Add", m_addMenu);
  m_menuBar->insertItem("&Edit", m_editMenu);
}


void VirtualConsole::slotAddButton()
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

  VCButton* b = new VCButton(parent);
  assert(b);
  b->init();
  b->show();

  _app->doc()->setModified(true);
}

void VirtualConsole::slotAddSlider()
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

  VCDockSlider* s = new VCDockSlider(parent);
  assert(s);
  s->setBusID(KBusIDDefaultFade);
  s->init();
  s->resize(55, 200);
  s->show();

  _app->doc()->setModified(true);
}

void VirtualConsole::slotAddFrame()
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

  VCFrame* f = new VCFrame(parent);
  assert(f);
  f->init();
  f->show();

  _app->doc()->setModified(true);
}

void VirtualConsole::slotAddXYPad()
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
assert(parent);
  VCXYPad* f = new VCXYPad(parent);
  assert(f);
  f->init();
  f->show();

  _app->doc()->setModified(true);
}


void VirtualConsole::slotAddLabel()
{
  QWidget* parent = NULL;

  if (m_selectedWidget && 
      (QString(m_selectedWidget->className()) == QString("VCFrame")
      || QString(m_selectedWidget->className()) == QString("VCXYPad")))
    {
      parent = m_selectedWidget;
    }
  else
    {
      parent = m_drawArea;
    }

  VCLabel* l = new VCLabel(parent);
  assert(l);
  l->init();
  l->show();

  _app->doc()->setModified(true);
}

void VirtualConsole::slotToolsSettings()
{
  VirtualConsoleProperties* p = new VirtualConsoleProperties(this);
  assert(p);
  p->init();
  if (p->exec() == QDialog::Accepted)
    {
      // Cache grid values so widgets don't have to get them
      // from settings each time (which is slow)
      m_gridEnabled = p->isGridEnabled();
      m_gridX = p->gridX();
      m_gridY = p->gridY();
    }

  delete p;
}

void VirtualConsole::slotToolsSliders()
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

void VirtualConsole::slotToolsPanic()
{
  // Panic button pressed: stop all running functions
  _app->slotPanic();
}

void VirtualConsole::slotEditCut()
{
  qDebug("Not implemented");
}

void VirtualConsole::slotEditCopy()
{
  qDebug("Not implemented");
}

void VirtualConsole::slotEditPaste()
{
  qDebug("Not implemented");
}

void VirtualConsole::slotEditDelete()
{
  if (m_selectedWidget)
    {
      if (m_selectedWidget->className() == QString("VCFrame") &&
	  ((VCFrame*) m_selectedWidget)->isBottomFrame() == true)
	{
	  //
	  // Bottom frame should not be deleted!
	  //
	  // Maybe the menu item should be removed, too, but it's
	  // a bit complicated. Darn shared-menu-system-I-had-to-do-one-day
	  //
	}
      else if (QMessageBox::warning(this, "Remove Selected Widget",
				    "Are you sure?", QMessageBox::Yes,
				    QMessageBox::No) == QMessageBox::Yes)
	{
	  _app->doc()->setModified(true);
	  delete m_selectedWidget;
	  m_selectedWidget = NULL;
	}
    }
}

void VirtualConsole::slotEditProperties()
{
  if (m_selectedWidget)
    {
      //
      // By sending an event we don't need to know
      // what class m_selectedWidget is when this is called.
      // sendEvent is also synchronous call, so this is almost
      // the same thing as calling the function directly.
      //
      QApplication::sendEvent(m_selectedWidget, 
			      new VCMenuEvent(KVCMenuEditProperties));
    }
}

void VirtualConsole::slotEditRename()
{
  if (m_selectedWidget)
    {
      //
      // Delete if there already is a rename edit somewhere
      // (a previous rename action...?)
      //
      if (m_renameEdit) delete m_renameEdit;
      
      // Create new QLineEdit widget.
      m_renameEdit = new FloatingEdit(m_selectedWidget->parentWidget());

      // See the two function declarations below
      connect(m_renameEdit, SIGNAL(returnPressed()),
	      this, SLOT(slotEditRenameReturnPressed()));
      connect(m_renameEdit, SIGNAL(cancelled()),
	      this, SLOT(slotEditRenameCancelled()));

      //
      // Draw a QLineEdit the size of the selected widget and start rename.
      // (might look ugly with big widgets, but who cares :)
      //
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
}

void VirtualConsole::slotEditRenameReturnPressed()
{
  // Rename action was ok
  if (m_selectedWidget)
    {
      m_selectedWidget->setCaption(m_renameEdit->text());
    }

  assert(m_renameEdit);
  m_renameEdit->deleteLater(); // QT 3.1 crashes with normal delete
  m_renameEdit = NULL;

  _app->doc()->setModified(true);
}


void VirtualConsole::slotEditRenameCancelled()
{
  // Rename action was cancelled
  assert(m_renameEdit);
  m_renameEdit->deleteLater(); // QT 3.1 crashes with normal delete
  m_renameEdit = NULL;
}


void VirtualConsole::slotForegroundFont()
{
  if (m_selectedWidget)
    {
      m_selectedWidget->setFont(QFontDialog::
				getFont(0, m_selectedWidget->font()));
      _app->doc()->setModified(true);
    }
}

void VirtualConsole::slotForegroundColor()
{
  if (m_selectedWidget)
    {
      QColor color = 
	QColorDialog::getColor(m_selectedWidget->paletteForegroundColor(), 
			       this);

      if (color.isValid())
	{
	  _app->doc()->setModified(true);
	  m_selectedWidget->setPaletteForegroundColor(color);
	}
  
      _app->doc()->setModified(true);
    }
}

void VirtualConsole::slotForegroundNone()
{
  if (m_selectedWidget)
    {
      // Save the background color (note that we are resetting
      // the foreground here!)
      QColor bgc = m_selectedWidget->paletteBackgroundColor();
      
      // Reset palette and font
      m_selectedWidget->unsetPalette();
      m_selectedWidget->unsetFont();
      
      // Now set the backround color again because we only reset
      // foreground options.
      m_selectedWidget->setPaletteBackgroundColor(bgc);
      
      _app->doc()->setModified(true);
    }
}

void VirtualConsole::slotBackgroundColor()
{
  if (m_selectedWidget)
    {
      QColor newcolor = QColorDialog::
	getColor(m_selectedWidget->paletteBackgroundColor(), this);
      
      if (newcolor.isValid() == true)
	{
	  m_selectedWidget->setPaletteBackgroundColor(newcolor);
	  _app->doc()->setModified(true);
	}
    }
}

void VirtualConsole::slotBackgroundImage()
{
  if (m_selectedWidget)
    {
      QString fileName = 
	QFileDialog::getOpenFileName(m_selectedWidget->iconText(), 
				     QString("*.jpg *.png *.xpm *.gif"), 
				     this);
      if (fileName.isEmpty() == false)
	{
	  // Set the selected pixmap as bg image and icon.
	  // Some QT styles don't know how to display bg pixmaps, hence icon.
	  QPixmap pm(fileName);
	  m_selectedWidget->setPaletteBackgroundPixmap(pm);
	  m_selectedWidget->setIconText(fileName);
	  _app->doc()->setModified(true);
	}
    }
}

void VirtualConsole::slotBackgroundNone()
{
  if (m_selectedWidget)
    {
      // Save the foreground color (note that we are resetting
      // the background here!)
      QColor fgc(m_selectedWidget->paletteForegroundColor());
      
      // Reset palette
      m_selectedWidget->unsetPalette();

      // Now set the foreground color again
      m_selectedWidget->setPaletteForegroundColor(fgc);

      _app->doc()->setModified(true);
    }
}

void VirtualConsole::slotBackgroundFrame()
{
  if (m_selectedWidget)
    {
      QApplication::sendEvent(m_selectedWidget, 
			      new VCMenuEvent(KVCMenuBackgroundFrame));
    }
}

void VirtualConsole::slotStackingRaise()
{
  if (m_selectedWidget)
    {
      m_selectedWidget->raise();
      _app->doc()->setModified(true);
    }
}

void VirtualConsole::slotStackingLower()
{
  if (m_selectedWidget)
    {
      m_selectedWidget->lower();
      _app->doc()->setModified(true);
    }
}


void VirtualConsole::slotDockAreaHidden(bool areaHidden)
{
  if (areaHidden == true)
    {
      m_menuBar->setItemChecked(KVCMenuToolsSliders, false);
    }
  else
    {
      m_menuBar->setItemChecked(KVCMenuToolsSliders, true);
    }
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

  if (_app->mode() == App::Operate)
    {
      // Don't allow editing in operate mode
      m_editMenu->setEnabled(false);
      m_addMenu->setEnabled(false);
      m_toolsMenu->setItemEnabled(KVCMenuToolsSettings, false);
    }
  else
    {
      // Allow editing in design mode
      m_editMenu->setEnabled(true);
      m_addMenu->setEnabled(true);
      m_toolsMenu->setItemEnabled(KVCMenuToolsSettings, true);
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
    //helphelphelp
      if (QString(it.current()->className()) == QString("VCFrame") || 
	  QString(it.current()->className()) == QString("VCXYPad"))
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
      else if (*s == QString("VCXYPad"))
	{
	  VCXYPad* w = new VCXYPad(m_drawArea);
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
	  else if (*s == QString("VCXYPad"))
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
