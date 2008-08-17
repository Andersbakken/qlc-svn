/*
  Q Light Controller
  inputpatcheditor.cpp

  Copyright (C) Heikki Junnila

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

#include <QPushButton>
#include <QComboBox>
#include <QDebug>

#include "common/qlctypes.h"

#include "inputpatcheditor.h"
#include "inputmap.h"

InputPatchEditor::InputPatchEditor(QWidget* parent, InputMap* inputMap,
				   t_input_universe universe,
				   const QString& pluginName, t_input input)
	: QDialog(parent)
{
	/* Setup UI controls */
	setupUi(this);

	connect(m_pluginCombo, SIGNAL(currentIndexChanged(const QString&)),
		this, SLOT(slotPluginActivated(const QString&)));
	connect(m_inputCombo, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotInputActivated(int)));

	/* InputMap */
	Q_ASSERT(inputMap != NULL);
	m_inputMap = inputMap;

	/* Universe */
	Q_ASSERT(universe < inputMap->universes());
	m_universe = universe;
	setWindowTitle(QString("Route input universe %1 thru...")
		       .arg(universe + 1));
	
	m_pluginName = pluginName;
	m_pluginCombo->addItem(KInputNone);
	m_pluginCombo->addItems(inputMap->pluginNames());

	/* Set the given plugin name as selected */
	int index = m_pluginCombo->findText(pluginName);
	m_pluginCombo->setCurrentIndex(index);

	/* Set the given input line number as selected */
	m_input = input;
	m_inputCombo->setCurrentIndex(m_input);
}

InputPatchEditor::~InputPatchEditor()
{
}

void InputPatchEditor::slotPluginActivated(const QString& pluginName)
{
	QStringList inputs;
	QString str;

	Q_ASSERT(pluginName.isEmpty() == false);

	m_inputCombo->clear();

	/* Put the selected plugin's inputs to the input combo */
	inputs = m_inputMap->pluginInputs(pluginName);
	m_inputCombo->addItems(inputs);
	
	m_pluginName = pluginName;

	if (m_inputCombo->count() > 0)
		m_input = 0;
	else
		m_input = -1;
}

void InputPatchEditor::slotInputActivated(int input)
{
	m_input = (t_input) input;
	m_inputName = m_inputCombo->itemText(input);
}
