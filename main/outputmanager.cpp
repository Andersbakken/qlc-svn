/*
  Q Light Controller
  outputmanager.cpp
  
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
#include <QStringList>
#include <QHeaderView>
#include <QToolBar>
#include <QAction>
#include <QMenu>

#include "common/qlcoutplugin.h"
#include "outputpatcheditor.h"
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "app.h"

#define KColumnUniverse   0
#define KColumnPlugin     1
#define KColumnOutputName 2
#define KColumnOutput     3

extern App* _app;

OutputManager::OutputManager(QWidget* parent) : QWidget(parent)
{
	init();
	update();
}

OutputManager::~OutputManager()
{
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void OutputManager::update()
{
	m_tree->clear();
	for (int uni = 0; uni < KUniverseCount; uni++)
	{
		OutputPatch* op = _app->outputMap()->patch(uni);
		updateItem(new QTreeWidgetItem(m_tree), op, uni);
	}
}

void OutputManager::init()
{
	QStringList columns;

	/* Create a new layout for this widget */
	new QVBoxLayout(this);

	/* Toolbar */
	m_toolbar = new QToolBar(tr("Output Manager"), this);
	m_toolbar->addAction(QIcon(":/edit.png"), tr("Edit Mapping"),
			     this, SLOT(slotEditClicked()));
	layout()->addWidget(m_toolbar);

	/* Tree */
	m_tree = new QTreeWidget(this);
	layout()->addWidget(m_tree);
	m_tree->setRootIsDecorated(false);
	m_tree->setItemsExpandable(false);
	m_tree->setSortingEnabled(false);
	m_tree->setAllColumnsShowFocus(true);
	columns << tr("Universe")
		<< tr("Plugin")
		<< tr("Output");
	m_tree->setHeaderLabels(columns);

	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));
}

void OutputManager::updateItem(QTreeWidgetItem* item, OutputPatch* op,
			       int universe)
{
	Q_ASSERT(item != NULL);
	Q_ASSERT(op != NULL);

	item->setText(KColumnUniverse, QString("%1").arg(universe + 1));
	item->setText(KColumnPlugin, op->pluginName());
	item->setText(KColumnOutputName, op->outputName());
	item->setText(KColumnOutput, QString("%1").arg(op->output() + 1));
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void OutputManager::slotEditClicked()
{
	QTreeWidgetItem* item;
	OutputPatch* patch;
	int universe;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	patch = _app->outputMap()->patch(universe);
	Q_ASSERT(patch != NULL);

	OutputPatchEditor ope(this, universe, patch);
	if (ope.exec() == QDialog::Accepted)
		updateItem(item, patch, universe);
}
