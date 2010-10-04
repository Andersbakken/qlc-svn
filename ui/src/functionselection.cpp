/*
  Q Light Controller
  functionselection.cpp

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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QDebug>

#include "functionselection.h"
#include "collectioneditor.h"
#include "chasereditor.h"
#include "sceneeditor.h"
#include "collection.h"
#include "efxeditor.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "app.h"
#include "doc.h"

#define KColumnName 0
#define KColumnType 1
#define KColumnID   2

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent,
                                     bool multiple,
                                     t_function_id disableFunction,
                                     int filter,
                                     bool constFilter)
        : QDialog(parent)
{
    m_toolbar = NULL;
    m_addSceneAction = NULL;
    m_addChaserAction = NULL;
    m_addEFXAction = NULL;
    m_addCollectionAction = NULL;

    setupUi(this);

    /* Create toolbar */
    initToolBar();

    /* Disable function */
    m_disabledFunctions << disableFunction;

    /* Filter */
    m_filter = filter;

    m_sceneCheck->setChecked(m_filter & Function::Scene);
    m_addSceneAction->setEnabled(m_filter & Function::Scene);
    connect(m_sceneCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSceneChecked(bool)));

    m_chaserCheck->setChecked(m_filter & Function::Chaser);
    m_addChaserAction->setEnabled(m_filter & Function::Chaser);
    connect(m_chaserCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotChaserChecked(bool)));

    m_efxCheck->setChecked(m_filter & Function::EFX);
    m_addEFXAction->setEnabled(m_filter & Function::EFX);
    connect(m_efxCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotEFXChecked(bool)));

    m_collectionCheck->setChecked(m_filter & Function::Collection);
    m_addCollectionAction->setEnabled(m_filter & Function::Collection);
    connect(m_collectionCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotCollectionChecked(bool)));

    if (constFilter == true)
    {
        m_sceneCheck->setEnabled(false);
        m_chaserCheck->setEnabled(false);
        m_efxCheck->setEnabled(false);
        m_collectionCheck->setEnabled(false);
    }

    /* Multiple/single selection */
    if (multiple == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    refillTree();
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
}

FunctionSelection::~FunctionSelection()
{
}

/*****************************************************************************
 * Toolbar
 *****************************************************************************/

void FunctionSelection::initToolBar()
{
    m_toolbar = new QToolBar(this);
    layout()->setMenuBar(m_toolbar);

    m_addSceneAction = m_toolbar->addAction(QIcon(":/scene.png"),
                                            tr("New Scene"), this, SLOT(slotNewScene()));
    m_addChaserAction = m_toolbar->addAction(QIcon(":/chaser.png"),
                        tr("New Chaser"), this, SLOT(slotNewChaser()));
    m_addEFXAction = m_toolbar->addAction(QIcon(":/efx.png"),
                                          tr("New EFX"), this, SLOT(slotNewEFX()));
    m_addCollectionAction = m_toolbar->addAction(QIcon(":/collection.png"),
                            tr("New Collection"), this, SLOT(slotNewCollection()));
}

void FunctionSelection::slotNewScene()
{
    Function* function = new Scene(_app->doc());
    function->setName(tr("New Scene"));

    if (_app->doc()->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewChaser()
{
    Function* function = new Chaser(_app->doc());
    function->setName(tr("New Chaser"));

    if (_app->doc()->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewEFX()
{
    Function* function = new EFX(_app->doc());
    function->setName(tr("New EFX"));

    if (_app->doc()->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

void FunctionSelection::slotNewCollection()
{
    Function* function = new Collection(_app->doc());
    function->setName(tr("New Collection"));

    if (_app->doc()->addFunction(function) == true)
        addFunction(function);
    else
        addFunctionErrorMessage();
}

/*****************************************************************************
 * Tree
 *****************************************************************************/

void FunctionSelection::addFunction(Function* function)
{
    QTreeWidgetItem* item;

    Q_ASSERT(function != NULL);

    /* Create a new item for the function */
    item = new QTreeWidgetItem(m_tree);
    updateFunctionItem(item, function);

    /* Append the new function to current selection */
    item->setSelected(true);

    if (editFunction(function) == QDialog::Rejected)
    {
        _app->doc()->deleteFunction(function->id());
        delete item;
    }
    else
    {
        updateFunctionItem(item, function);
        m_tree->sortItems(KColumnName, Qt::AscendingOrder);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionSelection::addFunctionErrorMessage()
{
    if (_app->doc()->functions() >= KFunctionArraySize)
    {
        QMessageBox::critical(this, tr("Too many functions"),
                              tr("You can't create more than %1 functions.")
                              .arg(KFunctionArraySize));
        return;
    }
    else
    {
        QMessageBox::critical(this, tr("Function creation failed"),
                              tr("Unable to create new function."));
        return;
    }
}

void FunctionSelection::updateFunctionItem(QTreeWidgetItem* item,
        Function* function)
{
    QString str;

    item->setText(KColumnName, function->name());
    item->setText(KColumnType, function->typeString());
    item->setText(KColumnID, str.setNum(function->id()));
}

void FunctionSelection::refillTree()
{
    m_tree->clear();

    /* Fill the tree */
    for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
    {
        QTreeWidgetItem* item;
        Function* function;

        function = _app->doc()->function(fid);
        if (function == NULL)
            continue;

        if (m_filter & function->type())
        {
            item = new QTreeWidgetItem(m_tree);
            updateFunctionItem(item, function);

            if (disabledFunctions().contains(function->id()))
                item->setFlags(0); // Disables the item
        }
    }
}

void FunctionSelection::slotItemSelectionChanged()
{
    QList <t_function_id> removeList(m_selection);

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        t_function_id id = it.next()->text(KColumnID).toInt();
        if (m_selection.contains(id) == false)
            m_selection.append(id);

        removeList.removeAll(id);
    }

    while (removeList.isEmpty() == false)
        m_selection.removeAll(removeList.takeFirst());
}

void FunctionSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}

void FunctionSelection::slotSceneChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Scene);
    else
        m_filter = (m_filter & ~Function::Scene);
    m_addSceneAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotChaserChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Chaser);
    else
        m_filter = (m_filter & ~Function::Chaser);
    m_addChaserAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotEFXChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::EFX);
    else
        m_filter = (m_filter & ~Function::EFX);
    m_addEFXAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::slotCollectionChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Collection);
    else
        m_filter = (m_filter & ~Function::Collection);
    m_addCollectionAction->setEnabled(state);
    refillTree();
}

void FunctionSelection::accept()
{
    QDialog::accept();
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

int FunctionSelection::editFunction(Function* function)
{
    int result = QDialog::Rejected;

    Q_ASSERT(function != NULL);

    if (function->type() == Function::Scene)
    {
        SceneEditor editor(this, qobject_cast<Scene*> (function));
        result = editor.exec();
    }
    else if (function->type() == Function::Chaser)
    {
        ChaserEditor editor(this, qobject_cast<Chaser*> (function));
        result = editor.exec();
    }
    else if (function->type() == Function::Collection)
    {
        CollectionEditor editor(this,
                                qobject_cast<Collection*> (function));
        result = editor.exec();
    }
    else if (function->type() == Function::EFX)
    {
        EFXEditor editor(this, qobject_cast<EFX*> (function));
        result = editor.exec();
    }
    else
    {
        result = QDialog::Rejected;
    }

    return result;
}
