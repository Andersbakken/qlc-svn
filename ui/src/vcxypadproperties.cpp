/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (C) Stefan Krumm, Heikki Junnila

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
#include <QMessageBox>
#include <QHeaderView>
#include <QSettings>

#include "qlcchannel.h"
#include "vcxypadfixtureeditor.h"
#include "vcxypadproperties.h"
#include "fixtureselection.h"
#include "vcxypadfixture.h"
#include "vcxypad.h"
#include "apputil.h"

extern App* _app;

#define SETTINGS_GEOMETRY "vcxypad/geometry"

#define KColumnFixture   0
#define KColumnXAxis     1
#define KColumnYAxis     2

/****************************************************************************
 * Initialization
 ****************************************************************************/

VCXYPadProperties::VCXYPadProperties(QWidget* parent, VCXYPad* xypad)
    : QDialog(parent)
{
    Q_ASSERT(xypad != NULL);
    m_xypad = xypad;

    setupUi(this);

    m_nameEdit->setText(m_xypad->caption());
    slotSelectionChanged(NULL);
    fillTree();
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

VCXYPadProperties::~VCXYPadProperties()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/****************************************************************************
 * Tree
 ****************************************************************************/

void VCXYPadProperties::fillTree()
{
    m_tree->clear();

    QListIterator <VCXYPadFixture> it(m_xypad->fixtures());
    while (it.hasNext() == true)
        updateFixtureItem(new QTreeWidgetItem(m_tree), it.next());
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

void VCXYPadProperties::updateFixtureItem(QTreeWidgetItem* item,
                                          const VCXYPadFixture& fxi)
{
    Q_ASSERT(item != NULL);

    item->setText(KColumnFixture, fxi.name());
    item->setText(KColumnXAxis, fxi.xBrief());
    item->setText(KColumnYAxis, fxi.yBrief());
    item->setData(KColumnFixture, Qt::UserRole, QVariant(fxi));
}

QList <t_fixture_id> VCXYPadProperties::selectedFixtureIDs() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <t_fixture_id> list;

    /* Put all selected fixture IDs to a list and return it */
    while (it.hasNext() == true)
    {
        QVariant var(it.next()->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture fxi(var);
        list << fxi.fixture();
    }

    return list;
}

QList <VCXYPadFixture> VCXYPadProperties::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <VCXYPadFixture> list;

    /* Put all selected fixtures to a list and return it */
    while (it.hasNext() == true)
        list << it.next()->data(KColumnFixture, Qt::UserRole);

    return list;
}

QTreeWidgetItem* VCXYPadProperties::fixtureItem(const VCXYPadFixture& fxi)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture another(var);
        if (fxi.fixture() == another.fixture())
            return *it;
        else
            ++it;
    }

    return NULL;
}

void VCXYPadProperties::removeFixtureItem(t_fixture_id fxi_id)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture fxi(var);
        if (fxi.fixture() == fxi_id)
        {
            delete (*it);
            break;
        }

        ++it;
    }
}

void VCXYPadProperties::slotAddClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <t_fixture_id> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        QVariant var((*twit)->data(KColumnFixture, Qt::UserRole));
        VCXYPadFixture fxi(var);
        disabled << fxi.fixture();
        ++twit;
    }

    /* Disable all fixtures that don't have pan OR tilt channels */
    for (t_fixture_id fxi_id = 0; fxi_id < KFixtureArraySize; fxi_id++)
    {
        Fixture* fixture = _app->doc()->fixture(fxi_id);
        if (fixture == NULL)
            continue;

        // If a channel with pan group exists, don't disable this fixture
        if (fixture->channel("", Qt::CaseSensitive, QLCChannel::Pan)
                != QLCChannel::invalid())
        {
            continue;
        }

        // If a channel with tilt group exists, don't disable this fixture
        if (fixture->channel("", Qt::CaseSensitive, QLCChannel::Tilt)
                != QLCChannel::invalid())
        {
            continue;
        }

        // Disable all fixtures without pan or tilt channels
        disabled << fxi_id;

    }

    /* Get a list of new fixtures to add to the pad */
    QTreeWidgetItem* item = NULL;
    FixtureSelection fs(this, _app->doc(), true, disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <t_fixture_id> it(fs.selection);
        while (it.hasNext() == true)
        {
            VCXYPadFixture fxi;
            fxi.setFixture(it.next());
            item = new QTreeWidgetItem(m_tree);
            updateFixtureItem(item, fxi);
        }
    }

    if (item != NULL)
        m_tree->setCurrentItem(item);
}

void VCXYPadProperties::slotRemoveClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixtures?"),
                QMessageBox::Yes, QMessageBox::No);

    if (r == QMessageBox::Yes)
    {
        QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
        while (it.hasNext() == true)
            delete it.next();
    }
}

void VCXYPadProperties::slotEditClicked()
{
    /* Get a list of selected fixtures */
    QList <VCXYPadFixture> list(selectedFixtures());

    /* Start editor */
    VCXYPadFixtureEditor editor(this, list);
    if (editor.exec() == QDialog::Accepted)
    {
        QListIterator <VCXYPadFixture> it(editor.fixtures());
        while (it.hasNext() == true)
        {
            VCXYPadFixture fxi(it.next());
            QTreeWidgetItem* item = fixtureItem(fxi);

            updateFixtureItem(item, fxi);
        }
    }
}

void VCXYPadProperties::slotSelectionChanged(QTreeWidgetItem* item)
{
    if (item == NULL)
    {
        m_removeButton->setEnabled(false);
        m_editButton->setEnabled(false);
    }
    else
    {
        m_removeButton->setEnabled(true);
        m_editButton->setEnabled(true);
    }
}

void VCXYPadProperties::accept()
{
    m_xypad->clearFixtures();
    m_xypad->setCaption(m_nameEdit->text());

    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QVariant var((*it)->data(KColumnFixture, Qt::UserRole));
        m_xypad->appendFixture(var);
        ++it;
    }

    QDialog::accept();
}
