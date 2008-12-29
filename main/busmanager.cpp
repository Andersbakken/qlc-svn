/*
  Q Light Controller
  busmanager.cpp

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
#include <QInputDialog>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>
#include <QToolBar>
#include <QAction>
#include <QString>
#include <QIcon>

#include "busmanager.h"
#include "app.h"
#include "bus.h"

#define KColumnID   0
#define KColumnName 1

BusManager::BusManager(QWidget* parent) : QWidget(parent)
{
	Q_ASSERT(parent != NULL);

	setupUi();
	fillTree();
}

BusManager::~BusManager()
{
}

void BusManager::setupUi()
{
	QStringList columns;

	/* Create a new layout for this widget */
	new QVBoxLayout(this);

	/* Toolbar */
	m_toolbar = new QToolBar(tr("Bus Manager"), this);
	m_toolbar->addAction(QIcon(":/edit.png"), tr("Edit bus name"),
			     this, SLOT(slotEditClicked()));
	layout()->addWidget(m_toolbar);

	/* Tree */
	m_tree = new QTreeWidget(this);
	layout()->addWidget(m_tree);
	m_tree->setRootIsDecorated(false);
	m_tree->setItemsExpandable(false);
	m_tree->setSortingEnabled(false);
	m_tree->setAllColumnsShowFocus(true);
	m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

	columns << tr("Bus ID") << tr("Name");
	m_tree->setHeaderLabels(columns);
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void BusManager::slotEditClicked()
{
	QTreeWidgetItem* item = m_tree->currentItem();

	if (item != NULL)
	{
		QString label;
		QString name;
		t_bus_id id;
		bool ok = false;

		id = item->text(KColumnID).toInt() - 1;
		label = tr("Bus #%1 name:").arg(id + 1);
		name = QInputDialog::getText(this, tr("Rename bus"), label,
					     QLineEdit::Normal, Bus::name(id),
					     &ok);
		if (ok == true)
		{
			Bus::setName(id, name);
			item->setText(KColumnName, name);
		}
	}
}

/****************************************************************************
 * Tree widget
 ****************************************************************************/

void BusManager::fillTree()
{
	QTreeWidgetItem* item;

	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnID, QString("%1").arg(id + 1));
		item->setText(KColumnName, Bus::name(id));
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}

	connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void BusManager::slotItemChanged(QTreeWidgetItem* item, int column)
{
	int index;

	Q_ASSERT(item != NULL);

	index = m_tree->indexOfTopLevelItem(item);
	if (column == KColumnID) /* Reject ID column edits */
		item->setText(KColumnID, QString("%1").arg(index + 1));
	else /* Change bus name */
		Bus::setName(index, item->text(KColumnName));
}
