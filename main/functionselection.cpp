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
#include <QDebug>

#include "functionselection.h"
#include "function.h"
#include "fixture.h"
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
	setupUi(this);

	/* Disable function */
	m_disable = disableFunction;

	/* Filter */
	m_filter = filter;

	if (m_filter & Function::Scene)
		m_sceneCheck->setChecked(true);
	connect(m_sceneCheck, SIGNAL(toggled(bool)),
		this, SLOT(slotSceneChecked(bool)));

	if (m_filter & Function::Chaser)
		m_chaserCheck->setChecked(true);
	connect(m_chaserCheck, SIGNAL(toggled(bool)),
		this, SLOT(slotChaserChecked(bool)));

	if (m_filter & Function::EFX)
		m_efxCheck->setChecked(true);
	connect(m_efxCheck, SIGNAL(toggled(bool)),
		this, SLOT(slotEFXChecked(bool)));

	if (m_filter & Function::Collection)
		m_collectionCheck->setChecked(true);
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
		m_tree->setSelectionMode(QAbstractItemView::MultiSelection);
	else
		m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

	refillTree();
}

FunctionSelection::~FunctionSelection()
{
}

/*****************************************************************************
 * Internal
 *****************************************************************************/

void FunctionSelection::refillTree()
{
	m_tree->clear();

	/* Fill the tree */
	for (t_function_id fid = 0; fid < KFunctionArraySize; fid++)
	{
		QTreeWidgetItem* item;
		Function* function;
		QString str;
		
		function = _app->doc()->function(fid);
		if (function == NULL)
			continue;

		if (m_filter == Function::Undefined ||
		    m_filter & function->type())
		{
			item = new QTreeWidgetItem(m_tree);
			item->setText(KColumnName, function->name());
			item->setText(KColumnType, function->typeString());
			item->setText(KColumnID, str.setNum(fid));

			if (m_disable == fid)
				item->setFlags(0); // Disables the item
		}
	}
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
	refillTree();
}

void FunctionSelection::slotChaserChecked(bool state)
{
	if (state == true)
		m_filter = (m_filter | Function::Chaser);
	else
		m_filter = (m_filter & ~Function::Chaser);
	refillTree();
}

void FunctionSelection::slotEFXChecked(bool state)
{
	if (state == true)
		m_filter = (m_filter | Function::EFX);
	else
		m_filter = (m_filter & ~Function::EFX);
	refillTree();
}

void FunctionSelection::slotCollectionChecked(bool state)
{
	if (state == true)
		m_filter = (m_filter | Function::Collection);
	else
		m_filter = (m_filter & ~Function::Collection);
	refillTree();
}

void FunctionSelection::accept()
{
	selection.clear();

	/* TODO: Check, whether some items are fixture items. If they are,
	   don't put them into selection list. See above Qt::ItemIsEnabled. */
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
		selection.append(it.next()->text(KColumnID).toInt());

	QDialog::accept();
}
