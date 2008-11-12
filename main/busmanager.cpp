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
#include <QString>
#include <QIcon>

#include "busmanager.h"
#include "app.h"
#include "bus.h"

#define KColumnID   0
#define KColumnName 1

BusManager::BusManager(QWidget* parent) : QWidget(parent)
{
	Q_ASSERT(parentWidget() != NULL);

	setupUi(this);

	connect(m_list, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEdit()));
	connect(m_edit, SIGNAL(clicked()), this, SLOT(slotEdit()));
	connect(m_close, SIGNAL(clicked()), parentWidget(), SLOT(close()));

	fillTree();
}

BusManager::~BusManager()
{
}

void BusManager::slotEdit()
{
	QTreeWidgetItem* item = m_list->currentItem();

	if (item != NULL)
	{
		QString label;
		QString name;
		t_bus_id id;
		bool ok = false;

		id = item->text(KColumnID).toInt() - 1;
		label.sprintf("Bus #%d name:", id + 1);
		name = QInputDialog::getText(this, "Rename bus", label,
					     QLineEdit::Normal, Bus::name(id),
					     &ok);
		if (ok == true)
		{
			Bus::setName(id, name);
			item->setText(KColumnName, name);
		}
	}
}

void BusManager::fillTree()
{
	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		QString str;

		str.sprintf("%.2d", id + 1);
		item->setText(KColumnID, str);
		item->setText(KColumnName, Bus::name(id));
	}
}

