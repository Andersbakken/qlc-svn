/*
  Q Light Controller
  vccuelistproperties.cpp

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

#include "vccuelistproperties.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vccuelist.h"
#include "keybind.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCCueListProperties::VCCueListProperties(QWidget* parent, VCCueList* cueList)
	: QDialog(parent)
{
	Q_ASSERT(cueList != NULL);
	m_cueList = cueList;

	setupUi(this);

	/* Name */
	m_nameEdit->setText(cueList->caption());
	m_nameEdit->setSelection(0, cueList->caption().length());

	/* Connections */
	connect(m_addButton, SIGNAL(clicked()),
		this, SLOT(slotAddClicked()));
	connect(m_removeButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveClicked()));
	connect(m_raiseButton, SIGNAL(clicked()),
		this, SLOT(slotRaiseClicked()));
	connect(m_lowerButton, SIGNAL(clicked()),
		this, SLOT(slotLowerClicked()));
	connect(m_attachButton, SIGNAL(clicked()),
		this, SLOT(slotAttachClicked()));
	connect(m_detachButton, SIGNAL(clicked()),
		this, SLOT(slotDetachClicked()));

	/* Put all cues into the tree widget */
	QTreeWidgetItemIterator it(m_cueList->m_list);
	while (*it != NULL)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(KVCCueListColumnNumber, QString("%1")
			      .arg(m_list->indexOfTopLevelItem(item) + 1));
		item->setText(KVCCueListColumnName,
			      (*it)->text(KVCCueListColumnName));
		item->setText(KVCCueListColumnID,
			      (*it)->text(KVCCueListColumnID));
		++it;
	}

	m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

	/* Key bind */
	if (cueList->keyBind() != NULL)
	{
		m_keyBind = new KeyBind(cueList->keyBind());
		m_keyEdit->setText(m_keyBind->keyString());
	}
	else
	{
		m_keyBind = NULL;
		m_keyEdit->setText(tr("No key binding"));
	}
}

VCCueListProperties::~VCCueListProperties()
{
	if (m_keyBind != NULL)
		delete m_keyBind;
}

void VCCueListProperties::slotAddClicked()
{
	QTreeWidgetItem* item;
	Function* function;

	/* Select only scenes */
	FunctionSelection fs(this, true, KNoID, Function::Scene, true);
	if (fs.exec() == QDialog::Accepted)
	{
		/* Append selected scenes. TODO: Insert after current item. */
		QListIterator <t_function_id> it(fs.selection);
		while (it.hasNext() == true)
		{
			function = _app->doc()->function(it.next());
			Q_ASSERT(function != NULL);

			item = new QTreeWidgetItem(m_list);
			item->setText(KVCCueListColumnNumber, QString("%1")
			      .arg(m_list->indexOfTopLevelItem(item) + 1));
			item->setText(KVCCueListColumnName, function->name());
			item->setText(KVCCueListColumnID,
				      QString("%1").arg(function->id()));
		}
	}
}

void VCCueListProperties::slotRemoveClicked()
{
	QTreeWidgetItem* item;
	QTreeWidgetItem* next;

	item = m_list->currentItem();
	if (item != NULL)
	{
		next = m_list->itemBelow(item);
		if (next == NULL)
			next = m_list->itemAbove(item);
		delete item;

		if (next != NULL)
		{
			m_list->setCurrentItem(next);

			for (int i = m_list->indexOfTopLevelItem(next);
			     i < m_list->topLevelItemCount(); i++)
			{
				m_list->topLevelItem(i)->setText(
					KVCCueListColumnNumber,
					QString("%1").arg(i + 1));
			}
		}
	}
}

void VCCueListProperties::slotRaiseClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_list->currentItem();
	if (item == NULL)
		return;

	index = m_list->indexOfTopLevelItem(item);
	if (index == 0)
		return;

	m_list->takeTopLevelItem(index);
	m_list->insertTopLevelItem(index - 1, item);
	m_list->setCurrentItem(item);

	item->setText(KVCCueListColumnNumber, QString("%1").arg(index - 1 + 1));
	m_list->itemBelow(item)->setText(KVCCueListColumnNumber,
					 QString("%1").arg(index + 1));
}

void VCCueListProperties::slotLowerClicked()
{
	QTreeWidgetItem* item;
	int index;

	item = m_list->currentItem();
	if (item == NULL)
		return;

	index = m_list->indexOfTopLevelItem(item);
	if (index == m_list->topLevelItemCount() - 1)
		return;

	m_list->takeTopLevelItem(index);
	m_list->insertTopLevelItem(index + 1, item);
	m_list->setCurrentItem(item);

	item->setText(KVCCueListColumnNumber, QString("%1").arg(index + 1 + 1));
	m_list->itemAbove(item)->setText(KVCCueListColumnNumber,
					 QString("%1").arg(index + 1));
}

void VCCueListProperties::slotAttachClicked()
{
	AssignHotKey ahk(this);
	if (ahk.exec() == QDialog::Accepted)
	{
		delete m_keyBind;
		m_keyBind = new KeyBind(ahk.keyBind());
		m_keyEdit->setText(m_keyBind->keyString());
	}
}

void VCCueListProperties::slotDetachClicked()
{
	delete m_keyBind;
	m_keyBind = NULL;
	m_keyEdit->setText(tr("No key binding"));
}

void VCCueListProperties::accept()
{
	/* Replace existing list of cues */
	m_cueList->clear();

	QTreeWidgetItemIterator it(m_list);
	while (*it != NULL)
	{
		m_cueList->append((*it)->text(KVCCueListColumnID).toInt());
		++it;
	}

	/* Key binding */
	m_cueList->setKeyBind(m_keyBind);

	QDialog::accept();
}
