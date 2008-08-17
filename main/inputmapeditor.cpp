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
#include <QDebug>
#include <QMenu>

#include "common/qlcinplugin.h"

#include "inputpatcheditor.h"
#include "inputmapeditor.h"
#include "inputmap.h"

using namespace std;

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput    2
#define KColumnInputNum 3

InputMapEditor::InputMapEditor(QWidget* parent, InputMap* inputMap)
	: QDialog(parent)
{
	Q_ASSERT(inputMap != NULL);
	m_inputMap = inputMap;

	setupUi(this);

	connect(m_edit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));

	/* Clear the mapping list just in case and fill with plugins */
	m_tree->clear();
	for (t_input_universe i = 0; i < inputMap->universes(); i++)
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
		{
			/* Plugin name */
			item->setText(KColumnPlugin,
				      inputPatch->plugin->name());

			/* Plugin's input name */
			item->setText(KColumnInput,
				      inputPatch->plugin->inputs()
				      [inputPatch->input]);

			/* Plugin's input number */
			item->setText(KColumnInputNum,
				      str.setNum(inputPatch->input + 1));
		}
		else
		{
			item->setText(KColumnPlugin, KInputNone);
			item->setText(KColumnInput, KInputNone);
			item->setText(KColumnInputNum, KInputNone);
		}
	}
}

InputMapEditor::~InputMapEditor()
{
}

/*****************************************************************************
 * Slots
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
	input = item->text(KColumnInputNum).toInt() - 1;

	InputPatchEditor ipe(this, m_inputMap, universe, pluginName, input);
	if (ipe.exec() == QDialog::Accepted)
	{
		QString str;
		item->setText(KColumnUniverse, str.setNum(universe + 1));
		if (ipe.pluginName() != KInputNone &&
		    ipe.pluginName().isEmpty() == false)
		{
			item->setText(KColumnPlugin, ipe.pluginName());
			item->setText(KColumnInput, ipe.inputName());
			item->setText(KColumnInputNum,
				      str.setNum(ipe.input() + 1));
		}
		else
		{
			item->setText(KColumnPlugin, KInputNone);
			item->setText(KColumnInput, KInputNone);
			item->setText(KColumnInputNum, KInputNone);
		}
	}
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
		input = (*it)->text(KColumnInputNum).toInt() - 1;

		m_inputMap->setPatch(universe, pluginName, input);

		++it;
	}

	QDialog::accept();
}
