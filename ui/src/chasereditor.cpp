/*
  Q Light Controller
  chasereditor.cpp

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
#include <QRadioButton>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>

#include "qlcfixturedef.h"

#include "functionselection.h"
#include "chasereditor.h"
#include "apputil.h"
#include "fixture.h"
#include "chaser.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define SETTINGS_GEOMETRY "chasereditor/geometry"

#define KColumnNumber     0
#define KColumnFunction   1
#define KColumnFunctionID 2

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser) : QDialog(parent)
{
    Q_ASSERT(chaser != NULL);

    setupUi(this);

    /* Resize columns to fit contents */
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    /* Connect UI controls */
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
    connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
    connect(m_raise, SIGNAL(clicked()), this, SLOT(slotRaiseClicked()));
    connect(m_lower, SIGNAL(clicked()), this, SLOT(slotLowerClicked()));

    /* Create a copy of the original chaser so that we can freely modify
       it and keep a pointer to the original so that we can move the
       contents from the copied chaser to the original when OK is clicked */
    m_chaser = new Chaser(_app->doc());
    m_chaser->copyFrom(chaser);
    Q_ASSERT(m_chaser != NULL);
    m_original = chaser;

    /* Name edit */
    m_nameEdit->setText(m_chaser->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    setWindowTitle(tr("Chaser - %1").arg(m_chaser->name()));

    /* Bus */
    connect(m_busCombo, SIGNAL(activated(int)),
            this, SLOT(slotBusComboActivated(int)));
    fillBusCombo();

    /* Running order */
    switch (m_chaser->runOrder())
    {
    default:
    case Chaser::Loop:
        m_loop->setChecked(true);
        break;
    case Chaser::PingPong:
        m_pingPong->setChecked(true);
        break;
    case Chaser::SingleShot:
        m_singleShot->setChecked(true);
        break;
    }

    /* Running direction */
    switch (m_chaser->direction())
    {
    default:
    case Chaser::Forward:
        m_forward->setChecked(true);
        break;
    case Chaser::Backward:
        m_backward->setChecked(true);
        break;
    }

    /* Chaser steps */
    updateStepList(0);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

ChaserEditor::~ChaserEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    delete m_chaser;
}

void ChaserEditor::fillBusCombo()
{
    m_busCombo->clear();
    m_busCombo->addItems(Bus::instance()->idNames());
    m_busCombo->setCurrentIndex(m_chaser->busID());
}

void ChaserEditor::updateStepList(int selectIndex)
{
    m_tree->clear();

    QListIterator <t_function_id> it(m_chaser->steps());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;
        Function* function;
        t_function_id fid;
        QString str;

        fid = it.next();
        function = _app->doc()->function(fid);
        Q_ASSERT(function != NULL);

        item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnNumber, "###");
        item->setText(KColumnFunction, function->name());
        item->setText(KColumnFunctionID, str.setNum(fid));
    }

    /* Select the specified item */
    m_tree->setCurrentItem(m_tree->topLevelItem(selectIndex));

    /* Update the order number column */
    updateOrderNumbers();
}

void ChaserEditor::updateOrderNumbers()
{
    int i = 1;
    QString num;

    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        num.sprintf("%.03d", i++);
        (*it)->setText(KColumnNumber, num);
        ++it;
    }
}

void ChaserEditor::slotNameEdited(const QString& text)
{
    setWindowTitle(QString(tr("Chaser editor - %1")).arg(text));
}

void ChaserEditor::slotBusComboActivated(int index)
{
    Q_ASSERT(m_chaser != NULL);
    m_chaser->setBus(index);
}

void ChaserEditor::slotAddClicked()
{
    FunctionSelection fs(this, true, m_original->id(), Function::Scene, true);
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <t_function_id> it(fs.selection());
        while (it.hasNext() == true)
            m_chaser->addStep(it.next());

        // Update all steps in the list
        updateStepList();
    }
}

void ChaserEditor::slotRemoveClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        int index = item->text(KColumnNumber).toInt() - 1;
        m_chaser->removeStep(index);
        updateStepList(index - 1);
    }
}

void ChaserEditor::slotRaiseClicked()
{
    QTreeWidgetItem* item;
    int index;

    item = m_tree->currentItem();
    if (item != NULL)
    {
        index = m_tree->indexOfTopLevelItem(item);

        /* Raise the step */
        m_chaser->raiseStep(index);

        /* Update step list and select the same item */
        updateStepList(index - 1);
    }
}

void ChaserEditor::slotLowerClicked()
{
    QTreeWidgetItem* item;
    int index;

    item = m_tree->currentItem();
    if (item != NULL)
    {
        index = m_tree->indexOfTopLevelItem(item);

        /* Raise the step */
        m_chaser->lowerStep(index);

        /* Update step list and select the same item */
        updateStepList(index + 1);
    }
}

void ChaserEditor::accept()
{
    /* Name */
    m_chaser->setName(m_nameEdit->text());

    /* Run Order */
    if (m_singleShot->isChecked() == true)
        m_chaser->setRunOrder(Chaser::SingleShot);
    else if (m_pingPong->isChecked() == true)
        m_chaser->setRunOrder(Chaser::PingPong);
    else
        m_chaser->setRunOrder(Chaser::Loop);

    /* Direction */
    if (m_backward->isChecked() == true)
        m_chaser->setDirection(Chaser::Backward);
    else
        m_chaser->setDirection(Chaser::Forward);

    /* Copy the temp chaser's contents to the original */
    m_original->copyFrom(m_chaser);

    /* Mark doc as modified, close and accept */
    _app->doc()->setModified();
    QDialog::accept();
}
