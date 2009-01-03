/*
  Q Light Controller
  inputmanager.cpp

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
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include <common/qlcinplugin.h>

#include "inputpatcheditor.h"
#include "inputmanager.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput    2
#define KColumnDevice   3
#define KColumnEditor   4
#define KColumnInputNum 5

extern App* _app;

InputManager::InputManager(QWidget* parent) : QWidget(parent)
{
	init();

	/* Timer that clears the input data icon after a while */
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

	/* Listen to input map's input data signals */
	connect(_app->inputMap(),
		SIGNAL(inputValueChanged(t_input_universe, t_input_channel,
					 t_input_value)),
		this,
		SLOT(slotInputValueChanged(t_input_universe, t_input_channel,
					   t_input_value)));

	update();
}

InputManager::~InputManager()
{
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void InputManager::update()
{
	m_tree->clear();
	for (t_input_universe i = 0; i < _app->inputMap()->universes(); i++)
	{
		InputPatch* inputPatch;
		QTreeWidgetItem* item;

		inputPatch = _app->inputMap()->patch(i);
		Q_ASSERT(inputPatch != NULL);

		item = new QTreeWidgetItem(m_tree);
		updateItem(item, inputPatch, i);
	}
}

void InputManager::init()
{
	QStringList columns;

	/* Create a new layout for this widget */
	new QVBoxLayout(this);

	/* Toolbar */
	m_toolbar = new QToolBar(tr("Input Manager"), this);
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
		<< tr("Input")
		<< tr("Device")
		<< tr("Editor universe");
	m_tree->setHeaderLabels(columns);

	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));
}

void InputManager::updateItem(QTreeWidgetItem* item, InputPatch* ip,
			      t_input_universe universe)
{
	Q_ASSERT(item != NULL);
	Q_ASSERT(ip != NULL);

	item->setText(KColumnUniverse, tr("%1").arg(universe + 1));
	item->setText(KColumnPlugin, ip->pluginName());
	item->setText(KColumnInput, ip->inputName());
	item->setText(KColumnDevice, ip->deviceName());
	if (_app->inputMap()->editorUniverse() == universe)
	{
		item->setCheckState(KColumnEditor, Qt::Checked);
		item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
	}
	item->setText(KColumnInputNum, QString("%1").arg(ip->input() + 1));
}

void InputManager::slotInputValueChanged(t_input_universe universe,
					 t_input_channel channel,
					 t_input_value value)
{
	QTreeWidgetItem* item;

	Q_UNUSED(channel);
	Q_UNUSED(value);

	item = m_tree->topLevelItem(universe);
	if (item == NULL)
		return;

	/* Show an icon on a universe row that received input data */
	QIcon icon(":/input.png");
	item->setIcon(KColumnUniverse, icon);

	/* Restart the timer */
	m_timer->start(250);
}

void InputManager::slotTimerTimeout()
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		(*it)->setIcon(KColumnUniverse, QIcon());
		++it;
	}
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void InputManager::slotEditClicked()
{
	t_input_universe universe;
	QTreeWidgetItem* item;
	InputPatch* patch;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	patch = _app->inputMap()->patch(universe);
	Q_ASSERT(patch != NULL);

	InputPatchEditor ipe(this, universe, patch);
	if (ipe.exec() == QDialog::Accepted)
		update();
}
