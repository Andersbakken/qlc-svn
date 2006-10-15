/*
  Q Light Controller
  functionmanager.cpp

  Copyright (C) Heikki Junnila

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

#include <qdialog.h>
#include <qheader.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qdockarea.h>
#include <qtooltip.h>
#include <qinputdialog.h>
#include <qptrlist.h>
#include <assert.h>

#include "functionmanager.h"
#include "function.h"
#include "device.h"
#include "deviceclass.h"
#include "app.h"
#include "doc.h"
#include "common/settings.h"
#include "scene.h"
#include "advancedsceneeditor.h"
#include "chaser.h"
#include "chasereditor.h"
#include "functioncollection.h"
#include "functioncollectioneditor.h"
#include "sequence.h"
#include "sequenceeditor.h"
#include "configkeys.h"
#include "efx.h"
#include "efxeditor.h"

const int KMenuEdit           ( 0 );
const int KMenuDelete         ( 1 );
const int KMenuCut            ( 2 );
const int KMenuCopy           ( 3 );
const int KMenuPaste          ( 4 );
const int KMenuSelectAll      ( 5 );

const int KColumnName         ( 0 );
const int KColumnBus          ( 1 );
const int KColumnID           ( 2 );

// FunctionManager Dialog keys
const QString KEY_FunctionManager_OPEN   ( "FunctionManagerOpen" );
const QString KEY_WARN_COPY_FUNCTION  ( "WarnCopyFunction" );

extern App* _app;

/**
 * Constructor. If selectionMode = true, the dialog is in
 * selection mode. Otherwise the dialog is in normal edit mode.
 */
FunctionManager::FunctionManager(QWidget* parent, WFlags flags)
  : QWidget(parent, "Function Manager", flags),
    m_addMenu  ( NULL ),
    m_editMenu ( NULL ),
    m_busMenu  ( NULL ),
    
    m_fixtureTree  ( NULL ),
    m_functionTree ( NULL ),
    
    m_treeViewSplitter ( NULL ),
    m_clipboardAction ( ClipboardNone ),
    
    m_inactiveID ( KNoID ),
    m_buttonLayout ( NULL ),
    m_result ( QDialog::Rejected ),
    m_ok ( NULL ),
    m_cancel ( NULL ),

    m_blockAddFunctionSignal ( false ),
    m_blockRemoveFunctionSignal ( false )
{
	if (flags == FunctionManager::SelectionMode)
	{
		m_selectionMode = true;
	}
	else
	{
		m_selectionMode = false;
	}
}

/**
 * Destructor
 */
FunctionManager::~FunctionManager()
{
	if (m_addMenu)
	{
		delete m_addMenu;
	}

	if (m_busMenu)
	{
		delete m_busMenu;
	}

	if (m_editMenu)
	{
		delete m_editMenu;
	}
}


/**
 * This function is called when the user presses the close
 * button in the window handle
 */
void FunctionManager::closeEvent(QCloseEvent* e)
{
	e->accept();
	emit closed();
}


/**
 * Initialize the dialog, second stage construction
 */
void FunctionManager::init()
{
	if (m_selectionMode == false)
	{
		// Set the window title
		setCaption("Function Manager");
	}
	else
	{
		setCaption("Select Function(s)");
	}

	// Icon
	setIcon(QPixmap(QString(PIXMAPS) + QString("/function.png")));

	// Window size
	resize(640, 480);

	// No need to save the pointer because the parent widget takes
	// control of the layout item
	new QVBoxLayout(this);
	layout()->setAutoAdd(false);

	if (m_selectionMode == false)
	{
		// Create menu bar
		initMenu();

		// Create tool bar
		initToolbar();
	}
	else
	{
		layout()->setSpacing(10);
		layout()->setMargin(5);
	}

	// Create the splitter which contains the fixture & function trees
	m_treeViewSplitter = new QSplitter(this);
	layout()->add(m_treeViewSplitter);
	m_treeViewSplitter->setSizePolicy(QSizePolicy::Expanding,
					  QSizePolicy::Expanding);

	// Create fixture tree
	initFixtureTree();

	// Create function tree
	initFunctionTree();

	if (m_selectionMode == true)
	{
		QFrame* frame = NULL;
		frame = new QFrame(this);
		layout()->add(frame);
		frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
		frame->setSizePolicy(QSizePolicy::Expanding,
				    QSizePolicy::Minimum);

		m_buttonLayout = new QHBoxLayout(layout());
		m_buttonLayout->setSpacing(10);
		m_buttonLayout->addItem(new QSpacerItem(100, 30));

		m_ok = new QPushButton(this);
		m_buttonLayout->add(m_ok);
		m_ok->setText("&OK");
		m_ok->setSizePolicy(QSizePolicy::Minimum,
				    QSizePolicy::Preferred);
		connect(m_ok, SIGNAL(clicked()),
			this, SLOT(slotOKClicked()));

		m_cancel = new QPushButton(this);
		m_buttonLayout->add(m_cancel);
		m_cancel->setText("&Cancel");
		m_cancel->setSizePolicy(QSizePolicy::Minimum,
					QSizePolicy::Preferred);
		connect(m_cancel, SIGNAL(clicked()),
			this, SLOT(slotCancelClicked()));
	}

	// Clear selections
	m_selectedFunctions.clear();
	m_selectedFunctions.setAutoDelete(false);

	// Clear clipboard contents
	m_clipboard.clear();

	// Select the first fixture
	m_fixtureTree->setSelected(m_fixtureTree->firstChild(), true);
}


/**
 * Copy the selected function IDs to the list
 */
int FunctionManager::selection(FunctionIDList& list)
{
	FunctionIDList::iterator it;
	list.clear();

	for (it = m_selection.begin(); it != m_selection.end(); ++it)
	{
		list.append(*it);
	}

	return m_result;
}

//
// Signal handler for deviceAdded() signals from Doc
//
void FunctionManager::slotDeviceAdded(t_device_id id)
{
  Device* device = NULL;
  QListViewItem* item = NULL;
  QString s;

  device = _app->doc()->device(id);
  if (device != NULL)
    {
      item = new QListViewItem(m_fixtureTree);
      
      // Pixmap column
      item->setPixmap(KColumnName, QPixmap(QString(PIXMAPS) +
					   QString("/fixture.png")));
      // Name column
      item->setText(KColumnName, device->name());
      
      // ID column
      s.setNum(id);
      item->setText(KColumnID, s);
    }
}

//
// Signal handler for deviceRemoved() signals from Doc
//
void FunctionManager::slotDeviceRemoved(t_device_id id)
{
  QListViewItem* item = NULL;
  QListViewItem* nextItem = NULL;

  item = getItem(id, m_fixtureTree);
  if (item)
    {
      if (item->isSelected())
	{
	  // Try to select the closest neighbour
	  if (item->itemAbove())
	    nextItem = item->itemAbove();
	  else
	    nextItem = item->itemBelow();
	  
	  // Select the neighbour
	  m_fixtureTree->setSelected(nextItem, true);
	}

      delete item;
    }
}

//
// Signal handler for deviceChanged() signals from Doc
//
void FunctionManager::slotDeviceChanged(t_device_id id)
{
  QListViewItem* item = NULL;
  Device* device = NULL;

  item = getItem(id, m_fixtureTree);
  if (item)
    {
      device = _app->doc()->device(id);
      ASSERT(device);

      item->setText(KColumnName, device->name());
    }
}

//
// Signal handler for functionAdded() signals from Doc
//
void FunctionManager::slotFunctionAdded(t_function_id id)
{
  QListViewItem* deviceItem = NULL;
  QListViewItem* item = NULL;
  Function* function = NULL;

  // The function manager has its own routines for functions that are
  // created with it.
  if (m_blockAddFunctionSignal)
    return;

  deviceItem = m_fixtureTree->currentItem();

  if (deviceItem == NULL)
    return; // Function tree should be empty, nothing to do

  function = _app->doc()->function(id);
  if (function)
    {
      // Check if the selected device is the one that the newly added
      // function belongs to.
      if (deviceItem->text(KColumnID).toInt() == function->device())
	{
	  // Create a new item for the function
	  item = new QListViewItem(m_functionTree);
	  updateFunctionItem(item, function);
	}
    }
}

//
// Signal handler for functionRemoved() signals from Doc
//
void FunctionManager::slotFunctionRemoved(t_function_id id)
{
  QListViewItem* item = NULL;
  QListViewItem* nextItem = NULL;

  // The function manager has its own routines for functions that are
  // removed with it.
  if (m_blockRemoveFunctionSignal)
    return;

  item = getItem(id, m_functionTree);
  if (item)
    {
      if (item->isSelected())
	{
	  // Try to select the closest neighbour
	  if (item->itemAbove())
	    nextItem = item->itemAbove();
	  else
	    nextItem = item->itemBelow();
	  
	  // Select the neighbour
	  m_functionTree->setSelected(nextItem, true);
	}
      
      delete item;
    }
}

//
// Signal handler for functionChanged() signals from Doc
//
void FunctionManager::slotFunctionChanged(t_function_id id)
{
  QListViewItem* item = NULL;
  Function* function = NULL;

  item = getItem(id, m_functionTree);
  if (item)
    {
      function = _app->doc()->function(id);
      ASSERT(function);

      updateFunctionItem(item, function);
    }
}

//
// Get an item from the given listview by the given id
//
QListViewItem* FunctionManager::getItem(t_function_id id, QListView* listView)
{
  ASSERT(listView);

  QListViewItemIterator it(listView);
  QListViewItem* item = NULL;

  while ((item = it.current()) != NULL)
    {
      if (item->text(KColumnID).toInt() == id)
	{
	  break;
	}
      ++it;
    }

  return item;
}

/**
 * Initialize the menu bar
 */
void FunctionManager::initMenu()
{
	QMenuBar* menubar = new QMenuBar(this);
	layout()->setMenuBar(menubar);

	//
	// Edit menu
	//
	m_editMenu = new QPopupMenu();
	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/edit.png")),
				"Edit...",
				this, SLOT(slotEdit()),
				CTRL+Key_E, KMenuEdit);

	m_editMenu->insertSeparator();

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/editcut.png")),
				"Cut",
				this, SLOT(slotCut()),
				CTRL+Key_X, KMenuCut);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/editcopy.png")),
				"Copy",
				this, SLOT(slotCopy()),
				CTRL+Key_C, KMenuCopy);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/editpaste.png")),
				"Paste",
				this, SLOT(slotPaste()),
				CTRL+Key_V, KMenuPaste);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/editdelete.png")),
				"Delete",
				this, SLOT(slotDelete()),
				Key_Delete, KMenuDelete);

	m_editMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/selectall.png")),
				"Select All",
				this, SLOT(slotSelectAll()),
				CTRL+Key_A, KMenuSelectAll);

	//
	// Bus menu
	//
	slotUpdateBusMenu();
	m_editMenu->insertItem("Assign Bus", m_busMenu);

	//
	// Add menu
	//
	m_addMenu = new QPopupMenu();
	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/scene.png")),
				"Scene...",
				Function::Scene);

	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/chaser.png")),
				"Chaser...",
				Function::Chaser);

	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/collection.png")),
				"Collection...",
				Function::Collection);

	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/sequence.png")),
				"Sequence...",
				Function::Sequence);

	m_addMenu->insertItem(QPixmap(QString(PIXMAPS) +
				QString("/efx.png")),
				"EFX...",
				Function::EFX);

	menubar->insertItem("Add", m_addMenu);
	menubar->insertItem("Edit", m_editMenu);

	connect(m_addMenu, SIGNAL(activated(int)),
		this, SLOT(slotAddMenuCallback(int)));
}


/**
 * Create a toolbar
 */
void FunctionManager::initToolbar()
{
	// Create a dock area for the toolbar
	m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
	m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	// Add the dock area to the top of the vertical layout
	layout()->add(m_dockArea);

	// Add a toolbar to the dock area
	m_toolbar = new QToolBar("Function Manager", _app, m_dockArea);

	// Scene
	m_addSceneButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/scene.png"))),
					"Scene",
					0,
					this,
					SLOT(slotAddScene()),
					m_toolbar);
	QToolTip::add(m_addSceneButton, "Add new scene");

	// Chaser
	m_addChaserButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/chaser.png"))),
					"Chaser",
					0,
					this,
					SLOT(slotAddChaser()),
					m_toolbar);
	QToolTip::add(m_addChaserButton, "Add new chaser");

	// Collection
	m_addCollectionButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/collection.png"))),
					"Collection",
					0,
					this,
					SLOT(slotAddCollection()),
					m_toolbar);
	QToolTip::add(m_addCollectionButton, "Add new collection");

	// Sequence
	m_addSequenceButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/sequence.png"))),
					"Sequence",
					0,
					this,
					SLOT(slotAddSequence()),
					m_toolbar);
	QToolTip::add(m_addSequenceButton, "Add new sequence");

	// EFX
	m_addEFXButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/efx.png"))),
					"EFX",
					0,
					this,
					SLOT(slotAddEFX()),
					m_toolbar);
	QToolTip::add(m_addEFXButton, "Add new EFX");

	// Separator
	m_toolbar->addSeparator();

	// Cut
	m_cutButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/editcut.png"))),
					"Cut",
					0,
					this,
					SLOT(slotCut()),
					m_toolbar);
	QToolTip::add(m_cutButton, "Cut selected function(s)");

	// Copy
	m_copyButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/editcopy.png"))),
					"Copy",
					0,
					this,
					SLOT(slotCopy()),
					m_toolbar);
	QToolTip::add(m_copyButton, "Copy selected function(s)");

	// Paste
	m_pasteButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/editpaste.png"))),
					"Paste",
					0,
					this,
					SLOT(slotPaste()),
					m_toolbar);
	QToolTip::add(m_pasteButton, "Paste cut/copied function(s)");

	// Delete
	m_deleteButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/editdelete.png"))),
					"Delete",
					0,
					this,
					SLOT(slotDelete()),
					m_toolbar);
	QToolTip::add(m_deleteButton, "Delete selected function(s)");

	// Separator
	m_toolbar->addSeparator();

	// Edit
	m_editButton = new QToolButton(
					QIconSet(QPixmap(
						QString(PIXMAPS) +
						QString("/edit.png"))),
					"Edit",
					0,
					this,
					SLOT(slotEdit()),
					m_toolbar);
	QToolTip::add(m_editButton, "Edit selected function");
}


/**
 * Initialize the tree view that holds fixtures
 */
void FunctionManager::initFixtureTree()
{
	// Create the tree view as the first child of the splitter
	m_fixtureTree = new QListView(m_treeViewSplitter);

	m_fixtureTree->setMultiSelection(false);
	m_fixtureTree->setAllColumnsShowFocus(true);
	m_fixtureTree->setSorting(KColumnName, true);
	m_fixtureTree->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	m_fixtureTree->header()->setClickEnabled(true);
	m_fixtureTree->header()->setResizeEnabled(false);
	m_fixtureTree->header()->setMovingEnabled(false);
	m_fixtureTree->header()->setSortIndicator(KColumnName, Ascending);

	// Add the one and only column
	m_fixtureTree->addColumn("Fixture");
	m_fixtureTree->setResizeMode(QListView::AllColumns);

	// Catch header clicks
	connect(m_fixtureTree->header(), SIGNAL(clicked(int)),
		this, SLOT(slotFixtureHeaderClicked(int)));

	// Catch selection changes
	connect(m_fixtureTree, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotFixtureTreeSelectionChanged(QListViewItem*)));

	if (m_selectionMode == false)
	{
		// Catch right-mouse clicks
		connect(m_fixtureTree,
			SIGNAL(contextMenuRequested(QListViewItem*,
						const QPoint&,
						int)),
			this,
			SLOT(slotFixtureTreeContextMenuRequested(QListViewItem*,
								const QPoint&,
								int)));
	}

	updateFixtureTree();
}


/**
 * Initialize the tree view that holds functions
 */
void FunctionManager::initFunctionTree()
{
	// Create the tree view as the second child of the splitter
	m_functionTree = new QListView(m_treeViewSplitter);

	// Normal multi-selection behaviour
	m_functionTree->setSelectionMode(QListView::Extended);
	m_functionTree->setAllColumnsShowFocus(true);
	m_functionTree->setSorting(KColumnName, true);
	m_functionTree->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	m_functionTree->header()->setClickEnabled(true);
	m_functionTree->header()->setResizeEnabled(true);
	m_functionTree->header()->setMovingEnabled(false);
	m_functionTree->header()->setSortIndicator(KColumnName, Ascending);

	// Add two columns for function and bus
	m_functionTree->addColumn("Function");
	m_functionTree->addColumn("Bus");
	m_functionTree->setResizeMode(QListView::LastColumn);

	// Catch header clicks
	connect(m_functionTree->header(), SIGNAL(clicked(int)),
		this, SLOT(slotFunctionHeaderClicked(int)));

	// Catch selection changes
	connect(m_functionTree, SIGNAL(selectionChanged()),
		this, SLOT(slotFunctionTreeSelectionChanged()));

	// Catch mouse double clicks
	connect(m_functionTree,
		SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
		this,
		SLOT(slotFunctionTreeDoubleClicked(QListViewItem*, const QPoint&, int)));

	if (m_selectionMode == false)
	{
		// Catch right-mouse clicks
		connect(m_functionTree,
			SIGNAL(contextMenuRequested(QListViewItem*,
					    const QPoint&,
					    int)),
			this,
			SLOT(slotFunctionTreeContextMenuRequested(QListViewItem*,
							  const QPoint&,
							  int)));
	}
}


/**
 * Update fixtures to the fixture tree
 */
void FunctionManager::updateFixtureTree()
{
	QString devid;
	t_device_id id = KNoID;
	Device* device = NULL;

	m_fixtureTree->clear();

	//
	// Global root node
	//
	devid.setNum(KNoID);
	QListViewItem* item = new QListViewItem(m_fixtureTree);
	item->setText(KColumnName, QString("Global"));
	item->setText(KColumnID, devid);
	item->setPixmap(KColumnName, QPixmap(QString(PIXMAPS) +
				     QString("/global.png")));
	// item->setOpen(m_alwaysOpen->isChecked());

	//
	// Device root nodes
	//
	for (id = 0; id < KDeviceArraySize; id++)
	{
		device = _app->doc()->device(id);
		if (device == NULL)
		{
			continue;
		}

		devid.setNum(id);
		item = new QListViewItem(m_fixtureTree);
		item->setText(KColumnName, device->name());
		item->setText(KColumnID, devid);
		item->setPixmap(KColumnName, QPixmap(QString(PIXMAPS) +
					     QString("/fixture.png")));
	}

	m_selectedFunctions.clear();
}


/**
 * Update the contents of the given tree item using the given function
 */
void FunctionManager::updateFunctionItem(QListViewItem* item,
					 Function* function)
{
	if (item == NULL || function == NULL)
	{
		return;
	}
	else
	{
		item->setText(KColumnName, function->name());
		item->setPixmap(KColumnName, function->pixmap());

		item->setText(KColumnBus, function->busName());
		item->setText(KColumnID, QString::number(function->id()));

		if (function->id() == m_inactiveID)
		{
			item->setEnabled(false);
		}
		else
		{
			item->setEnabled(true);
		}
	}
}


/**
 * Delete selected functions
 */
void FunctionManager::deleteSelectedFunctions()
{
  QListViewItem* item = NULL;
  t_function_id fid = KNoID;
  
  // Delete functions and listview items
  while ( (item = m_selectedFunctions.take(0)) != NULL)
    {
      fid = item->text(KColumnID).toInt();

      m_blockRemoveFunctionSignal = true;
      _app->doc()->deleteFunction(fid);
      m_blockRemoveFunctionSignal = false;
      
      delete item;
    }
}

/**
 * Enable/disable menu items according to selections
 */
void FunctionManager::updateMenuItems()
{
	QListViewItem* deviceitem = m_fixtureTree->currentItem();

	t_device_id did = KNoID;
	t_function_id fid = KNoID;

	if (m_selectionMode == true)
	{
		return;
	}

	if (deviceitem == NULL)
	{
		// There are no devices (at least none is selected)
		// so nothing can be done in function manager
		m_addMenu->setEnabled(false);
		m_editMenu->setEnabled(false);
		m_busMenu->setEnabled(false);

		m_toolbar->setEnabled(false);
	}
	else
	{
		m_addMenu->setEnabled(true);
		m_editMenu->setEnabled(true);
		m_busMenu->setEnabled(true);

		m_toolbar->setEnabled(true);

		// Get the selected device
		did = deviceitem->text(KColumnID).toInt();

		if (did == KNoID)
		{
			// Global has been selected

			// Disable non-global functions, enable globals
			m_addMenu->setItemEnabled(Function::Scene, false);
			m_addMenu->setItemEnabled(Function::Chaser, true);
			m_addMenu->setItemEnabled(Function::Collection, true);
			m_addMenu->setItemEnabled(Function::Sequence, false);
			m_addMenu->setItemEnabled(Function::EFX, false);

			m_addSceneButton->setEnabled(false);
			m_addChaserButton->setEnabled(true);
			m_addCollectionButton->setEnabled(true);
			m_addSequenceButton->setEnabled(false);
			m_addEFXButton->setEnabled(false);
		}
		else
		{
			// A regular device has been selected

			// Disable global functions, enable others
			m_addMenu->setItemEnabled(Function::Scene, true);
			m_addMenu->setItemEnabled(Function::Chaser, false);
			m_addMenu->setItemEnabled(Function::Collection, false);
			m_addMenu->setItemEnabled(Function::Sequence, true);
			m_addMenu->setItemEnabled(Function::EFX, true);

			m_addSceneButton->setEnabled(true);
			m_addChaserButton->setEnabled(false);
			m_addCollectionButton->setEnabled(false);
			m_addSequenceButton->setEnabled(true);
			m_addEFXButton->setEnabled(true);
		}

		if (m_selectedFunctions.count() > 0)
		{
			// Something has been selected
			m_editMenu->setItemEnabled(KMenuCut, true);
			m_editMenu->setItemEnabled(KMenuCopy, true);
			m_editMenu->setItemEnabled(KMenuDelete, true);
			m_editMenu->setItemEnabled(KMenuSelectAll, true);
			m_editMenu->setItemEnabled(KMenuEdit, true);

			m_cutButton->setEnabled(true);
			m_copyButton->setEnabled(true);
			m_deleteButton->setEnabled(true);
			m_editButton->setEnabled(true);
		}
		else
		{
			// Nothing is selected
			m_editMenu->setItemEnabled(KMenuCut, false);
			m_editMenu->setItemEnabled(KMenuCopy, false);
			m_editMenu->setItemEnabled(KMenuDelete, false);
			m_editMenu->setItemEnabled(KMenuSelectAll, false);
			m_editMenu->setItemEnabled(KMenuEdit, false);

			m_cutButton->setEnabled(false);
			m_copyButton->setEnabled(false);
			m_deleteButton->setEnabled(false);
			m_editButton->setEnabled(false);

			m_busMenu->setEnabled(false);
		}

		if (m_clipboard.count() > 0)
		{
			// Clipboard contains something to paste
			m_editMenu->setItemEnabled(KMenuPaste, true);

			m_pasteButton->setEnabled(true);
		}
		else
		{
			// Clipboard is empty
			m_editMenu->setItemEnabled(KMenuPaste, false);
			// Ensure that there is no clipboard action
			m_clipboardAction = ClipboardNone;

			m_pasteButton->setEnabled(false);
		}
	}
}


/****************************************************************************
 * SLOTS
 ****************************************************************************/

/**
 * Update the sort indicator in fixture tree
 */
void FunctionManager::slotFixtureHeaderClicked(int section)
{
	if (m_fixtureTree->sortOrder() == Ascending)
	{
		m_fixtureTree->header()->setSortIndicator(section,
							  Ascending);
	}
	else
	{
		m_fixtureTree->header()->setSortIndicator(section,
							  Descending);
	}
}


/**
 * Update the sort indicator in fixture tree
 */
void FunctionManager::slotFunctionHeaderClicked(int section)
{
	if (m_fixtureTree->sortOrder() == Ascending)
	{
		m_functionTree->header()->setSortIndicator(section,
							   Ascending);
	}
	else
	{
		m_functionTree->header()->setSortIndicator(section,
							   Descending);
	}
}


/**
 * A fixture has been selected from the fixture tree
 */
void FunctionManager::slotFixtureTreeSelectionChanged(QListViewItem* item)
{
	t_device_id did = KNoID;
	t_function_id fid = KNoID;
	Function* function = NULL;

	if (item == NULL)
	{
		return;
	}

	// Get the selected device's ID
	did = item->text(KColumnID).toInt();

	// Clear the current list of selected functions
	m_selectedFunctions.clear();

	// Clear the contents of the function tree
	m_functionTree->clear();

	// Get all functions belonging to the selected fixture
	for (fid = 0; fid < KFunctionArraySize; fid++)
	{
	        function = _app->doc()->function(fid);
		if (function && function->device() == did)
		{
			item = new QListViewItem(m_functionTree);
			updateFunctionItem(item, function);
		}
	}

	updateMenuItems();
}


/**
 * Keep record of currently selected functions
 */
void FunctionManager::slotFunctionTreeSelectionChanged()
{
	QListViewItem* item = NULL;

	for (item = m_functionTree->firstChild();
	     item != NULL;
	     item = item->nextSibling())
	{
		if (item->isSelected() == false)
		{
			if (m_selectedFunctions.contains(item) > 0)
			{
				m_selectedFunctions.remove(item);
			}
		}
		else
		{
			if (m_selectedFunctions.contains(item) <= 0)
			{
				m_selectedFunctions.append(item);
			}
		}
	}

	updateMenuItems();
}


/**
 * User has clicked the right mouse button in the fixture tree
 */
void FunctionManager::slotFixtureTreeContextMenuRequested(QListViewItem* item,
					      const QPoint& pos, int col)
{
	// Append the "Add" menu to the end of "Edit" menu
	int separatorIndex = m_editMenu->insertSeparator();
	int addMenuIndex = m_editMenu->insertItem("Add", m_addMenu);

	updateMenuItems();

	// No need to have cut/copy/delete/edit/select in fixture menu
	m_editMenu->setItemEnabled(KMenuCut, false);
	m_editMenu->setItemEnabled(KMenuCopy, false);
	m_editMenu->setItemEnabled(KMenuDelete, false);
	m_editMenu->setItemEnabled(KMenuEdit, false);
	m_editMenu->setItemEnabled(KMenuSelectAll, false);

	m_editMenu->exec(pos);

	// Enable the menu items again
	updateMenuItems();

	// Remove the "Add" menu from the end of "Edit" menu
	m_editMenu->removeItem(separatorIndex);
	m_editMenu->removeItem(addMenuIndex);
}


/**
 * User has clicked the right mouse button in the function tree
 */
void FunctionManager::slotFunctionTreeContextMenuRequested(QListViewItem* item,
					      const QPoint& pos, int col)
{
	// Append the "Add" menu to the end of "Edit" menu
	int separatorIndex = m_editMenu->insertSeparator();
	int addMenuIndex = m_editMenu->insertItem("Add", m_addMenu);

	updateMenuItems();

	m_editMenu->exec(pos);

	// Remove the "Add" menu from the end of "Edit" menu
	m_editMenu->removeItem(separatorIndex);
	m_editMenu->removeItem(addMenuIndex);
}


/**
 * Get all available buses to the bus menu
 */
void FunctionManager::slotUpdateBusMenu()
{
	if (m_selectionMode == true)
	{
		return;
	}

	if (m_busMenu == NULL)
	{
		m_busMenu = new QPopupMenu();
	}

	m_busMenu->clear();
	m_busMenu->setCheckable(false);
	for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
	{
		QString bus;
		bus.sprintf("%.2d: ", i + 1);
		bus += Bus::name(i);
		m_busMenu->insertItem(bus, i);
	}

	// Disconnect first to prevent double callbacks
	disconnect(m_busMenu, SIGNAL(activated(int)),
		this, SLOT(slotBusActivated(int)));

	connect(m_busMenu, SIGNAL(activated(int)),
		this, SLOT(slotBusActivated(int)));

	// Disconnect first to prevent double callbacks
	disconnect(
		Bus::emitter(),
		SIGNAL(nameChanged(t_bus_id, const QString&)),
		this,
		SLOT(slotUpdateBusMenu())
	);

	connect(
		Bus::emitter(),
		SIGNAL(nameChanged(t_bus_id, const QString&)),
		this,
		SLOT(slotUpdateBusMenu())
	);
}


/**
 * Set the selected bus to all selected functions
 */
void FunctionManager::slotBusActivated(int busID)
{
	Function* function = NULL;
	QPtrListIterator<QListViewItem> it(m_selectedFunctions);

	while ( it.current() != NULL)
	{
		function = _app->doc()->function(
			it.current()->text(KColumnID).toInt());
		assert(function);
		function->setBus(busID);
		updateFunctionItem(it.current(), function);
		++it;
	}
}


/**
 * Add a new function
 */
void FunctionManager::slotAddMenuCallback(int type)
{
	QListViewItem* deviceItem = m_fixtureTree->currentItem();
	QListViewItem* newItem = NULL;
	t_device_id did = KNoID;
	Function* f = NULL;

	if (deviceItem == NULL)
	{
		return;
	}

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = true;

	did = deviceItem->text(KColumnID).toInt();

	//
	// Create the function
	//
	switch(type)
	{
		case Function::Scene:
		{
		  f = _app->doc()->newFunction(Function::Scene, did);
		  assert(f);
		  f->setName("New Scene");
		}
		break;

		case Function::Chaser:
		{
		  f = _app->doc()->newFunction(Function::Chaser, KNoID);
		  assert(f);
		  f->setName("New Chaser");
		}
		break;

		case Function::Collection:
		{
		  f = _app->doc()->newFunction(Function::Collection, KNoID);
		  assert(f);
		  f->setName("New Collection");
		}
		break;

		case Function::Sequence:
		{
		  f = _app->doc()->newFunction(Function::Sequence, did);
		  assert(f);
		  f->setName("New Sequence");
		}
		break;

		case Function::EFX:
		{
		  f = _app->doc()->newFunction(Function::EFX, did);
		  assert(f);
		  f->setName("New EFX");
		}
		break;

		default:
		break;
	}

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = false;

	// Create a new item for the function
	newItem = new QListViewItem(m_functionTree);

	// Update the item's contents based on the function itself
	updateFunctionItem(newItem, f);

	// Clear current selection so that slotEdit() behaves correctly
	m_functionTree->clearSelection();

	// Select only the new item
	m_functionTree->setSelected(newItem, true);

	if (slotEdit() == QDialog::Rejected)
	{
		// Edit dialog was rejected -> delete function
		deleteSelectedFunctions();
	}
	else
	{
		// Sort the list so that the new item takes its correct place
		m_functionTree->sort();

		// Make sure that the new item is visible
		m_functionTree->ensureItemVisible(newItem);
	}
}

/**
 * Callback for function tree double clicks
 */
void FunctionManager::slotFunctionTreeDoubleClicked(QListViewItem* item,
						    const QPoint& pos, int col)
{
  if (m_selectionMode == true)
    {
      slotOKClicked();
    }
  else
    {
      slotEdit();
    }
}

/**
 * Callback for Scene tool button
 */
void FunctionManager::slotAddScene()
{
	slotAddMenuCallback(Function::Scene);
}


/**
 * Callback for Chaser tool button
 */
void FunctionManager::slotAddChaser()
{
	slotAddMenuCallback(Function::Chaser);
}


/**
 * Callback for Collection tool button
 */
void FunctionManager::slotAddCollection()
{
	slotAddMenuCallback(Function::Collection);
}


/**
 * Callback for Sequence tool button
 */
void FunctionManager::slotAddSequence()
{
	slotAddMenuCallback(Function::Sequence);
}


/**
 * Callback for EFX tool button
 */
void FunctionManager::slotAddEFX()
{
	slotAddMenuCallback(Function::EFX);
}


//
// Begin to move a function
//
void FunctionManager::slotCut()
{
	QListViewItem* item = NULL;
	QPtrListIterator<QListViewItem> it(m_selectedFunctions);

	// Clear existing stuff from clipboard
	if (m_clipboard.count() > 0)
	{
		m_clipboard.clear();
	}

	while ( (item = it.current()) != NULL)
	{
		// Add selected function ID's to clipboard
		m_clipboard.append(item->text(KColumnID).toInt());

		// Set the action to Cut so that we know what to
		// do with paste
		m_clipboardAction = ClipboardCut;

		// Set the item disabled to indicate that they are
		// about to get cut away
		item->setEnabled(false);

		// Next tree item, please
		++it;
	}
}

/**
 * Copy a function. Just gets the selected functions' ID's to clipboard.
 */
void FunctionManager::slotCopy()
{
	QListViewItem* item = NULL;
	QPtrListIterator<QListViewItem> it(m_selectedFunctions);

	// Clear existing stuff from the clipboard
	if (m_clipboard.count() > 0)
	{
		m_clipboard.clear();
	}

	while ( (item = it.current()) != NULL)
	{
		// Add selected function ID's to clipboard
		m_clipboard.append(item->text(KColumnID).toInt());

		// Set the action to Cut so that we know what to
		// do with paste
		m_clipboardAction = ClipboardCopy;

		// In case the user does a cut & then copy, enable the
		// selected items because cut disables them.
		item->setEnabled(true);

		// Next tree item, please
		++it;
	}
}


/**
 * Paste (move or copy) a function
 */
void FunctionManager::slotPaste()
{
	t_device_id did = KNoID;
	QListViewItem* deviceitem = NULL;
	QListViewItem* functionitem = NULL;
	FunctionIDList::iterator it;
	Function* newFunction = NULL;

	deviceitem = m_fixtureTree->currentItem();
	if (deviceitem == NULL)
	{
		return;
	}

	if (m_clipboardAction == ClipboardCut)
	{
		// Get the currently selected device item
		did = deviceitem->text(KColumnID).toInt();

		for (it = m_clipboard.begin(); it != m_clipboard.end(); ++it)
		{
			// Copy the function
			newFunction = copyFunction(*it, did);
			if (newFunction)
			{
				// Add the function to the current function tree
				functionitem = new QListViewItem(m_functionTree);
				updateFunctionItem(functionitem, newFunction);

				_app->doc()->deleteFunction(*it);
			}
			else
			{
				// Stop pasting so we don't get the user
				// too annoyed with a message box for each
				// function. They all will fail, because
				// the user can't select functions from more
				// than one device (or global) at a time
				break;
			}
		}

		// Since the functions that were cut previously, are now
		// deleted, their function id's in the clipboard are also invalid.
		m_clipboard.clear();
	}
	else if (m_clipboardAction == ClipboardCopy)
	{
		// Get the currently selected device item
		did = deviceitem->text(KColumnID).toInt();

		for (it = m_clipboard.begin(); it != m_clipboard.end(); ++it)
		{
			// Copy the function
			newFunction = copyFunction(*it, did);
			if (newFunction)
			{
				// Add the function to the current function tree
				functionitem = new QListViewItem(m_functionTree);
				updateFunctionItem(functionitem, newFunction);
			}
			else
			{
				// Stop pasting so we don't get the user
				// too annoyed with a message box for each
				// function. They all will fail, because
				// the user can't select functions from more
				// than one device (or global) at a time
				break;
			}
		}
	}
}

/**
 * Copy the given function to the given device (or global)
 */
Function* FunctionManager::copyFunction(t_function_id fid, t_device_id did)
{
	Function* newFunction = NULL;
	QString msg;

	Function* function = _app->doc()->function(fid);
	assert(function);

	// Check that we are not trying to copy global functions to a device
	// or vice versa
	if (did == KNoID)
	{
		if (function->type() == Function::Scene)
		{
			msg = "Scenes cannot be created to global space.\n";
			msg += "Paste aborted.\n";
		}
		else if (function->type() == Function::EFX)
		{
			msg = "EFX's cannot be created to global space.\n";
			msg += "Paste aborted.\n";
		}
		else if (function->type() == Function::Sequence)
		{
			msg = "Sequences cannot be created to global space.\n";
			msg += "Paste aborted.\n";
		}
		else
		{
			msg = QString::null;
		}
	}
	else if (did != KNoID)
	{
		if (function->type() == Function::Chaser)
		{
			msg = "Chasers cannot be created for fixtures.\n";
			msg += "Paste aborted.\n";
		}
		else if (function->type() == Function::Collection)
		{
			msg = "Collections cannot be created for fixtures.\n";
			msg += "Paste aborted.\n";
		}
		else
		{
			msg = QString::null;
		}
	}

	if (msg)
	{
		// Display error message and exit without creating functions
		QMessageBox::warning(this, KApplicationNameShort, msg);
		return NULL;
	}

	switch(function->type())
	{
		case Function::Scene:
		{
			newFunction =
			  _app->doc()->newFunction(Function::Scene, did);

			Scene* scene = static_cast<Scene*> (newFunction);

			scene->copyFrom(static_cast<Scene*> (function), did);
		}
		break;

		case Function::Chaser:
		{
			newFunction =
			  _app->doc()->newFunction(Function::Chaser, KNoID);

			Chaser* chaser = static_cast<Chaser*> (newFunction);

			chaser->copyFrom(static_cast<Chaser*> (function));
		}
		break;

		case Function::Collection:
		{
			newFunction =
			  _app->doc()->newFunction(Function::Collection, KNoID);
			
			FunctionCollection* fc =
				static_cast<FunctionCollection*> (newFunction);

			fc->copyFrom(static_cast<FunctionCollection*> (function));
		}
		break;

		case Function::Sequence:
		{
			newFunction =
			  _app->doc()->newFunction(Function::Sequence, did);
			Sequence* sequence =
				static_cast<Sequence*> (newFunction);

			sequence->copyFrom(static_cast<Sequence*> (function),
					   did);
		}
		break;

		case Function::EFX:
		{
			newFunction = 
			  _app->doc()->newFunction(Function::EFX, did);

			EFX* efx = static_cast<EFX*> (newFunction);

			efx->copyFrom(static_cast<EFX*> (function), did);
		}
		break;

		default:
			newFunction = NULL;
		break;
	}

	return newFunction;
}

/**
 * Edit the selected function
 */
int FunctionManager::slotEdit()
{
	Function* function = NULL;
	int result;

	QListViewItem* item = m_selectedFunctions.first();
	if (item == NULL)
	{
		return QDialog::Rejected;
	}

	//
	// Find function
	//
	function = _app->doc()->function(item->text(KColumnID).toInt());
	if (function == NULL)
	{
		return QDialog::Rejected;
	}

	//
	// Edit the function
	//
	switch (function->type())
	{
		case Function::Scene:
		{
			AdvancedSceneEditor* ase = NULL;
			ase = new AdvancedSceneEditor(this, (Scene*) function);
			ase->init();
			result = ase->exec();
			delete ase;
		}
		break;

		case Function::Chaser:
		{
			ChaserEditor* ce = new ChaserEditor(
					(Chaser*) function, NULL);
			ce->init();
			result = ce->exec();
			delete ce;
		}
		break;

		case Function::Collection:
		{
			FunctionCollectionEditor* fce = NULL;
			fce = new FunctionCollectionEditor(
					(FunctionCollection*) function, NULL);
			result = fce->exec();
			delete fce;
		}
		break;

		case Function::Sequence:
		{
			SequenceEditor* se = NULL;
			se = new SequenceEditor((Sequence*) function);
			se->init();
			result = se->exec();
			delete se;
		}
		break;

		case Function::EFX:
		{
			EFXEditor* ee = NULL;
			ee = new EFXEditor((EFX*) function);
			ee->init();
			result = ee->exec();
			delete ee;
		}
		break;

		default:
			result = QDialog::Rejected;
		break;
	}

	updateFunctionItem(item, function);

	return result;
}

/**
 * Delete selected functions
 */
void FunctionManager::slotDelete()
{
	Function* function = NULL;
	QListViewItem* item = NULL;
	QPtrListIterator<QListViewItem> it(m_selectedFunctions);
	QString msg;

	if (m_selectedFunctions.count() <= 0)
	{
		// Nothing to do
		return;
	}

	msg = "Do you want to delete function(s):\n\n";

	// Append functions' names to the message
	while ( (item = it.current()) != NULL)
	{
		function = _app->doc()->function(
			item->text(KColumnID).toInt());
		assert(function);
		msg += item->text(KColumnName);
		msg += QString("\n");
		++it;
	}

	msg += QString("\n");

	// Ask for user's confirmation
	if (QMessageBox::information(this, KApplicationNameShort, msg,
				     QMessageBox::Yes, QMessageBox::No)
		== QMessageBox::Yes)
	{
		deleteSelectedFunctions();
		updateMenuItems();
	}
}


/**
 * Callback for menu item "Select All"; selects all visible functions
 */
void FunctionManager::slotSelectAll()
{
	QListViewItem* item = NULL;

	for (item = m_functionTree->firstChild();
	     item != NULL;
	     item = item->nextSibling())
	{
		m_functionTree->setSelected(item, true);
	}
}


/**
 * OK clicked in selection mode
 */
void FunctionManager::slotOKClicked()
{
	QListViewItem* item = NULL;
	QPtrListIterator<QListViewItem> it(m_selectedFunctions);

	m_selection.clear();

	// Append selected functions' IDs to the selection list
	while ( (item = it.current()) != NULL)
	{
		m_selection.append(item->text(KColumnID).toInt());

		// Set this only if there are selected items
		m_result = QDialog::Accepted;

		++it;
	}

	close();
}


/**
 * Cancel clicked in selection mode
 */
void FunctionManager::slotCancelClicked()
{
	m_result = QDialog::Rejected;
	close();
}
