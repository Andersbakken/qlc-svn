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

#include "vcxypadfixtureeditor.h"
#include "vcxypadproperties.h"
#include "fixtureselection.h"
#include "vcxypadfixture.h"
#include "vcxypad.h"

extern App* _app;

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
	fillTree();
}

VCXYPadProperties::~VCXYPadProperties()
{
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
	/* TODO: Disable all fixtures that don't have pan&tilt chans */
	QList <t_fixture_id> disabled;
	QTreeWidgetItemIterator twit(m_tree);
	while (*twit != NULL)
	{
		QVariant var((*twit)->data(KColumnFixture, Qt::UserRole));
		VCXYPadFixture fxi(var);
		disabled << fxi.fixture();
	}

	/* Get a list of new fixtures to add to the pad */
	FixtureSelection fs(this, _app->doc(), true, disabled);
	if (fs.exec() == QDialog::Accepted)
	{
		QListIterator <t_fixture_id> it(fs.selection);
		while (it.hasNext() == true)
		{
			VCXYPadFixture fxi;
			fxi.setFixture(it.next());
			updateFixtureItem(new QTreeWidgetItem(m_tree), fxi);
		}
	}
}

void VCXYPadProperties::slotRemoveClicked()
{
	int r = QMessageBox::question(
		this, tr("Remove fixtures"),
		tr("Do you want to remove the selected fixtures?"),
		QMessageBox::Yes, QMessageBox::No);

	if (r == QMessageBox::Yes)
	{
		QListIterator <t_fixture_id> it(selectedFixtureIDs());
		while (it.hasNext() == true)
		{
			t_fixture_id fxi_id = it.next();
			removeFixtureItem(fxi_id);
		}
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
