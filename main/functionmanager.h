/*
  Q Light Controller
  functionmanager.h

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

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <QWidget>
#include <QList>

#include "common/qlctypes.h"
#include "function.h"

class QTreeWidgetItem;
class QActionGroup;
class QTreeWidget;
class QSplitter;
class QToolBar;
class QAction;
class QMenu;

class Fixture;

class FunctionManager : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	FunctionManager(QWidget* parent);
	~FunctionManager();

	void update();

private:
	Q_DISABLE_COPY(FunctionManager)

	/*********************************************************************
	 * Doc signal handlers
	 *********************************************************************/
public slots:
	/** Function was added to Doc */
	void slotFunctionAdded(t_function_id fid);

	/** Function was removed from Doc */
	void slotFunctionRemoved(t_function_id fid);

	/** Function was changed */
	void slotFunctionChanged(t_function_id fid);

	/*********************************************************************
	 * Function tree
	 *********************************************************************/
protected:
	/** Init function tree view */
	void initFunctionTree();

	/** Update all functions to function tree */
	void updateFunctionTree();

	/** Update the item's contents from the given function */
	void updateFunctionItem(QTreeWidgetItem* item, Function* function);

	/** Delete all currently selected functions */
	void deleteSelectedFunctions();

protected slots:
	/** Function selection was changed */
	void slotFunctionTreeSelectionChanged();

	/** Right mouse button was clicked on function tree */
	void slotFunctionTreeContextMenuRequested(const QPoint& pos);

protected:
	QTreeWidget* m_functionTree;

	// Get an item from the given listview by the given id
	QTreeWidgetItem* getItem(t_function_id id, QTreeWidget* listView);

protected slots:
	/** Set the selected bus to all selected functions */
	void slotBusTriggered(QAction* action);

	/*********************************************************************
	 * Menus, toolbar & actions
	 *********************************************************************/
protected:
	void initActions();
	void initMenu();
	void initToolbar();

protected slots:
	/** When bus name changes, its action must also be updated */
	void slotBusNameChanged(t_bus_id id, const QString& name);

	void slotAddScene();
	void slotAddChaser();
	void slotAddCollection();
	void slotAddEFX();

	int slotEdit();
	void slotClone();
	void slotDelete();
	void slotSelectAll();

protected:
	/** Update action enabled status */
	void updateActionStatus();
	void updateBusActions();

protected:
	QMenu* m_manageMenu;
	QMenu* m_editMenu;
	QMenu* m_busMenu;

	QToolBar* m_toolbar;

	QAction* m_addSceneAction;
	QAction* m_addChaserAction;
	QAction* m_addCollectionAction;
	QAction* m_addEFXAction;
	QAction* m_closeAction;

	QAction* m_cloneAction;
	QAction* m_editAction;
	QAction* m_deleteAction;
	QAction* m_selectAllAction;

	QActionGroup* m_busGroup;
	QList <QAction*> m_busActions;

	/*********************************************************************
	 * Helpers
	 *********************************************************************/
protected:
	/** Create a copy of the given function */
	Function* copyFunction(t_function_id fid);

	/** Add a new function of the given type */
	void addFunction(Function::Type type);

protected:
	/** Don't listen to Doc::functionAdded signal when this is true */
	bool m_blockAddFunctionSignal;

	/** Don't listen to Doc::functionRemoved signal when this is true */
	bool m_blockRemoveFunctionSignal;
};

#endif

