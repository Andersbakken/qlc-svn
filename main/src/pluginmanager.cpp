/*
  Q Light Controller
  pluginmanager.cpp

  Copyright (C) 2006 Heikki Junnila
  
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

#include "app.h"
#include "pluginmanager.h"

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

static const int KColumnName = 0;
static const int KColumnID   = 1;

/**
 * Constructor
 *
 */
PluginManager::PluginManager(QWidget* parent, const char* name)
  : QWidget(parent, name),

    m_layout   ( NULL ),
    m_dockArea ( NULL ),
    m_toolbar  ( NULL ),
    m_splitter ( NULL ),
    m_listView ( NULL ),
    m_textView ( NULL )
{
}


/**
 * Destructor
 *
 */
PluginManager::~PluginManager()
{
}


/**
 * Create the plugin manager view
 *
 */
void PluginManager::initView()
{
  // Create a vertical layout to this widget
  m_layout = new QVBoxLayout(this);
  
  // Init the title and icon
  initTitle();
  
  // Set up toolbar
  initToolBar();
  
  // Init the device view and text view
  initDataView();

  // Update the view
  fillPlugins();
}

/**
 * Initialize the window title
 *
 */
void PluginManager::initTitle()
{
  // Set the name
  setCaption(QString("Plugin Manager - Just a prototype for now, don't use!!"));
  
  // Set an icon TODO: better icon for plugin idiom
  setIcon(QString(PIXMAPS) + QString("/plugin.png"));
}


/**
 * Initialize the plugin manager toolbar
 *
 */
void PluginManager::initToolBar()
{
  // Create a dock area for the toolbar
  m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
  m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  
  // Add the dock area to the top of the vertical layout
  m_layout->addWidget(m_dockArea);

  //
  // Add a toolbar to the dock area
  //
  m_toolbar = new QToolBar("Plugin Manager", _app, m_dockArea);

  m_configureButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/configure.png"))),
		    "Configure", 0, this,
		    SLOT(slotConfigure()), m_toolbar);
  m_configureButton->setUsesTextLabel(true);
  QToolTip::add(m_configureButton, "Configure the plugin");
}


/**
 * Initialize the treeview and textview components
 *
 */
void PluginManager::initDataView()
{
  // Create a splitter to divide list view and text view
  m_splitter = new QSplitter(this);
  m_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_layout->addWidget(m_splitter);

  // Create the list view
  m_listView = new QListView(m_splitter);
  m_splitter->setResizeMode(m_listView, QSplitter::Auto);

  m_listView->setRootIsDecorated(true);
  m_listView->setMultiSelection(false);
  m_listView->setAllColumnsShowFocus(true);
  m_listView->setSorting(KColumnName, true);
  m_listView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  m_listView->header()->setClickEnabled(true);
  m_listView->header()->setResizeEnabled(false);
  m_listView->header()->setMovingEnabled(false);

  m_listView->addColumn("Plugin");
  m_listView->setResizeMode(QListView::LastColumn);

  connect(m_listView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));
  
  connect(m_listView, SIGNAL(doubleClicked(QListViewItem*)),
	  this, SLOT(slotDoubleClicked(QListViewItem*)));

  connect(m_listView, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
	  this, SLOT(slotRightButtonClicked(QListViewItem*, const QPoint&, int)));

  // Create the text view
  m_textView = new QTextView(m_splitter);
  m_splitter->setResizeMode(m_textView, QSplitter::Auto);
}


/**
 * Update the list view
 *
 */
void PluginManager::fillPlugins()
{
  QPtrListIterator <Plugin> it(*_app->pluginList());
  QListViewItem* item = NULL;
  QListViewItem* inputParent = NULL;
  QListViewItem* outputParent = NULL;
  Plugin* plugin = NULL;
  QString id;

  m_listView->clear();

  /* Input plugins' parent node */
  inputParent = new QListViewItem(m_listView);
  inputParent->setText(KColumnName, "Input");
  inputParent->setText(KColumnID, KPluginID);
  inputParent->setOpen(true);

  /* Output plugins' parent node */
  outputParent = new QListViewItem(m_listView);
  outputParent->setText(KColumnName, "Output");
  outputParent->setText(KColumnID, KPluginID);
  outputParent->setOpen(true);

  while ((plugin = it.current()) != NULL)
    {
      if (plugin->type() == Plugin::InputType)
	{
	  item = new QListViewItem(inputParent);
	}
      else
	{
	  item = new QListViewItem(outputParent);
	}
      
      assert(item);
      item->setText(KColumnName, plugin->name());
      
      id.sprintf("%d", plugin->id());
      item->setText(KColumnID, id);

      /* Catch activation signals */
      connect(plugin, SIGNAL(activated(Plugin*)),
	      this, SLOT(slotPluginActivated(Plugin*)));

      ++it;
    }

  updateActivePlugins();
}


/**
 * Update all plugins' active status
 *
 */
void PluginManager::updateActivePlugins()
{
  QListViewItem* parent = NULL;

  /* Input plugins */
  parent = m_listView->firstChild();
  updateActiveStatus(parent);

  /* Output plugins */
  parent = parent->nextSibling();
  updateActiveStatus(parent);
}


/**
 * Update the active status of the plugins under the given parent item
 *
 */
void PluginManager::updateActiveStatus(QListViewItem* parent)
{
  t_plugin_id id = 0;
  Plugin* plugin = NULL;
  QListViewItem* item = NULL;

  if (parent == NULL)
    return;

  item = parent->firstChild();
  if (item == NULL)
      return;

  do {
      id = item->text(KColumnID).toInt();
      plugin = _app->searchPlugin(id);
      if (plugin)
	{
	  if (plugin->isOpen() == true)
	    {
	      item->setPixmap(KColumnName,
			      QPixmap(QString(PIXMAPS) +
				      QString("/apply.png")));
	    }
	  else
	    {
	      item->setPixmap(KColumnName, NULL);
	    }
	}
    } while (item = item->nextSibling());
}


/**
 * Emit a closed() event so App* can delete plugin manager's pointer
 *
 */
void PluginManager::closeEvent(QCloseEvent* e)
{
  emit closed();
}


/**
 * Callback for double clicks on the list view
 *
 */
void PluginManager::slotDoubleClicked(QListViewItem* item)
{
  slotConfigure();
}


/**
 * Configure the plugin
 *
 */
void PluginManager::slotConfigure()
{
  QListViewItem* item = NULL;
  Plugin* plugin = NULL;
  t_plugin_id id = 0;

  item = m_listView->currentItem();
  if (item)
    {
      id = item->text(KColumnID).toInt();

      if (id == KPluginID) // Root item
	return;

      plugin = _app->searchPlugin(id);

      if (plugin)
	{
	  plugin->configure();
	}
      else
	{
	  QMessageBox::critical(this, KApplicationNameShort,
				"Unable to access plugin!");
	}
    }
}


/**
 * A listview item has been selected. Update the info text
 *
 */
void PluginManager::slotSelectionChanged(QListViewItem* item)
{
  Plugin* plugin = NULL;
  t_plugin_id id = 0;

  if (item)
    {
      id = item->text(KColumnID).toInt();

      if (id == KPluginID) // Root item
	{
	  m_textView->setText("");
	}
      else
	{
	  plugin = _app->searchPlugin(id);
	  if (plugin)
	    {
	      m_textView->setText(plugin->infoText());
	    }
	  else
	    {
	      m_textView->setText("");
	    }
	}
    }
}


/**
 * A plugin has been activated
 *
 */
void PluginManager::slotPluginActivated(Plugin* plugin)
{
  updateActivePlugins();
}


/**
 * Right mouse button has been pressed in the listview
 *
 */
void PluginManager::slotRightButtonClicked(QListViewItem* item,
					   const QPoint& point, int col)
{
  QPopupMenu* menu = NULL;
  int index = 0;

  if (item == NULL)
    return;

  menu = new QPopupMenu();
  menu->setCheckable(false);

  index = menu->insertItem(QPixmap(QString(PIXMAPS) + 
				   QString("/configure.png")),
			   "Configure...", this, SLOT(slotConfigure()));

  /* Disable configure option for root items */
  if (item->text(KColumnID).toInt() == KPluginID)
    menu->setItemEnabled(index, false);

  menu->exec(point, 0);
  delete menu;
}

