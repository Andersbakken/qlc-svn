/*
  Q Light Controller
  devicemanagerview.cpp
  
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

#include "devicemanagerview.h"
#include "settings.h"
#include "app.h"
#include "doc.h"
#include "configkeys.h"
#include "newdevice.h"
#include "chaser.h"
#include "functioncollection.h"
#include "scene.h"
#include "sequence.h"

#include <qwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qdockarea.h>
#include <qtextview.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <assert.h>

extern App* _app;

// List view column numbers
const int KColumnName ( 0 );
const int KColumnID   ( 1 );

// List view item menu callback id's
const int KMenuItemAdd        ( 0 );
const int KMenuItemRemove     ( 1 );
const int KMenuItemProperties ( 2 );
const int KMenuItemRename     ( 3 );
const int KMenuItemMonitor    ( 4 );
const int KMenuItemConsole    ( 5 );
const int KMenuItemClone      ( 6 );

const QString KEY_DEVICE_MANAGER_OPEN  (  "DeviceManagerOpen" );
const QString KEY_DEVICE_MANAGER_X     ( "DeviceManagerRectX" );
const QString KEY_DEVICE_MANAGER_Y     ( "DeviceManagerRectY" );
const QString KEY_DEVICE_MANAGER_W     ( "DeviceManagerRectW" );
const QString KEY_DEVICE_MANAGER_H     ( "DeviceManagerRectH" );

//
// Constructor
//
DeviceManagerView::DeviceManagerView(QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  m_layout = NULL;
  m_dockArea = NULL;
  m_toolbar = NULL;
  m_splitter = NULL;
  m_listView = NULL;
  m_textView = NULL;
}


//
// Destructor
//
DeviceManagerView::~DeviceManagerView()
{
  QString config;

  if (isShown())
    {
      config = Settings::trueValue();
    }
  else
    {
      config = Settings::falseValue();
    }

  _app->settings()->set(KEY_DEVICE_MANAGER_OPEN, config);

  //
  // Save rect
  //
  _app->settings()->set(KEY_DEVICE_MANAGER_X, rect().x());
  _app->settings()->set(KEY_DEVICE_MANAGER_Y, rect().y());
  _app->settings()->set(KEY_DEVICE_MANAGER_W, rect().width());
  _app->settings()->set(KEY_DEVICE_MANAGER_H, rect().height());
}


//
// When closing this, send a notification (to app)
//
void DeviceManagerView::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}


//
// Initialize this widget
//
void DeviceManagerView::initView()
{
  // Create a vertical layout to this widget
  m_layout = new QVBoxLayout(this);

  // Init the title and icon
  initTitle();

  // Set up toolbar
  initToolBar();

  // Init the device view and text view
  initDataView();

  // Connect to know when to enable/disable buttons
  connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));

  // Update view
  update();

  //
  // Set widget proportions
  //
  QString x, y, w, h;
  _app->settings()->get(KEY_DEVICE_MANAGER_X, x);
  _app->settings()->get(KEY_DEVICE_MANAGER_Y, y);
  _app->settings()->get(KEY_DEVICE_MANAGER_W, w);
  _app->settings()->get(KEY_DEVICE_MANAGER_H, h);
  if (w == 0 || h == 0)
    {
      setGeometry(0, 0, 350, 200);
    }
  else
    {
      setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());
    }

  // Check if DM should be open
  QString config;
  _app->settings()->get(KEY_DEVICE_MANAGER_OPEN, config);
  if (config == Settings::trueValue())
    {
      _app->slotViewDeviceManager();
    }
  else
    {
      hide();
      _app->slotDeviceManagerViewClosed();
    }
}


//
// Set an icon to this widget
//
void DeviceManagerView::initTitle()
{
  // Set the name
  setCaption(QString("Device Manager"));

  // Initial size, should be saved to settings on the fly instead
  resize(500, 300);

  // Get the system directory to get to pixmaps
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  // Set an icon
  setIcon(dir + QString("/device.xpm"));
}


//
// Set up a toolbar
//
void DeviceManagerView::initToolBar()
{
  // Get the system directory to get to pixmaps
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  // Create a dock area for the toolbar
  m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
  m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  // Add the dock area to the top of the vertical layout
  m_layout->addWidget(m_dockArea);

  //
  // Add a toolbar to the dock area
  //
  m_toolbar = new QToolBar("Device Manager", _app, m_dockArea);

  m_addButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/addoutputdevice.xpm")), 
		    "Add", 0, this,
		    SLOT(slotAdd()), m_toolbar);
  m_addButton->setUsesTextLabel(true);
  QToolTip::add( m_addButton,  "Add New Device");  
  
  m_cloneButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/editcopy.xpm")), 
		    "Clone", 0, this,
		    SLOT(slotClone()), m_toolbar);
  m_cloneButton->setUsesTextLabel(true);
  QToolTip::add( m_cloneButton, "Clone a device and its functions" );
  
  m_removeButton =
    new QToolButton(QIconSet(QPixmap(dir + "/remove.xpm")),
		    "Remove", 0, this,
		    SLOT(slotRemove()), m_toolbar);
  m_removeButton->setUsesTextLabel(true);
  QToolTip::add( m_removeButton, "Remove Current Selection");
  
  m_propertiesButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/settings.xpm")), 
		    "Properties", 0, this,
		    SLOT(slotProperties()), m_toolbar);
  m_propertiesButton->setUsesTextLabel(true);
  QToolTip::add( m_propertiesButton,   "Device properties"); 

  m_toolbar->addSeparator();

  m_monitorButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/monitor.xpm")), 
		    "Monitor Device", 0, this,
		    SLOT(slotMonitor()), m_toolbar);
  m_monitorButton->setUsesTextLabel(true);
  QToolTip::add(m_monitorButton, "Monitor Device");

  m_consoleButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/console.xpm")), 
		    "View Console", 0, this,
		    SLOT(slotConsole()), m_toolbar);
  m_consoleButton->setUsesTextLabel(true);
  QToolTip::add( m_consoleButton, "View Console");
}


//
// Set up the actual data view
//
void DeviceManagerView::initDataView()
{
  // Create a splitter to divide list view and text view
  m_splitter = new QSplitter(this);
  m_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_layout->addWidget(m_splitter);

  // Create the list view
  m_listView = new QListView(m_splitter);
  m_listView->setResizeMode(QListView::AllColumns);
  m_splitter->setResizeMode(m_listView, QSplitter::Auto);

  m_listView->setMultiSelection(false);
  m_listView->setAllColumnsShowFocus(true);
  m_listView->setSorting(KColumnName, true);
  m_listView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  
  m_listView->header()->setClickEnabled(true);
  m_listView->header()->setResizeEnabled(true);
  m_listView->header()->setMovingEnabled(false);

  m_listView->addColumn("Devices");
  m_listView->setColumnWidth(KColumnName, 200);

  connect(m_listView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));

  connect(m_listView, SIGNAL(rightButtonClicked(QListViewItem*,
						const QPoint&, int)),
	  this, SLOT(slotRightButtonClicked(QListViewItem*,
					    const QPoint&, int)));

  connect(m_listView, SIGNAL(itemRenamed(QListViewItem*, int)),
	  this, SLOT(slotItemRenamed(QListViewItem*, int)));

  // Create the text view
  m_textView = new QTextView(m_splitter);
  m_splitter->setResizeMode(m_textView, QSplitter::Auto);

  slotSelectionChanged(NULL);
}


//
// Update List View
//
void DeviceManagerView::slotUpdate()
{
  t_device_id currentId = 0;
  QListViewItem* newItem = NULL;

  QString id;

  if (m_listView->currentItem() != NULL)
    {
      currentId = m_listView->currentItem()->text(KColumnID).toInt();
    }

  m_listView->clear();

  // Add output devices
  for (t_device_id i = 0; i < KDeviceArraySize; i++)
    {
      Device* dev = _app->doc()->device(i);
      if (!dev)
	{
	  continue;
	}
      else
	{
	  newItem = new QListViewItem(m_listView, dev->name());
	  
	  // ID column
	  id.setNum(dev->id());
	  newItem->setText(KColumnID, id);
	  
	  // Enable rename
	  newItem->setRenameEnabled(KColumnName, true);
	  
	  // Select this if it was selected before update
	  if (currentId == dev->id())
	    {
	      m_listView->setSelected(newItem, true);
	    }
	}
    }
}


//
// Add a device
//
void DeviceManagerView::slotAdd()
{
  NewDevice* ndlg = new NewDevice(_app);
  
  if (ndlg->exec() == QDialog::Accepted)
    {
      int address = ndlg->address();
      QString name = ndlg->name();
      QString manufacturer = ndlg->manufacturer();
      QString model = ndlg->model();
      
      if (name.stripWhiteSpace() == QString::null)
	{
	  name = QString("Noname");
	}
      
      // Search the actual device class instance to be associated
      // with the new device
      DeviceClass* dc = _app->searchDeviceClass(manufacturer, model);
      assert(dc);

      // Add new device
      _app->doc()->newDevice(dc, name, address);
    }

  delete ndlg;
}


//
// Clone a device
//
void DeviceManagerView::slotClone()
{
  QListViewItem* item = m_listView->currentItem();

  // Get the device id and name
  t_device_id old_id = item->text(KColumnID).toInt();

   DeviceClass* dc = _app->doc()->device(old_id)->deviceClass();
   assert(dc);

   QString new_name;
   new_name = item->text(KColumnName);
   new_name += "_new";

   // Add new device
   Device* d = _app->doc()->newDevice(dc, new_name , 0);
   assert(d);

   for (t_function_id id = 0; id < KFunctionArraySize; id++)
   {
      Function* f = _app->doc()->function(id);
      if (!f)
	{
	  continue;
	}

       //copy only functions that belong to parent device
       if (f->device() == old_id)
         {
           copyFunction(f,d);
         }
   }
   
   m_listView->setCurrentItem(m_listView->findItem(new_name,0,0));
   slotProperties();
}


//
// Remove a device
//
void DeviceManagerView::slotRemove()
{
  QListViewItem* item = m_listView->currentItem();
  
  // Get the device id
  t_device_id id = item->text(KColumnID).toInt();
  
  // Display a warning
  QString msg;
  msg = ("Do you want to remove device \"");
  msg += item->text(KColumnName) + QString("\"?");
  if (QMessageBox::warning(this, KApplicationNameShort, msg,
			   QMessageBox::Yes, QMessageBox::No) 
      == QMessageBox::No)
    {
      return;
    }
  else
    {
      Device* device = _app->doc()->device(id);
      
      ASSERT(device);
      
      _app->doc()->deleteDevice(id);
    }

  if (m_listView->currentItem())
    {
      m_listView->setSelected(m_listView->currentItem(), true);
    }
  else
    {
      slotSelectionChanged(NULL);
    }
}


//
// View Properties
//
void DeviceManagerView::slotProperties()
{
  QListViewItem* item = m_listView->currentItem();

  t_device_id id = item->text(KColumnID).toInt();
  Device* device = _app->doc()->device(id);
  
  ASSERT(device);

  device->viewProperties();

  item->setText(KColumnName, device->name());
  slotSelectionChanged(item);
}


//
// View Monitor
//
void DeviceManagerView::slotMonitor()
{
  QListViewItem* item = m_listView->currentItem();

  t_device_id id = item->text(KColumnID).toInt();
  Device* device = _app->doc()->device(id);
  
  ASSERT(device);
  
  device->viewMonitor();
}


//
// View Console
//
void DeviceManagerView::slotConsole()
{
  QListViewItem* item = m_listView->currentItem();

  t_device_id id = item->text(KColumnID).toInt();
  Device* device = _app->doc()->device(id);
  
  ASSERT(device);
  
  device->viewConsole();
}


//
// The main operating mode has changed
//
void DeviceManagerView::slotModeChanged()
{
  slotSelectionChanged(m_listView->currentItem());
}


//
// Enable / disable toolbar buttons according to selected item & type
//
void DeviceManagerView::slotSelectionChanged(QListViewItem* item)
{
  if (item == NULL)
    {
      // Disable all
      if (_app->mode() == App::Design)
	{
	  m_addButton->setEnabled(true);
	}
      else
	{
	  m_addButton->setEnabled(false);
	}
      m_removeButton->setEnabled(false);
      m_propertiesButton->setEnabled(false);
      m_monitorButton->setEnabled(false);
      m_consoleButton->setEnabled(false);
      m_cloneButton->setEnabled(false);
      
      QString text;

      text = QString("<HTML><BODY>");
      text += QString("<H1>No Devices</H1>");
      text += QString("Click \"Add\" on the toolbar to add a new device.");
      text += QString("</BODY></HTML>");
      
      m_textView->setText(text);
    }
  else
    {
      // Set the text view's contents
      Device* dev = _app->doc()->device(item->text(KColumnID).toInt());
      ASSERT(dev);
      m_textView->setText(dev->infoText());

      // Enable console & monitor always
      m_consoleButton->setEnabled(true);
      m_monitorButton->setEnabled(true);
      
      if (_app->mode() == App::Design)
	{
	  m_addButton->setEnabled(true);
	  m_removeButton->setEnabled(true);
	  m_propertiesButton->setEnabled(true);
	  m_cloneButton->setEnabled(true);
	}
      else
	{
	  m_addButton->setEnabled(false);
	  m_removeButton->setEnabled(false);
	  m_propertiesButton->setEnabled(false);
	  m_cloneButton->setEnabled(false);
	}
    }
}


//
// Right mouse button has been pressed in the listview
//
void DeviceManagerView::slotRightButtonClicked(QListViewItem* item, 
					       const QPoint& point, int col)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->setCheckable(false);
  
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  menu->insertItem(QPixmap(dir + "/addoutputdevice.xpm"),
		   "Add...", KMenuItemAdd);
  menu->insertItem(QPixmap(dir + "/editcopy.xpm"),
                   "Clone...", KMenuItemClone);
  menu->insertItem(QPixmap(dir + "/remove.xpm"),
		   "Remove", KMenuItemRemove);
  menu->insertItem(QPixmap(dir + "/settings.xpm"),
		   "Properties...", KMenuItemProperties);
  menu->insertItem(QPixmap(dir + "/rename.xpm"),
		   "Rename...", KMenuItemRename);
  menu->insertSeparator();
  menu->insertItem(QPixmap(dir + "/monitor.xpm"),
		   "View Monitor...", KMenuItemMonitor);
  menu->insertItem(QPixmap(dir + "/console.xpm"),
		   "View Console...", KMenuItemConsole);
  
  if (_app->mode() == App::Operate)
    {
      // Operate mode, remove and edit impossible
      menu->setItemEnabled(KMenuItemAdd, false);
      menu->setItemEnabled(KMenuItemRemove, false);
      menu->setItemEnabled(KMenuItemProperties, false);
      menu->setItemEnabled(KMenuItemRename, false);
      menu->setItemEnabled(KMenuItemClone, false);
    }
  
  // No item selected, unable to do other things either
  if (!item)
    {
      menu->setItemEnabled(KMenuItemRemove, false);
      menu->setItemEnabled(KMenuItemConsole, false);
      menu->setItemEnabled(KMenuItemMonitor, false);
      menu->setItemEnabled(KMenuItemProperties, false);
      menu->setItemEnabled(KMenuItemRename, false);
      menu->setItemEnabled(KMenuItemClone, false);
    }

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotMenuCallBack(int)));
  menu->exec(point, 0);
  delete menu;
}


//
// Listview item popup menu callback
//
void DeviceManagerView::slotMenuCallBack(int item)
{
  switch (item)
    {
    case KMenuItemAdd:
      slotAdd();
      break;

    case KMenuItemRemove:
      slotRemove();
      break;

    case KMenuItemClone:
      slotClone();
      break;

    case KMenuItemProperties:
      slotProperties();
      break;

    case KMenuItemRename:
      slotRename();
      break;

    case KMenuItemConsole:
      slotConsole();
      break;

    case KMenuItemMonitor:
      slotMonitor();
      break;

    default:
      break;
    }
}


void DeviceManagerView::slotRename()
{
  QListViewItem* item = m_listView->currentItem();
  item->startRename(KColumnName);
}

//
// An item has been renamed
//
void DeviceManagerView::slotItemRenamed(QListViewItem* item, int col)
{
  Device* dev = _app->doc()->device(item->text(KColumnID).toInt());
  ASSERT(dev);

  dev->setName(item->text(KColumnName));
  m_textView->setText(dev->infoText());
}



void DeviceManagerView::copyFunction(Function* function, Device* device)
{
  switch(function->type())
    {
    case Function::Scene:
      {
	Scene* scene = static_cast<Scene*>
	  (_app->doc()->newFunction(Function::Scene));

	scene->copyFrom(static_cast<Scene*> (function), device->id());
      }
      break;

    case Function::Chaser:
      {
	Chaser* chaser = static_cast<Chaser*>
	  (_app->doc()->newFunction(Function::Chaser));

	chaser->copyFrom(static_cast<Chaser*> (function));
	assert(device == NULL);
      }
      break;

    case Function::Collection:
      {
	FunctionCollection* fc = static_cast<FunctionCollection*>
	  (_app->doc()->newFunction(Function::Collection));

	fc->copyFrom(static_cast<FunctionCollection*> (function));
	assert(device == NULL);
      }
      break;

    case Function::Sequence:
      {
	Sequence* sequence = static_cast<Sequence*> 
	  (_app->doc()->newFunction(Function::Sequence));
	
	sequence->copyFrom(static_cast<Sequence*> (function), device->id());
      }
      break;

    default:
      break;
    }
}

