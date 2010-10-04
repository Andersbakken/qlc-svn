/*
  Q Light Controller
  functionselection.h

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

#ifndef FUNCTIONSELECTION_H
#define FUNCTIONSELECTION_H

#include <QDialog>
#include <QList>

#include "qlctypes.h"
#include "ui_functionselection.h"
#include "function.h"

class QTreeWidgetItem;
class QToolBar;
class QAction;
class QWidget;

class Fixture;
class Doc;

class FunctionSelection : public QDialog, public Ui_FunctionSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionSelection)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Constructor
     *
     * @param parent The parent widget
     * @param multiple Set true to enable multiple selection
     * @param disableID A function ID to disable (when adding steps to
     *                  a chaser, disable the chaser itself)
     * @param filter Show only functions of the given types OR'd. All
     *               functions are shown by default.
     * @param constFilter If true, don't allow user filter selection
     */
    FunctionSelection(QWidget* parent,
                      bool multiple,
                      t_function_id disableFunction = Function::invalidId(),
                      int filter = Function::Scene | Function::Chaser |
                                   Function::EFX | Function::Collection,
                      bool constFilter = false);

    /**
     * Destructor
     */
    ~FunctionSelection();

    /*********************************************************************
     * Disabled functions
     *********************************************************************/
public:
    void setDisabledFunctions(const QList <t_function_id>& ids)
    {
        m_disabledFunctions = ids;
    }

    QList <t_function_id> disabledFunctions() const
    {
        return m_disabledFunctions;
    }

protected:
    QList <t_function_id> m_disabledFunctions;

    /*********************************************************************
     * Selection
     *********************************************************************/
public:
    /** Get a list of selected function IDs */
    const QList <t_function_id> selection() const {
        return m_selection;
    }

protected:
    /** The list of selected function IDs */
    QList <t_function_id> m_selection;

    /*********************************************************************
     * Toolbar
     *********************************************************************/
protected slots:
    void slotNewScene();
    void slotNewChaser();
    void slotNewEFX();
    void slotNewCollection();

protected:
    /** Create toolbar */
    void initToolBar();

protected:
    QToolBar* m_toolbar;
    QAction* m_addSceneAction;
    QAction* m_addChaserAction;
    QAction* m_addEFXAction;
    QAction* m_addCollectionAction;

    /*********************************************************************
     * Internal
     *********************************************************************/
protected:
    /** Add the given function to the tree and select it */
    void addFunction(Function* function);

    /** Display an error message if function creation failed */
    void addFunctionErrorMessage();

    /** Update the contents of the given function to the tree item */
    void updateFunctionItem(QTreeWidgetItem* item, Function* function);

    /** Clear & (re)fill the tree */
    void refillTree();

    /**
     * Find a top-level item that matches the given fixture instance or
     * create one if it doesn't exist.
     *
     * @param fxi_id The fixture ID to search for
     * @param doc A QLC Doc* pointer that contains all fixture instances
     */
    QTreeWidgetItem* fixtureItem(t_fixture_id fxi_id, Doc* doc);

    /**
     * Edit the given function with an editor dialog
     */
    int editFunction(Function* function);

protected slots:
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QTreeWidgetItem* item);

    void slotCollectionChecked(bool state);
    void slotEFXChecked(bool state);
    void slotChaserChecked(bool state);
    void slotSceneChecked(bool state);

    /**
     * OK button click
     */
    void accept();

protected:
    int m_filter;
    t_function_id m_disable;
};

#endif
