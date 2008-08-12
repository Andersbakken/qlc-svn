/*
  Q Light Controller
  inputmapeditor.cpp
  
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
#include <iostream>
#include <QAction>
#include <QMenu>

#include "common/qlcinplugin.h"

#include "inputpatcheditor.h"
#include "inputmapeditor.h"
#include "inputmap.h"

using namespace std;

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput   2

InputMapEditor::InputMapEditor(QWidget* parent, InputMap* inputMap)
	: QDialog(parent)
{
	Q_ASSERT(inputMap != NULL);
	m_inputMap = inputMap;
	m_pluginList = m_inputMap->pluginNames();

	setupUi(this);

	m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_edit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));
	connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(slotContextMenuRequested(const QPoint&)));

	/* Clear the mapping list just in case and fill with plugins */
	m_tree->clear();
	for (int i = 0; i < KUniverseCount; i++)
	{
		QTreeWidgetItem* item;
		InputPatch* inputPatch;
		QString str;
		
		inputPatch = m_inputMap->patch(i);
		Q_ASSERT(inputPatch != NULL);
		
		item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnUniverse, str.setNum(i + 1));

		/* Plugin name */
		if (inputPatch->plugin != NULL)
			item->setText(KColumnPlugin, inputPatch->plugin->name());
		else
			item->setText(KColumnPlugin, KInputNone);

		/* Input line */
		if (inputPatch->input < 0)
			item->setText(KColumnInput, KInputNone);
		else
			item->setText(KColumnInput,
				      str.setNum(inputPatch->input + 1));
	}
}

InputMapEditor::~InputMapEditor()
{
}

/*****************************************************************************
 *
 *****************************************************************************/

void InputMapEditor::slotEditClicked()
{
	QTreeWidgetItem* item;
	int universe;
	QString pluginName;
	int input;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	pluginName = item->text(KColumnPlugin);
	input = item->text(KColumnInput).remove("Input").toInt() - 1;

	InputPatchEditor dpe(this, m_inputMap, universe, pluginName, input);
	if (dpe.exec() == QDialog::Accepted)
	{
		QString str;
		item->setText(KColumnUniverse, str.setNum(universe + 1));
		item->setText(KColumnPlugin, dpe.pluginName());
		if (dpe.input() >= 0)
			item->setText(KColumnInput, str.setNum(dpe.input()+1));
		else
			item->setText(KColumnInput, KInputNone);
	}
}

void InputMapEditor::slotContextMenuRequested(const QPoint& point)
{
	QStringList::Iterator it;
	QTreeWidgetItem* item;
	QAction* action;
	QString name;
	int universe;
	int inputs;
	QString str;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	QMenu pluginMenu(this);
	pluginMenu.setTitle(QString("Route universe %1 thru...")
			    .arg(item->text(KColumnUniverse)));
	universe = item->text(KColumnUniverse).toInt();

	for (it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
	{
		name = *it;

		inputs = m_inputMap->pluginInputs(name);
		if (inputs >= 0)
		{
			/* Put the plugin's inputs into a sub menu and
			   insert that submenu to the top level menu */
			QMenu inputMenu(&pluginMenu);
			inputMenu.setTitle(name);
			pluginMenu.addMenu(&inputMenu);

			for (int i = 0; i < inputs && i < 100; i++)
			{
				action = inputMenu.addAction(
					QString("Input %1").arg(i + 1));
				action->setData(name);
			}
		}
	}

	/* Execute menu and check, whether something was selected */
	action = pluginMenu.exec(m_tree->mapToGlobal(point));
	if (action == NULL)
		return;

	/* Extract selected information from the action and patch them */
	inputs = action->text().remove("Input").toInt() - 1;
	name = action->data().toString();
	m_inputMap->setPatch(universe, name, inputs);
	
	/* Update the current tree widget item */
	item->setText(KColumnUniverse, str.setNum(universe + 1));
	item->setText(KColumnPlugin, name);
	item->setText(KColumnInput, str.setNum(inputs + 1));
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void InputMapEditor::accept()
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		QString pluginName;
		int universe;
		int input;

		universe = (*it)->text(KColumnUniverse).toInt() - 1;
		pluginName = (*it)->text(KColumnPlugin);
		input = (*it)->text(KColumnInput).toInt() - 1;

		m_inputMap->setPatch(universe, pluginName, input);

		++it;
	}

	QDialog::accept();
}
