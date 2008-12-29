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

#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QPixmap>
#include <QMenu>
#include <QList>

#include "collectioneditor.h"
#include "functionmanager.h"
#include "chasereditor.h"
#include "sceneeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "function.h"
#include "chaser.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "efx.h"

#define KColumnName 0
#define KColumnBus  1
#define KColumnID   2

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionManager::FunctionManager(QWidget* parent) : QWidget(parent)
{
	m_blockAddFunctionSignal = false;
	m_blockRemoveFunctionSignal = false;

	new QVBoxLayout(this);

	initActions();
	initMenu();
	initToolbar();

	// Create function tree
	initFunctionTree();

	// Get all functions
	updateFunctionTree();
}

FunctionManager::~FunctionManager()
{
}

void FunctionManager::update()
{
	updateFunctionTree();
}

/*****************************************************************************
 * Doc signal handlers
 *****************************************************************************/

void FunctionManager::slotFunctionAdded(t_function_id fid)
{
	QTreeWidgetItem* item;
	Function* function;

	// The function manager has its own routines for functions that are
	// created with it.
	if (m_blockAddFunctionSignal == true)
		return;

	// Create a new item for the function
	item = new QTreeWidgetItem(m_functionTree);
	function = _app->doc()->function(fid);
	updateFunctionItem(item, function);
}

void FunctionManager::slotFunctionRemoved(t_function_id id)
{
	QTreeWidgetItem* item;

	// The function manager has its own routines for functions that are
	// removed with it.
	if (m_blockRemoveFunctionSignal == true)
		return;

	item = getItem(id, m_functionTree);
	if (item != NULL)
	{
		if (item->isSelected() == true)
		{
			QTreeWidgetItem* nextItem;

			// Try to select the closest neighbour
			if (m_functionTree->itemAbove(item) != NULL)
				nextItem = m_functionTree->itemAbove(item);
			else
				nextItem = m_functionTree->itemBelow(item);
  
			if (nextItem != NULL)
				nextItem->setSelected(true);
		}

		delete item;
	}
}

void FunctionManager::slotFunctionChanged(t_function_id id)
{
	QTreeWidgetItem* item;

	item = getItem(id, m_functionTree);
	if (item != NULL)
	{
		Function* function = _app->doc()->function(id);
		updateFunctionItem(item, function);
	}
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FunctionManager::initActions()
{
	Q_ASSERT(parentWidget() != NULL);

	/* Manage actions */
	m_addSceneAction = new QAction(QIcon(":/scene.png"),
				       tr("New scene"), this);
	connect(m_addSceneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddScene()));

	m_addChaserAction = new QAction(QIcon(":/chaser.png"),
					tr("New chaser"), this);
	connect(m_addChaserAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddChaser()));

	m_addCollectionAction = new QAction(QIcon(":/collection.png"),
					    tr("New collection"), this);
	connect(m_addCollectionAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddCollection()));

	m_addEFXAction = new QAction(QIcon(":/efx.png"),
				     tr("New EFX"), this);
	connect(m_addEFXAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddEFX()));

	m_closeAction = new QAction(QIcon(":/fileclose.png"),
				    tr("Close"), this);
	connect(m_closeAction, SIGNAL(triggered(bool)),
		parentWidget(), SLOT(close()));

	/* Edit actions */
	m_editAction = new QAction(QIcon(":/edit.png"),
				   tr("Edit"), this);
	connect(m_editAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEdit()));

	m_cloneAction = new QAction(QIcon(":/editcopy.png"),
				    tr("Clone"), this);
	connect(m_cloneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotClone()));

	m_deleteAction = new QAction(QIcon(":/editdelete.png"),
				     tr("Delete"), this);
	connect(m_deleteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotDelete()));

	m_selectAllAction = new QAction(QIcon(":/selectall.png"),
					tr("Select all"), this);
	connect(m_selectAllAction, SIGNAL(triggered(bool)),
		this, SLOT(slotSelectAll()));
}

void FunctionManager::initMenu()
{
	QAction* action;

	layout()->setMenuBar(new QMenuBar(this));

	/* Function menu */
	m_manageMenu = new QMenu(this);
	m_manageMenu->setTitle(tr("Manage"));
	m_manageMenu->addAction(m_addSceneAction);
	m_manageMenu->addAction(m_addChaserAction);
	m_manageMenu->addAction(m_addEFXAction);
	m_manageMenu->addAction(m_addCollectionAction);
	m_manageMenu->addSeparator();
	m_manageMenu->addAction(m_closeAction);

	/* Edit menu */
	m_editMenu = new QMenu(this);
	m_editMenu->setTitle("Edit");
	m_editMenu->addAction(m_editAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_cloneAction);
	m_editMenu->addAction(m_selectAllAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_deleteAction);
	m_editMenu->addSeparator();

	/* Bus menu */
	m_busGroup = new QActionGroup(this);
	m_busMenu = new QMenu(this);
	m_busMenu->setTitle("Assign bus");
	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		/* <xx>: <name> */
		action = new QAction(QString("%1: %2").arg(id + 1, 2, 10,
							   QChar('0'))
				     .arg(Bus::name(id)), this);
		action->setCheckable(true);
		m_busGroup->addAction(action);
		m_busActions.append(action);
		m_busMenu->addAction(action);
	}

	/* Catch bus assignment changes */
	connect(m_busGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(slotBusTriggered(QAction*)));

	/* Catch bus name changes */
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

	/* Construct menu bar */
	static_cast<QMenuBar*>(layout()->menuBar())->addMenu(m_manageMenu);
	static_cast<QMenuBar*>(layout()->menuBar())->addMenu(m_editMenu);
	m_editMenu->addMenu(m_busMenu);
}

void FunctionManager::initToolbar()
{
	// Add a toolbar to the dock area
	m_toolbar = new QToolBar("Function Manager", this);
	m_toolbar->setFloatable(false);
	m_toolbar->setMovable(false);
	layout()->addWidget(m_toolbar);
	m_toolbar->addAction(m_addSceneAction);
	m_toolbar->addAction(m_addChaserAction);
	m_toolbar->addAction(m_addEFXAction);
	m_toolbar->addAction(m_addCollectionAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_editAction);
	m_toolbar->addAction(m_cloneAction);
	m_toolbar->addSeparator();
	m_toolbar->addAction(m_deleteAction);
}

void FunctionManager::slotBusTriggered(QAction* action)
{
	/* Bus actions are in ascending order in the list so the index of
	   the triggered action is also the ID of the bus represented by
	   the action. */
	t_bus_id busID = m_busActions.indexOf(action);

	/* Set the selected bus to all selected functions */
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		Function* function;

		item = it.next();
		Q_ASSERT(item != NULL);
		function = _app->doc()->function(item->text(KColumnID).toInt());
		Q_ASSERT(function != NULL);

		function->setBus(busID);
		updateFunctionItem(item, function);
	}
}

void FunctionManager::slotBusNameChanged(t_bus_id id, const QString& name)
{
	QAction* action;

	action = m_busActions.at(id);
	Q_ASSERT(action != NULL);

	action->setText(QString("%1: %2").arg(id).arg(name));
}

void FunctionManager::slotAddScene()
{
	addFunction(Function::Scene);
}

void FunctionManager::slotAddChaser()
{
	addFunction(Function::Chaser);
}

void FunctionManager::slotAddCollection()
{
	addFunction(Function::Collection);
}

void FunctionManager::slotAddEFX()
{
	addFunction(Function::EFX);
}

int FunctionManager::slotEdit()
{
	QTreeWidgetItem* item;
	Function* function;
	int result;

	if (m_functionTree->selectedItems().isEmpty() == true)
		return QDialog::Rejected;

	item = m_functionTree->selectedItems().first();
	Q_ASSERT(item != NULL);

	// Find the selected function
	function = _app->doc()->function(item->text(KColumnID).toInt());
	if (function == NULL)
		return QDialog::Rejected;

	// Edit the selected function
	switch (function->type())
	{
	case Function::Scene:
	{
		SceneEditor se(this, static_cast<Scene*> (function));
		result = se.exec();
	}
	break;

	case Function::Chaser:
	{
		ChaserEditor ce(this, static_cast<Chaser*> (function));
		result = ce.exec();
	}
	break;

	case Function::Collection:
	{
		CollectionEditor fce(this, static_cast<Collection*> (function));
		result = fce.exec();
	}
	break;

	case Function::EFX:
	{
		EFXEditor ee(this, static_cast<EFX*> (function));
		result = ee.exec();
	}
	break;

	default:
		result = QDialog::Rejected;
		break;
	}

	updateFunctionItem(item, function);

	return result;
}

void FunctionManager::slotClone()
{
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
		copyFunction(it.next()->text(KColumnID).toInt());
}

void FunctionManager::slotDelete()
{
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	QString msg;

	if (it.hasNext() == false)
		return;

	msg = "Do you want to delete function(s):\n";

	// Append functions' names to the message
	while (it.hasNext() == true)
		msg += it.next()->text(KColumnName) + QString("\n");

	// Ask for user's confirmation
	if (QMessageBox::question(this, "Delete function(s)", msg,
				  QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	{
		deleteSelectedFunctions();
		updateActionStatus();
	}
}

void FunctionManager::slotSelectAll()
{
	for (int i = 0; i < m_functionTree->topLevelItemCount(); i++)
		m_functionTree->topLevelItem(i)->setSelected(true);
}

void FunctionManager::updateActionStatus()
{
	if (m_functionTree->selectedItems().count() > 0)
	{
		/* At least one function has been selected, so
		   editing is possible. */
		m_editAction->setEnabled(true);
		m_cloneAction->setEnabled(true);

		m_deleteAction->setEnabled(true);
		m_selectAllAction->setEnabled(true);
	}
	else
	{
		/* No functions selected */
		m_editAction->setEnabled(false);
		m_cloneAction->setEnabled(false);

		m_deleteAction->setEnabled(false);
		m_selectAllAction->setEnabled(false);
	}

	/* Update bus menu actions in both cases */
	updateBusActions();
}

void FunctionManager::updateBusActions()
{
	QTreeWidgetItem* functionItem = m_functionTree->currentItem();
	if (functionItem == NULL)
	{
		/* No current function item selection, disable bus actions */
		m_busGroup->setEnabled(false);
	}
	else
	{
		Function* function;
		t_function_id fid;
		QAction* action;

		/* Find the selected function */
		fid = functionItem->text(KColumnID).toInt();
		function = _app->doc()->function(fid);
		Q_ASSERT(function != NULL);

		/* Collections don't have a bus ID and it's always KNoID.
		   Other functions' bus ID's can never be KNoID so it is
		   safe to check the function's bus-assignability this way */
		if (function->busID() == KNoID)
		{
			m_busGroup->setEnabled(false);
		}
		else
		{
			m_busGroup->setEnabled(true);

			/* Set the current function's bus checked */
			action = m_busActions.at(function->busID());
			Q_ASSERT(action != NULL);
			action->setChecked(true);
		}
	}
}

/****************************************************************************
 * Function tree
 ****************************************************************************/

void FunctionManager::initFunctionTree()
{
	m_functionTree = new QTreeWidget(this);
	layout()->addWidget(m_functionTree);

	// Add two columns for function and bus
	QStringList labels;
	labels << "Function" << "Bus";
	m_functionTree->setHeaderLabels(labels);
	m_functionTree->header()->setResizeMode(QHeaderView::ResizeToContents);
	m_functionTree->setRootIsDecorated(false);
	m_functionTree->setAllColumnsShowFocus(true);
	m_functionTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_functionTree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_functionTree->setSortingEnabled(true);
	m_functionTree->sortByColumn(KColumnName, Qt::AscendingOrder);

	// Catch selection changes
	connect(m_functionTree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotFunctionTreeSelectionChanged()));

	// Catch mouse double clicks
	connect(m_functionTree,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEdit()));

	// Catch right-mouse clicks
	connect(m_functionTree,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotFunctionTreeContextMenuRequested(const QPoint&)));
}

void FunctionManager::updateFunctionTree()
{
	m_functionTree->clear();
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		Function* function = _app->doc()->function(fid);
		if (function != NULL)
		{
			QTreeWidgetItem* item;
			item = new QTreeWidgetItem(m_functionTree);
			updateFunctionItem(item, function);
		}
	}
}

void FunctionManager::updateFunctionItem(QTreeWidgetItem* item,
					 Function* function)
{
	Q_ASSERT(item != NULL);
	Q_ASSERT(function != NULL);

	item->setText(KColumnName, function->name());
	item->setIcon(KColumnName, function->icon());
	item->setText(KColumnBus, function->busName());
	item->setText(KColumnID, QString::number(function->id()));
}

void FunctionManager::deleteSelectedFunctions()
{
	m_blockRemoveFunctionSignal = true;
	QListIterator <QTreeWidgetItem*> it(m_functionTree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		t_function_id fid;

		item = it.next();
		fid = item->text(KColumnID).toInt();
		_app->doc()->deleteFunction(fid);

		/* This is pretty weird, since QTreeWidget::selectedItems()
		   is const, but this seems to work so let's ship it... */
		delete item;
	}

	m_blockRemoveFunctionSignal = false;
}

void FunctionManager::slotFunctionTreeSelectionChanged()
{
	updateActionStatus();
}

void FunctionManager::slotFunctionTreeContextMenuRequested(const QPoint&)
{
	QMenu contextMenu(this);
	contextMenu.addMenu(m_manageMenu);
	contextMenu.addMenu(m_editMenu);

	updateActionStatus();

	contextMenu.exec(QCursor::pos());
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

Function* FunctionManager::copyFunction(t_function_id fid)
{
	Function* newFunction = NULL;
	QString msg;

	Function* function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	switch(function->type())
	{
	case Function::Scene:
	{
		newFunction = _app->doc()->newFunction(Function::Scene);
		Scene* scene = static_cast<Scene*> (newFunction);
		scene->copyFrom(static_cast<Scene*> (function));
		scene->setName("Copy of " + function->name());
	}
	break;

	case Function::Chaser:
	{
		newFunction = _app->doc()->newFunction(Function::Chaser);
		Chaser* chaser = static_cast<Chaser*> (newFunction);
		chaser->copyFrom(static_cast<Chaser*> (function));
		chaser->setName("Copy of " + function->name());
	}
	break;

	case Function::Collection:
	{
		newFunction =
			_app->doc()->newFunction(Function::Collection);

		Collection* fc = static_cast<Collection*> (newFunction);
		fc->copyFrom(static_cast<Collection*> (function));
		fc->setName("Copy of " + function->name());
	}
	break;

	case Function::EFX:
	{
		newFunction = _app->doc()->newFunction(Function::EFX);
		EFX* efx = static_cast<EFX*> (newFunction);
		efx->copyFrom(static_cast<EFX*> (function));
		efx->setName("Copy of " + function->name());
	}
	break;

	default:
		newFunction = NULL;
		break;
	}

	return newFunction;
}

void FunctionManager::addFunction(Function::Type type)
{
	QTreeWidgetItem* newItem;
	Function* function;

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = true;

	//
	// Create the function
	//
	switch(type)
	{
	case Function::Scene:
	{
		function = _app->doc()->newFunction(Function::Scene);
		if (function == NULL)
			return;
		function->setName(tr("New Scene"));
	}
	break;

	case Function::Chaser:
	{
		function = _app->doc()->newFunction(Function::Chaser);
		if (function == NULL)
			return;
		function->setName(tr("New Chaser"));
	}
	break;

	case Function::Collection:
	{
		function = _app->doc()->newFunction(Function::Collection);
		if (function == NULL)
			return;
		function->setName(tr("New Collection"));
	}
	break;

	case Function::EFX:
	{
		function = _app->doc()->newFunction(Function::EFX);
		if (function == NULL)
			return;
		function->setName(tr("New EFX"));
	}
	break;

	default:
		function = NULL;
		return;
		break;
	}

	// We don't want the signal handler to add the function twice
	m_blockAddFunctionSignal = false;

	// Create a new item for the function
	newItem = new QTreeWidgetItem(m_functionTree);

	// Update the item's contents based on the function itself
	updateFunctionItem(newItem, function);

	// Clear current selection so that slotEdit() behaves correctly
	m_functionTree->clearSelection();

	// Select only the new item
	newItem->setSelected(true);

	if (slotEdit() == QDialog::Rejected)
	{
		// Edit dialog was rejected -> delete function
		deleteSelectedFunctions();
	}
	else
	{
		// Sort the list so that the new item takes its correct place
		m_functionTree->sortItems(KColumnName, Qt::AscendingOrder);

		// Make sure that the new item is visible
		m_functionTree->scrollToItem(newItem);
	}
}

QTreeWidgetItem* FunctionManager::getItem(t_function_id id,
					  QTreeWidget* listView)
{
	Q_ASSERT(listView != NULL);

	QTreeWidgetItemIterator it(listView);

	while (*it != NULL)
	{
		if ((*it)->text(KColumnID).toInt() == id)
			return *it;
		++it;
	}

	return NULL;
}
