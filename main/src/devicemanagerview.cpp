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
#include "virtualconsole.h"
#include "newdevice.h"

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

extern App* _app;

// List view column numbers
const int KColumnName ( 0 );
const int KColumnID   ( 1 );

// List view item menu callback id's
const int KMenuItemAdd        ( 0 );
const int KMenuItemRemove     ( 1 );
const int KMenuItemProperties ( 2 );
const int KMenuItemMonitor    ( 3 );
const int KMenuItemConsole    ( 4 );

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
  setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());

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
  m_dockArea->setFixedHeight(30);

  // Add the dock area to the top of the vertical layout
  m_layout->addWidget(m_dockArea);

  //
  // Add a toolbar to the dock area
  //
  m_toolbar = new QToolBar("Device Manager", _app, m_dockArea);

  m_addButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/addoutputdevice.xpm")), 
		    "Add New Output Device", 0, this,
		    SLOT(slotAdd()), m_toolbar);

  m_removeButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/remove.xpm")), 
		    "Remove Current Selection", 0, this,
		    SLOT(slotRemove()), m_toolbar);

  m_propertiesButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/settings.xpm")), 
		    "Properties", 0, this,
		    SLOT(slotProperties()), m_toolbar);

  m_toolbar->addSeparator();

  m_monitorButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/monitor.xpm")), 
		    "Monitor Device", 0, this,
		    SLOT(slotMonitor()), m_toolbar);

  m_consoleButton = 
    new QToolButton(QIconSet(QPixmap(dir + "/console.xpm")), 
		    "View Console", 0, this,
		    SLOT(slotConsole()), m_toolbar);
}


//
// Set up the actual data view
//
void DeviceManagerView::initDataView()
{
  // Create a splitter to divide list view and text view
  m_splitter = new QSplitter(this);
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
							       

  // Create the text view
  m_textView = new QTextView(m_splitter);
  m_splitter->setResizeMode(m_textView, QSplitter::Auto);
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
      currentId = m_listView->currentItem()->text(KColumnID).toULong();
    }

  m_listView->clear();

  // Add output devices
  for (t_device_id i = 0; 
       i < (t_device_id) _app->doc()->deviceList()->count(); i++)
    {
      Device* dev = _app->doc()->deviceList()->at(i);
      newItem = new QListViewItem(m_listView, dev->name());
      
      // ID column
      id.setNum(dev->id());
      newItem->setText(KColumnID, id);

      // Select this if it was selected before update
      if (currentId == dev->id())
	{
	  m_listView->setSelected(newItem, true);
	}
    }
}


//
// Add a device
//
void DeviceManagerView::slotAdd()
{
  NewDevice* ndlg = new NewDevice(_app);
  
  while (1)
    {
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
	  DeviceClass* dc = _app->searchDeviceClass(manufacturer,model);
	  ASSERT(dc);

	  if (dc->channels()->count() == 0)
	    {
	      QString msg(QString("No channels specified for device class \"")
			  + manufacturer + QString(" ") + model
			  + QString("\".\n"));
	      QMessageBox::warning(this, KApplicationNameShort, msg);
	      continue;
	    }

	  // Add new device
	  _app->doc()->addDevice(new Device(address, dc, name));
	  break;
	}
      else
	{
	  break;
	}
    }

  delete ndlg;
}


//
// Remove a device
//
void DeviceManagerView::slotRemove()
{
  QListViewItem* item = m_listView->currentItem();
  QListViewItem* itemAbove = item->itemAbove();

  // Get the device id
  t_device_id id = item->text(KColumnID).toInt();
  
  // Display a warning
  QString msg;
  msg = ("Do you want to remove device \"");
  msg += item->text(KColumnName).latin1() + QString("\"?");
  if (QMessageBox::warning(this, KApplicationNameShort, msg,
			   QMessageBox::Yes, QMessageBox::No) 
      == QMessageBox::No)
    {
      return;
    }
  else
    {
      Device* device = _app->doc()->searchDevice(id);
      
      ASSERT(device);
      
      _app->doc()->removeDevice(device);
      
      // Select the item above the removed item
      m_listView->setSelected(itemAbove, true);
    }
}


//
// View Properties
//
void DeviceManagerView::slotProperties()
{
  QListViewItem* item = m_listView->currentItem();

  t_device_id id = item->text(KColumnID).toInt();
  Device* device = _app->doc()->searchDevice(id);
  
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
  Device* device = _app->doc()->searchDevice(id);
  
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
  Device* device = _app->doc()->searchDevice(id);
  
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
      m_addButton->setEnabled(false);
      m_removeButton->setEnabled(false);
      m_propertiesButton->setEnabled(false);
      m_monitorButton->setEnabled(false);
      m_consoleButton->setEnabled(false);

      m_textView->setText(QString::null);
    }
  else
    {
      // Set the text view's contents
      Device* dev = _app->doc()->searchDevice(item->text(KColumnID).toInt());
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
	}
      else
	{
	  m_addButton->setEnabled(false);
	  m_removeButton->setEnabled(false);
	  m_propertiesButton->setEnabled(false);
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
  
  menu->insertItem("Add...", KMenuItemAdd);
  menu->insertItem("Remove", KMenuItemRemove);
  menu->insertItem("Properties...", KMenuItemProperties);
  menu->insertSeparator();
  menu->insertItem("View Console...", KMenuItemConsole);
  menu->insertItem("View Monitor...", KMenuItemMonitor);
  
  if ( _app->mode() == App::Operate || !item )
    {
      // Operate mode, remove and edit impossible
      menu->setItemEnabled(KMenuItemRemove, false);
      menu->setItemEnabled(KMenuItemProperties, false);

      // No item selected, unable to view console either
      if (!item)
	{
	  menu->setItemEnabled(KMenuItemConsole, false);
	  menu->setItemEnabled(KMenuItemMonitor, false);
	}
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

    case KMenuItemProperties:
      slotProperties();
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
