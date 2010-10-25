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

#include "qlctypes.h"
#include "function.h"
#include "app.h"

class QTreeWidgetItem;
class QActionGroup;
class QTreeWidget;
class QSplitter;
class QToolBar;
class QAction;
class Fixture;
class QMenu;

class FunctionManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the FunctionManager singleton instance. Can be NULL. */
    static FunctionManager* instance() {
        return s_instance;
    }

    /** Create an instance with parent. Fails if s_instance is not NULL. */
    static void create(QWidget* parent);

    /** Normal public destructor */
    ~FunctionManager();

protected:
    /** Protected constructor to prevent multiple instances. */
    FunctionManager(QWidget* parent, Qt::WindowFlags flags = 0);

protected slots:
    void slotModeChanged(Doc::Mode mode);
    void slotDocumentChanged(Doc* doc);

protected:
    static FunctionManager* s_instance;

    /*********************************************************************
     * Function tree
     *********************************************************************/
public:
    /** Update all functions to function tree */
    void updateTree();

protected:
    /** Init function tree view */
    void initTree();

    /** Update the item's contents from the given function */
    void updateFunctionItem(QTreeWidgetItem* item, Function* function);

    /** Get an icon that represents the given function's type */
    QIcon functionIcon(const Function* function) const;

    /** Delete all currently selected functions */
    void deleteSelectedFunctions();

protected slots:
    /** Function selection was changed */
    void slotTreeSelectionChanged();

    /** Right mouse button was clicked on function tree */
    void slotTreeContextMenuRequested(const QPoint& pos);

protected:
    QTreeWidget* m_tree;

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
    void slotBusNameChanged(quint32 id, const QString& name);

    void slotAddScene();
    void slotAddChaser();
    void slotAddCollection();
    void slotAddEFX();
    void slotWizard();

    int slotEdit();
    void slotClone();
    void slotDelete();
    void slotSelectAll();

protected:
    void updateActionStatus();

protected:
    QMenu* m_addMenu;
    QMenu* m_editMenu;
    QMenu* m_busMenu;

    QToolBar* m_toolbar;

    QAction* m_addSceneAction;
    QAction* m_addChaserAction;
    QAction* m_addCollectionAction;
    QAction* m_addEFXAction;
    QAction* m_wizardAction;

    QAction* m_cloneAction;
    QAction* m_editAction;
    QAction* m_deleteAction;
    QAction* m_selectAllAction;

    QActionGroup* m_busGroup;
    QToolButton* m_busButton;

    /*********************************************************************
     * Helpers
     *********************************************************************/
protected:
    /** Create a copy of the given function */
    void copyFunction(t_function_id fid);

    /** Add a new function */
    void addFunction(Function* function);

    /** Open an editor for the given function */
    int editFunction(Function* function);

protected:
    /** Don't listen to Doc::functionAdded signal when this is true */
    bool m_blockAddFunctionSignal;

    /** Don't listen to Doc::functionRemoved signal when this is true */
    bool m_blockRemoveFunctionSignal;
};

#endif

