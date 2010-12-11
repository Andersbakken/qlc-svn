/*
  Q Light Controller
  fixtureselection.cpp

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
#include <QLabel>

#include "qlcfixturedef.h"

#include "fixtureselection.h"
#include "fixture.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName         0
#define KColumnManufacturer 1
#define KColumnModel        2
#define KColumnID           3

FixtureSelection::FixtureSelection(QWidget* parent, Doc* doc, bool multiple,
                                   QList <t_fixture_id> disabled)
        : QDialog(parent)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Multiple/single selection */
    if (multiple == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    /* Fill the tree */
    for (t_fixture_id fxi_id = 0; fxi_id < KFixtureArraySize; fxi_id++)
    {
        QTreeWidgetItem* item;
        Fixture* fixture;
        QString str;

        fixture = doc->fixture(fxi_id);
        if (fixture == NULL)
            continue;

        item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnName, fixture->name());
        item->setText(KColumnID, str.setNum(fxi_id));

        if (fixture->fixtureDef() == NULL)
        {
            item->setText(KColumnManufacturer, tr("Generic"));
            item->setText(KColumnModel, tr("Generic"));
        }
        else
        {
            item->setText(KColumnManufacturer,
                          fixture->fixtureDef()->manufacturer());
            item->setText(KColumnModel,
                          fixture->fixtureDef()->model());
        }

        if (disabled.contains(fxi_id) == true)
            item->setFlags(0); // Disables the item
    }

    if (m_tree->topLevelItemCount() == 0)
    {
        m_tree->setHeaderLabel(tr("No fixtures available"));
        m_tree->header()->hideSection(KColumnManufacturer);
        m_tree->header()->hideSection(KColumnModel);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, tr("Go to the Fixture Manager and add some fixtures first."));
        m_tree->setEnabled(false);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
    else
    {
        m_tree->sortItems(KColumnName, Qt::AscendingOrder);
        m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    }
}

FixtureSelection::~FixtureSelection()
{
}

void FixtureSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}

void FixtureSelection::accept()
{
    selection.clear();

    /* TODO: Check, whether some items are fixture items. If they are,
       don't put them into selection list. See above Qt::ItemIsEnabled. */
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        selection.append(it.next()->text(KColumnID).toInt());

    QDialog::accept();
}
