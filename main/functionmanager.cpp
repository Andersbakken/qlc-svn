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
#include <QMdiSubWindow>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QSettings>
#include <QMdiArea>
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

FunctionManager* FunctionManager::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionManager::FunctionManager(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags)
{
	new QVBoxLayout(this);

	initActions();
	initMenu();
	initToolbar();

	initTree();
	updateTree();
	updateActionStatus();
}

FunctionManager::~FunctionManager()
{
	QSettings settings;
	QRect rect;

#ifdef _APPLE_
	rect = this->rect();
#else
	rect = parentWidget()->rect();
#endif
	settings.setValue("functionmanager/width", rect.width());
	settings.setValue("functionmanager/height", rect.height());

	FunctionManager::s_instance = NULL;
}

void FunctionManager::create(QWidget* parent)
{
	QWidget* window;

	/* Must not create more than one instance */
	if (s_instance != NULL)
		return;

#ifdef _APPLE_
	/* Create a separate window for OSX */
	s_instance = new FunctionManager(parent, Qt::Window);
	window = s_instance;
#else
	/* Create an MDI window for X11 & Win32 */
	QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
	Q_ASSERT(area != NULL);
	s_instance = new FunctionManager(parent);
	window = area->addSubWindow(s_instance);
#endif

	/* Set some common properties for the window and show it */
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->setWindowIcon(QIcon(":/function.png"));
	window->setWindowTitle(tr("Function Manager"));
	window->setContextMenuPolicy(Qt::CustomContextMenu);
	window->show();

	connect(_app, SIGNAL(modeChanged(App::Mode)),
		s_instance, SLOT(slotAppModeChanged(App::Mode)));
	connect(_app, SIGNAL(documentChanged(Doc*)),
		s_instance, SLOT(slotDocumentChanged()));

	QSettings settings;
	QVariant w = settings.value("functionmanager/width");
	QVariant h = settings.value("functionmanager/height");
	if (w.isValid() == true && h.isValid() == true)
		window->resize(w.toInt(), h.toInt());
	else
		window->resize(600, 400);
}

void FunctionManager::slotAppModeChanged(App::Mode mode)
{
	/* Close this when entering operate mode */
	if (mode == App::Operate)
#ifdef _APPLE_
		deleteLater();
#else
		parent()->deleteLater();
#endif
}

void FunctionManager::slotDocumentChanged()
{
	updateTree();
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FunctionManager::initActions()
{
	/* Manage actions */
	m_addSceneAction = new QAction(QIcon(":/scene.png"),
				       tr("New &scene"), this);
	m_addSceneAction->setShortcut(QKeySequence("CTRL+S"));
	connect(m_addSceneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddScene()));

	m_addChaserAction = new QAction(QIcon(":/chaser.png"),
					tr("New c&haser"), this);
	m_addChaserAction->setShortcut(QKeySequence("CTRL+H"));
	connect(m_addChaserAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddChaser()));

	m_addCollectionAction = new QAction(QIcon(":/collection.png"),
					    tr("New c&ollection"), this);
	m_addCollectionAction->setShortcut(QKeySequence("CTRL+O"));
	connect(m_addCollectionAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddCollection()));

	m_addEFXAction = new QAction(QIcon(":/efx.png"),
				     tr("New E&FX"), this);
	m_addEFXAction->setShortcut(QKeySequence("CTRL+F"));
	connect(m_addEFXAction, SIGNAL(triggered(bool)),
		this, SLOT(slotAddEFX()));

	/* Edit actions */
	m_editAction = new QAction(QIcon(":/edit.png"),
				   tr("&Edit"), this);
	m_editAction->setShortcut(QKeySequence("CTRL+E"));
	connect(m_editAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEdit()));

	m_cloneAction = new QAction(QIcon(":/editcopy.png"),
				    tr("&Clone"), this);
	m_cloneAction->setShortcut(QKeySequence("CTRL+C"));
	connect(m_cloneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotClone()));

	m_deleteAction = new QAction(QIcon(":/editdelete.png"),
				     tr("&Delete"), this);
	m_deleteAction->setShortcut(QKeySequence("Delete"));
	connect(m_deleteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotDelete()));

	m_selectAllAction = new QAction(QIcon(":/selectall.png"),
					tr("Select &all"), this);
	m_selectAllAction->setShortcut(QKeySequence("CTRL+A"));
	connect(m_selectAllAction, SIGNAL(triggered(bool)),
		this, SLOT(slotSelectAll()));
}

void FunctionManager::initMenu()
{
	QAction* action;

	layout()->setMenuBar(new QMenuBar(this));

	/* Function menu */
	m_addMenu = new QMenu(this);
	m_addMenu->setTitle(tr("&Add"));
	m_addMenu->addAction(m_addSceneAction);
	m_addMenu->addAction(m_addChaserAction);
	m_addMenu->addAction(m_addEFXAction);
	m_addMenu->addAction(m_addCollectionAction);

	/* Edit menu */
	m_editMenu = new QMenu(this);
	m_editMenu->setTitle("&Edit");
	m_editMenu->addAction(m_editAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_cloneAction);
	m_editMenu->addAction(m_selectAllAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_deleteAction);
	m_editMenu->addSeparator();

	/* Bus menu */
	m_busGroup = new QActionGroup(this);
	m_busGroup->setExclusive(false);
	m_busMenu = new QMenu(this);
	m_busMenu->setTitle("Assign &bus");
	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		/* <xx>: <name> */
		action = new QAction(QString("%1: %2").arg(id + 1)
				     .arg(Bus::name(id)), this);
		action->setCheckable(false);
		action->setData(id);
		m_busGroup->addAction(action);
		m_busMenu->addAction(action);
	}

	/* Catch bus assignment changes */
	connect(m_busGroup, SIGNAL(triggered(QAction*)),
		this, SLOT(slotBusTriggered(QAction*)));

	/* Catch bus name changes */
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

	/* Construct menu bar */
	static_cast<QMenuBar*>(layout()->menuBar())->addMenu(m_addMenu);
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
	t_bus_id bus;

	Q_ASSERT(action != NULL);

	bus = action->data().toInt();

	/* Set the selected bus to all selected functions */
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		Function* function;

		item = it.next();
		Q_ASSERT(item != NULL);

		function = _app->doc()->function(item->text(KColumnID).toInt());
		Q_ASSERT(function != NULL);

		function->setBus(bus);
		updateFunctionItem(item, function);
	}
}

void FunctionManager::slotBusNameChanged(t_bus_id id, const QString& name)
{
	/* Change the menu item's name to reflect the new bus name */
	QListIterator <QAction*> it(m_busGroup->actions());
	while (it.hasNext() == true)
	{
		QAction* action = it.next();
		Q_ASSERT(action != NULL);

		if (action->data().toInt() == id)
		{
			action->setText(QString("%1: %2")
					.arg(id + 1).arg(name));
			break;
		}
	}

	/* Change all affected function item's bus names as well */
	QListIterator <QTreeWidgetItem*> twit(
			m_tree->findItems(QString("%1: ").arg(id + 1),
						  Qt::MatchStartsWith,
						  KColumnBus));
	while (twit.hasNext() == true)
		twit.next()->setText(KColumnBus,
				     QString("%1: %2").arg(id + 1).arg(name));
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

	if (m_tree->selectedItems().isEmpty() == true)
		return QDialog::Rejected;

	item = m_tree->selectedItems().first();
	Q_ASSERT(item != NULL);

	// Find the selected function
	function = _app->doc()->function(item->text(KColumnID).toInt());
	if (function == NULL)
		return QDialog::Rejected;

	// Edit the selected function
	result = function->edit();

	updateFunctionItem(item, function);

	return result;
}

void FunctionManager::slotClone()
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
		copyFunction(it.next()->text(KColumnID).toInt());
}

void FunctionManager::slotDelete()
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	QString msg;

	if (it.hasNext() == false)
		return;

	msg = "Do you want to DELETE:\n";

	// Append functions' names to the message
	while (it.hasNext() == true)
		msg += it.next()->text(KColumnName) + QString("\n");

	// Ask for user's confirmation
	if (QMessageBox::question(this, "Delete Functions", msg,
				  QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::Yes)
	{
		deleteSelectedFunctions();
		updateActionStatus();
	}
}

void FunctionManager::slotSelectAll()
{
	/* This has to be done thru an intermediary slot because the tree
	   widget hasn't been created when actions are being created and
	   so a direct slot collection to m_tree is not possible. */
	m_tree->selectAll();
}

void FunctionManager::updateActionStatus()
{
	if (m_tree->selectedItems().isEmpty() == false)
	{
		/* At least one function has been selected, so
		   editing is possible. */
		m_editAction->setEnabled(true);
		m_cloneAction->setEnabled(true);

		m_deleteAction->setEnabled(true);
		m_selectAllAction->setEnabled(true);

		m_busGroup->setEnabled(true);
	}
	else
	{
		/* No functions selected */
		m_editAction->setEnabled(false);
		m_cloneAction->setEnabled(false);

		m_deleteAction->setEnabled(false);
		m_selectAllAction->setEnabled(false);

		m_busGroup->setEnabled(false);
	}
}

/****************************************************************************
 * Function tree
 ****************************************************************************/

void FunctionManager::initTree()
{
	m_tree = new QTreeWidget(this);
	layout()->addWidget(m_tree);

	// Add two columns for function and bus
	QStringList labels;
	labels << "Function" << "Bus";
	m_tree->setHeaderLabels(labels);
	m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
	m_tree->setRootIsDecorated(false);
	m_tree->setAllColumnsShowFocus(true);
	m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tree->setSortingEnabled(true);
	m_tree->sortByColumn(KColumnName, Qt::AscendingOrder);

	// Catch selection changes
	connect(m_tree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotTreeSelectionChanged()));

	// Catch mouse double clicks
	connect(m_tree,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEdit()));

	// Catch right-mouse clicks
	connect(m_tree,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotTreeContextMenuRequested(const QPoint&)));
}

void FunctionManager::updateTree()
{
	m_tree->clear();
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		Function* function = _app->doc()->function(fid);
		if (function != NULL)
		{
			QTreeWidgetItem* item;
			item = new QTreeWidgetItem(m_tree);
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
	item->setText(KColumnBus, QString("%1: %2").arg(function->busID() + 1)
				  .arg(Bus::name(function->busID())));
	item->setText(KColumnID, QString("%1").arg(function->id()));
}

void FunctionManager::deleteSelectedFunctions()
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		t_function_id fid;

		item = it.next();
		fid = item->text(KColumnID).toInt();
		_app->doc()->deleteFunction(fid);

		delete item;
	}

}

void FunctionManager::slotTreeSelectionChanged()
{
	updateActionStatus();
}

void FunctionManager::slotTreeContextMenuRequested(const QPoint& point)
{
	Q_UNUSED(point);

	QMenu contextMenu(this);
	contextMenu.addMenu(m_addMenu);
	contextMenu.addMenu(m_editMenu);

	updateActionStatus();

	contextMenu.exec(QCursor::pos());
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

void FunctionManager::copyFunction(t_function_id fid)
{
	Function* function = _app->doc()->function(fid);
	Q_ASSERT(function != NULL);

	/* Create a new item for the copied function */
	Function* newFunction = function->createCopy();
	if (newFunction != NULL)
	{
		QTreeWidgetItem* item;
		item = new QTreeWidgetItem(m_tree);
		updateFunctionItem(item, newFunction);
	}
}

void FunctionManager::addFunction(Function::Type type)
{
	QTreeWidgetItem* item;
	Function* function;

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

	/* Create a new item for the function */
	item = new QTreeWidgetItem(m_tree);
	updateFunctionItem(item, function);

	/* Clear current selection and select only the new one */
	m_tree->clearSelection();
	item->setSelected(true);

	/* Start editing immediately */
	if (slotEdit() == QDialog::Rejected)
	{
		/* Edit dialog was rejected -> delete function */
		deleteSelectedFunctions();
	}
	else
	{
		m_tree->sortItems(KColumnName, Qt::AscendingOrder);
		m_tree->scrollToItem(item);
	}
}
