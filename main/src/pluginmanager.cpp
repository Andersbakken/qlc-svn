/*
  Q Light Controller
  pluginmanager.cpp

  Copyright (c) Heikki Junnila
  
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

#include "app.h"
#include "dmxmap.h"
#include "pluginmanager.h"

extern App* _app;

static const QString KInputNode ("Input");
static const QString KOutputNode ("Output");

static const int KColumnName ( 0 );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

PluginManager::PluginManager(QWidget* parent) :
	QWidget(parent, "Plugin Manager")
{
	m_layout = NULL;
	m_dockArea = NULL;
	m_toolbar = NULL;
	m_splitter = NULL;
	m_listView = NULL;
	m_textView = NULL;

	// Create a vertical layout to this widget
	m_layout = new QVBoxLayout(this);
  
	// Init the title and icon
	initTitle();
  
	// Set up toolbar
	initToolBar();
  
	// Create the list and text views
	initDataView();

	// Update plugins to list view
	fillPlugins();

	// Update text view
	slotSelectionChanged(NULL);
}

PluginManager::~PluginManager()
{
}

void PluginManager::initTitle()
{
	// Set the name
	setCaption(QString("Plugin Manager"));
  
	// Set an icon TODO: better icon for plugin idiom
	setIcon(QString(PIXMAPS) + QString("/plugin.png"));
}

void PluginManager::initToolBar()
{
	// Create a dock area for the toolbar
	m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
	m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  
	// Add the dock area to the top of the vertical layout
	m_layout->addWidget(m_dockArea);

	// Add a toolbar to the dock area
	m_toolbar = new QToolBar("Plugin Manager", _app, m_dockArea);
	m_toolbar->setMovingEnabled(false);

	// Configure button
	m_configureButton =
		new QToolButton(QPixmap(QString(PIXMAPS) +
					QString("/configure.png")),
				"Configure",
				0,
				this,
				SLOT(slotConfigureClicked()),
				m_toolbar);
	m_configureButton->setUsesTextLabel(true);
	QToolTip::add(m_configureButton, "Configure the plugin");

	// Map button
	m_mapButton = new QToolButton(QPixmap(QString(PIXMAPS) +
					      QString("/attach.png")),
				      "Output Mapper",
				      0,
				      this,
				      SLOT(slotDMXMapperClicked()),
				      m_toolbar);
	m_mapButton->setUsesTextLabel(true);
	QToolTip::add(m_mapButton, "Patch QLC universes to output plugins");
}

void PluginManager::initDataView()
{
	// Create a splitter to divide list view and text view
	m_splitter = new QSplitter(this);
	m_splitter->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
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

	connect(m_listView,
		SIGNAL(selectionChanged(QListViewItem*)),
		this,
		SLOT(slotSelectionChanged(QListViewItem*)));
  
	connect(m_listView,
		SIGNAL(doubleClicked(QListViewItem*)),
		this,
		SLOT(slotConfigureClicked()));

	connect(m_listView,
		SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)),
		this,
		SLOT(slotRightButtonClicked(QListViewItem*, const QPoint&, int)));

	// Create the text view
	m_textView = new QTextView(m_splitter);
	m_splitter->setResizeMode(m_textView, QSplitter::Auto);
}

void PluginManager::fillPlugins()
{
	QListViewItem* item = NULL;
	QListViewItem* parent = NULL;
	QStringList::iterator it;

	m_listView->clear();

	/* Output plugins */
	parent = new QListViewItem(m_listView, KOutputNode);
	parent->setOpen(true);

	QStringList list = _app->dmxMap()->pluginNames();
	for (it = list.begin(); it != list.end(); ++it)
		new QListViewItem(parent, *it);

	/* Input plugins */
	parent = new QListViewItem(m_listView, KInputNode);
	parent->setOpen(true);
}

/*****************************************************************************
 * Menu & tool button slots
 *****************************************************************************/

void PluginManager::slotConfigureClicked()
{
	QListViewItem* item = NULL;
	QString pluginName;
	
	item = m_listView->currentItem();
	if (item != NULL)
	{
		pluginName = item->text(KColumnName);
		if (pluginName == KOutputNode)
			;
		else if (pluginName == KInputNode)
			;
		else
			_app->dmxMap()->configurePlugin(pluginName);
	}
}


void PluginManager::slotDMXMapperClicked()
{
	_app->dmxMap()->openEditor(this);
	slotSelectionChanged(m_listView->currentItem());
}

/*****************************************************************************
 * List view slots
 *****************************************************************************/

void PluginManager::slotSelectionChanged(QListViewItem* item)
{
	QString status;
	QString name;
	QString parent;

	if (item != NULL)
	{
		name = item->text(KColumnName);

		if (item->parent() == NULL)
		{
			m_configureButton->setEnabled(false);
			if (name == KOutputNode)
				status = _app->dmxMap()->pluginStatus();
			else if (name == KInputNode)
				status = QString("TODO"); // TODO
		}
		else
		{
			m_configureButton->setEnabled(true);
			parent = item->parent()->text(KColumnName);

			if (parent == KOutputNode)
				status = _app->dmxMap()->pluginStatus(name);
			else if (parent == KInputNode)
				status = QString("TODO"); // TODO
		}
	}
	else
	{
		m_configureButton->setEnabled(false);
	}

	if (status.length() == 0)
	{
		status = QString("<FONT SIZE=\"5\">No selection</FONT>\n");
		status += QString("<BR>\n");
		status += QString("Select a plugin to begin.");
	}

	m_textView->setText(status);
}

void PluginManager::slotRightButtonClicked(QListViewItem* item,
					   const QPoint& point,
					   int col)
{
	if (item == NULL)
		return;

	QPopupMenu menu;
	menu.setCheckable(false);
	menu.insertItem(QPixmap(QString(PIXMAPS) + 
				QString("/configure.png")),
			"Configure...",
			this,
			SLOT(slotConfigureClicked()));

	menu.exec(point, 0);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void PluginManager::closeEvent(QCloseEvent* e)
{
	emit closed();
}
