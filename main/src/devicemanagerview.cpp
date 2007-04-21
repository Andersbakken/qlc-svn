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

#include "common/settings.h"
#include "common/logicalchannel.h"
#include "common/capability.h"
#include "devicemanagerview.h"
#include "app.h"
#include "doc.h"
#include "configkeys.h"
#include "newdevice.h"
#include "chaser.h"
#include "functioncollection.h"
#include "scene.h"
#include "sequence.h"
#include "efx.h"
#include "consolechannel.h"

extern App* _app;

// List view column numbers
const int KColumnUniverse ( 0 );
const int KColumnAddress  ( 1 );
const int KColumnName     ( 2 );
const int KColumnID       ( 3 );

// List view item menu callback id's
const int KMenuItemAdd          ( 0 );
const int KMenuItemRemove       ( 1 );
const int KMenuItemProperties   ( 2 );
const int KMenuItemConsole      ( 3 );
const int KMenuItemClone        ( 4 );
const int KMenuItemAutoFunction ( 5 );

const int KDefaultWidth  ( 600 );
const int KDefaultHeight ( 300 );

//
// Constructor
//
DeviceManagerView::DeviceManagerView(QWidget* parent, const char* name)
	: QWidget(parent, name),

	m_layout   ( NULL ),
	m_dockArea ( NULL ),
	m_toolbar  ( NULL ),
	m_splitter ( NULL ),
	m_listView ( NULL ),
	m_textView ( NULL )
{
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

	// Save the splitter position
	config.truncate(0);
	QValueList<int> list = m_splitter->sizes();
	QValueList<int>::Iterator it = list.begin();
	_app->settings()->set(KEY_DEVICE_MANAGER_SPLITTER, *it);
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

	// Update the list of devices
	updateView();

	//
	// Set widget proportions
	//
	QString x, y, w, h;

	if (_app->settings()->get(KEY_DEVICE_MANAGER_X, x) == -1
		|| x.toInt() <= 0 || x.toInt() >= _app->width())
	{
		x.setNum(0);
	}

	if (_app->settings()->get(KEY_DEVICE_MANAGER_Y, y) == -1
		|| y.toInt() <= 0 || y.toInt() >= _app->height())
	{
		y.setNum(0);
	}

	if (_app->settings()->get(KEY_DEVICE_MANAGER_W, w) == -1
		|| w.toInt() <= 0 || w.toInt() >= _app->width())
	{
		w.setNum(KDefaultWidth);
	}

	if (_app->settings()->get(KEY_DEVICE_MANAGER_H, h) == -1
		|| h.toInt() <= 0 || w.toInt() >= _app->height())
	{
		h.setNum(KDefaultHeight);
	}

	setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());

	// Set the splitter position
	QValueList<int> list;
	if (_app->settings()->get(KEY_DEVICE_MANAGER_SPLITTER, w) != -1
		&& w.toInt() >= 0)
	{
		list.append(w.toInt());
		list.append(width() - w.toInt());
	}
	else
	{
		list.append(width() / 2);
		list.append(width() / 2);
	}

	m_splitter->setSizes(list);
}


//
// Set an icon to this widget
//
void DeviceManagerView::initTitle()
{
	// Set the name
	setCaption(QString("Device Manager"));

	// Set an icon
	setIcon(QString(PIXMAPS) + QString("/fixture.png"));
}


//
// Set up a toolbar
//
void DeviceManagerView::initToolBar()
{
  // Create a dock area for the toolbar
  m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
  m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  // Add the dock area to the top of the vertical layout
  m_layout->addWidget(m_dockArea);

  //
  // Add a toolbar to the dock area
  //
  m_toolbar = new QToolBar("Device Manager", _app, m_dockArea);
  m_toolbar->setMovingEnabled(false);

  m_addButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/wizard.png"))),
		    "Add", 0, this,
		    SLOT(slotAdd()), m_toolbar);
  m_addButton->setUsesTextLabel(true);
  QToolTip::add( m_addButton,  "Add a new device");

  m_cloneButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/editcopy.png"))),
		    "Clone", 0, this,
		    SLOT(slotClone()), m_toolbar);
  m_cloneButton->setUsesTextLabel(true);
  QToolTip::add( m_cloneButton, "Clone a device and its functions" );

  m_removeButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/editdelete.png"))),
		    "Remove", 0, this,
		    SLOT(slotRemove()), m_toolbar);
  m_removeButton->setUsesTextLabel(true);
  QToolTip::add( m_removeButton, "Remove current device");

  m_toolbar->addSeparator();

  m_propertiesButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/configure.png"))),
		    "Properties", 0, this,
		    SLOT(slotProperties()), m_toolbar);
  m_propertiesButton->setUsesTextLabel(true);
  QToolTip::add( m_propertiesButton,   "Device properties");

  m_consoleButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/console.png"))),
		    "Console", 0, this,
		    SLOT(slotConsole()), m_toolbar);
  m_consoleButton->setUsesTextLabel(true);
  QToolTip::add( m_consoleButton, "View device console");
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
	m_splitter->setResizeMode(m_listView, QSplitter::Auto);
	
	m_listView->setMultiSelection(false);
	m_listView->setAllColumnsShowFocus(true);
	m_listView->setSorting(KColumnAddress, true);
	m_listView->setShowSortIndicator(true);
	m_listView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	m_listView->header()->setClickEnabled(true);
	m_listView->header()->setResizeEnabled(true);
	m_listView->header()->setMovingEnabled(false);
	
	m_listView->addColumn("Universe");
	m_listView->addColumn("Address");
	m_listView->addColumn("Device Name");
	m_listView->setResizeMode(QListView::LastColumn);
	
	connect(m_listView, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotSelectionChanged(QListViewItem*)));
	
	connect(m_listView, SIGNAL(doubleClicked(QListViewItem*)),
		this, SLOT(slotDoubleClicked(QListViewItem*)));
	
	connect(m_listView, SIGNAL(rightButtonClicked(QListViewItem*,
						const QPoint&, int)),
		this, SLOT(slotRightButtonClicked(QListViewItem*,
						const QPoint&, int)));

	// Create the text view
	m_textView = new QTextView(m_splitter);
	m_splitter->setResizeMode(m_textView, QSplitter::Auto);

	slotSelectionChanged(NULL);
}

//
// Signal handler for deviceAdded() signals from Doc
//
void DeviceManagerView::slotDeviceAdded(t_device_id id)
{
	Device* dev = NULL;
	QListViewItem* item = NULL;

	dev = _app->doc()->device(id);
	if (dev != NULL)
	{
		// Create a new list view item
		item = new QListViewItem(m_listView);

		// Fill device information to the item
		updateDeviceItem(item, dev);
	}
}

//
// Signal handler for deviceRemoved() signals from Doc
//
void DeviceManagerView::slotDeviceRemoved(t_device_id id)
{
  QListViewItemIterator it(m_listView);
  QListViewItem* item = NULL;
  QListViewItem* nextItem = NULL;
  Device* device = NULL;

  while ((item = it.current()) != NULL)
    {
      if (item->text(KColumnID).toInt() == id)
	{
	  if (item->isSelected())
	    {
	      // Try to select the closest neighbour
	      if (item->itemAbove())
		nextItem = item->itemAbove();
	      else
		nextItem = item->itemBelow();

	      // Select the neighbour
	      m_listView->setSelected(nextItem, true);
	      slotSelectionChanged(nextItem);
	    }

	  delete item;
	}

      ++it;
    }
}

//
// Update the list view completely
//
void DeviceManagerView::updateView()
{
	t_device_id currentId = KNoID;
	t_device_id id = KNoID;
	QListViewItem* item = NULL;
	Device* dev = NULL;

	// Store the currently selected device's ID
	if (m_listView->currentItem() != NULL)
	{
		currentId = m_listView->currentItem()->text(KColumnID).toInt();
	}

	// Clear the view
	m_listView->clear();

	// Add all devices
	for (id = 0; id < KDeviceArraySize; id++)
	{
		dev = _app->doc()->device(id);
		if (dev == NULL)
		{
			continue;
		}
		else
		{
			item = new QListViewItem(m_listView);
			
			// Update device information to the item
			updateDeviceItem(item, dev);

			// Select this if it was selected before update
			if (currentId == id)
			{
				m_listView->setSelected(item, true);
			}
		}
	}

	slotSelectionChanged(m_listView->currentItem());
}

//
// Add a device
//
void DeviceManagerView::slotAdd()
{
	NewDevice* ndlg = NULL;
	int i = 0;
	
	ndlg = new NewDevice(_app);
	ndlg->initView();
	
	if (ndlg->exec() == QDialog::Accepted)
	{
		int address = ndlg->address();
		int universe = ndlg->universe();
		int gap = ndlg->addressGap();
		QString name = ndlg->name();
		QString manufacturer = ndlg->manufacturer();
		QString model = ndlg->model();
		
		if (name.stripWhiteSpace() == QString::null)
		{
			// Empty name was given, use the model name instead
			name = ndlg->model();
		}

		// Search the actual device class instance to be associated
		// with the new device
		DeviceClass* dc = _app->searchDeviceClass(manufacturer, model);
		assert(dc);

		// Add the first device without gap
		_app->doc()->newDevice(dc, name, address, universe);

		// Add the rest (if any) with address gap
		for (i = 1; i < ndlg->multipleNumber(); i++)
		{
			_app->doc()->newDevice(dc, name,
				address + (i * dc->channels()->count()) + gap,
				universe);
		}
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
  Device* d = _app->doc()->newDevice(dc, new_name, 0, 0);
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
	  copyFunction(f, d);
	}
    }

  QString newid;
  newid.setNum(d->id());
  m_listView->setCurrentItem(m_listView->findItem(newid, KColumnID));
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
}

void DeviceManagerView::slotDoubleClicked(QListViewItem* item)
{
  if (item)
    {
      slotProperties();
    }
}

//
// View Properties
//
void DeviceManagerView::slotProperties()
{
	QListViewItem* item = NULL;
	Device* dev = NULL;

	item = m_listView->currentItem();
	if (item != NULL)
	{
		dev = _app->doc()->device(item->text(KColumnID).toInt());
		assert(dev);
		
		// View device properties dialog
		dev->viewProperties();

		// Then update device properties to view
		updateDeviceItem(item, dev);
		
		// Cause an update to the device info view
		slotSelectionChanged(item);
	}
}

//
// Autocreate functions
//
void DeviceManagerView::slotAutoFunction()
{
  Scene* sc = NULL;
  Device* device = NULL;
  LogicalChannel* channel = NULL;
  Capability* capability = NULL;

  QListViewItem* item = NULL;

  t_device_id did = KNoID;
  unsigned int i = 0;

  item = m_listView->currentItem();
  if (item == NULL)
    return;

  did = item->text(KColumnID).toInt();

  device = _app->doc()->device(did);
  assert(device);
  
  QPtrListIterator<LogicalChannel> ch_it(*device->deviceClass()->channels());

  // Loop over all channels
  while ((channel = ch_it.current()) != NULL)
    {
      QPtrListIterator<Capability> cap_it(*channel->capabilities());

      // Loop over all capabilities
      while ((capability = cap_it.current()) != NULL)
	{
	  sc = static_cast<Scene*> 
	    (_app->doc()->newFunction(Function::Scene, did));
	  
	  sc->setName(channel->name() + " - " + capability->name());

	  // Set the unused channels to NoSet and zero.
	  for (i = 0; i < device->deviceClass()->channels()->count(); i++)
	    {
	      sc->set(i, 0, Scene::NoSet);
	    }

	  // Set only the capability
	  sc->set(channel->channel(),
		  (t_value) ((capability->lo() + capability->hi()) / 2),
		  Scene::Set);

	  ++cap_it;
	}

      ++ch_it;
    }

  device->slotConsoleClosed();
  device->viewConsole();
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
  QString info;

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
      m_consoleButton->setEnabled(false);
      m_cloneButton->setEnabled(false);

      info = QString("<HTML><BODY>");
      info += QString("<H1>No Devices</H1>");
      info += QString("Click \"Add\" on the toolbar to add a new device.");
      info += QString("</BODY></HTML>");

      m_textView->setText(info);
    }
  else
    {
      // Set the text view's contents
      Device* dev = _app->doc()->device(item->text(KColumnID).toInt());
      ASSERT(dev);

      dev->infoText(info);
      m_textView->setText(info);

      // Enable console always
      m_consoleButton->setEnabled(true);

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

  menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/wizard.png")),
		   "Add...", KMenuItemAdd);
  menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
                   "Clone...", KMenuItemClone);
  menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
		   "Remove", KMenuItemRemove);
  menu->insertSeparator();
  menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
		   "Properties...", KMenuItemProperties);
  menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/console.png")),
		   "View Console...", KMenuItemConsole);
  menu->insertSeparator();
  menu->insertItem( QPixmap(QString(PIXMAPS) + QString("/function.png")),
                   "Autocreate Functions", KMenuItemAutoFunction);

  if (_app->mode() == App::Operate)
    {
      // Operate mode, remove and edit impossible
      menu->setItemEnabled(KMenuItemAdd, false);
      menu->setItemEnabled(KMenuItemRemove, false);
      menu->setItemEnabled(KMenuItemProperties, false);
      menu->setItemEnabled(KMenuItemClone, false);
      menu->setItemEnabled(KMenuItemAutoFunction, false);
    }

  // No item selected, unable to do other things either
  if (!item)
    {
      menu->setItemEnabled(KMenuItemRemove, false);
      menu->setItemEnabled(KMenuItemConsole, false);
      menu->setItemEnabled(KMenuItemProperties, false);
      menu->setItemEnabled(KMenuItemClone, false);
      menu->setItemEnabled(KMenuItemAutoFunction, false);
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

    case KMenuItemConsole:
      slotConsole();
      break;

    case KMenuItemAutoFunction:
      slotAutoFunction();
      break;
    default:
      break;
    }
}


void DeviceManagerView::copyFunction(Function* function, Device* device)
{
  switch(function->type())
    {
    case Function::Scene:
      {
	Scene* scene = static_cast<Scene*>
	  (_app->doc()->newFunction(Function::Scene, device->id()));

	scene->copyFrom(static_cast<Scene*> (function), device->id());
      }
      break;

    case Function::Chaser:
      {
	Chaser* chaser = static_cast<Chaser*>
	  (_app->doc()->newFunction(Function::Chaser, KNoID));

	chaser->copyFrom(static_cast<Chaser*> (function));
	assert(device == NULL);
      }
      break;

    case Function::Collection:
      {
	FunctionCollection* fc = static_cast<FunctionCollection*>
	  (_app->doc()->newFunction(Function::Collection, KNoID));

	fc->copyFrom(static_cast<FunctionCollection*> (function));
	assert(device == NULL);
      }
      break;

    case Function::Sequence:
      {
	Sequence* sequence = static_cast<Sequence*>
	  (_app->doc()->newFunction(Function::Sequence, KNoID));

	sequence->copyFrom(static_cast<Sequence*> (function), device->id());
      }
      break;

    case Function::EFX:
      {
	EFX* efx = static_cast<EFX*>
	  (_app->doc()->newFunction(Function::EFX, KNoID));

	efx->copyFrom(static_cast<EFX*> (function), device->id());
      }
      break;

    default:
      break;
    }
}

//
// Fill item's data fields with device information
//
void DeviceManagerView::updateDeviceItem(QListViewItem* item, Device* dev)
{
	QString s;
	
	assert(item);
	assert(dev);
	
	// Universe column
	s.sprintf("%d", dev->universe() + 1);
	item->setText(KColumnUniverse, s);

	// Address column
	s.sprintf("%.3d - %.3d", dev->address() + 1,
		dev->address() + 
		dev->deviceClass()->channels()->count());
	item->setText(KColumnAddress, s);
	
	// Name column
	item->setText(KColumnName, dev->name());

	// ID column
	s.setNum(dev->id());
	item->setText(KColumnID, s);
}
