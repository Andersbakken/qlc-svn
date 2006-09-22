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

  m_activateButton =
    new QToolButton(QIconSet(QPixmap(QString(PIXMAPS) + QString("/apply.png"))),
		    "Activate", 0, this,
		    SLOT(slotActivate()), m_toolbar);
  m_activateButton->setUsesTextLabel(true);
  QToolTip::add(m_activateButton, "Activate the plugin" );
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

  m_listView->addColumn("Plugins");
  m_listView->setResizeMode(QListView::LastColumn);

  connect(m_listView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));
  
  connect(m_listView, SIGNAL(doubleClicked(QListViewItem*)),
	  this, SLOT(slotDoubleClicked(QListViewItem*)));

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
  QListViewItem* item = NULL;

  m_listView->clear();

  /* Input plugins */
  item = new QListViewItem(m_listView);
  item->setText(KColumnName, "Input");
  item->setText(KColumnID, KPluginID);

  fillInputPlugins(item);

  /* Output plugins */
  item = new QListViewItem(m_listView);
  item->setText(KColumnName, "Output");
  item->setText(KColumnID, KPluginID);

  fillOutputPlugins(item);
}

/**
 * Fill in the input plugins
 *
 */
void PluginManager::fillInputPlugins(QListViewItem* parent)
{
  QPtrListIterator <Plugin> it(*_app->pluginList());
  QListViewItem* item = NULL;
  int i = 0;

  while (it.current() != NULL)
    {
      if (it.current()->type() == Plugin::InputType)
	{
	  item = new QListViewItem(parent);
	  item->setText(KColumnName, it.current()->name());
	  item->setText(KColumnID, QString::number(it.current()->id()));
	} 
     
      ++it;
    }
}

/**
 * Fill in the output plugins
 *
 */
void PluginManager::fillOutputPlugins(QListViewItem* parent)
{
  QPtrListIterator <Plugin> it(*_app->pluginList());
  QListViewItem* item = NULL;
  int i = 0;

  while (it.current() != NULL)
    {
      if (it.current()->type() == Plugin::OutputType)
	{
	  item = new QListViewItem(parent);
	  item->setText(KColumnName, it.current()->name());
	  item->setText(KColumnID, QString::number(it.current()->id()));
	} 
     
      ++it;
    }
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
