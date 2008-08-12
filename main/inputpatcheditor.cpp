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

#include "inputmap.h"
#include "inputpatcheditor.h"

InputPatchEditor::InputPatchEditor(QWidget* parent, InputMap* inputMap,
				   int universe, const QString& pluginName,
				   int input) : QDialog(parent)
{
	QStringList::iterator it;
	QString str;

	Q_ASSERT(inputMap != NULL);
	Q_ASSERT(universe < KUniverseCount);

	setupUi(this);

	m_inputMap = inputMap;
	m_pluginList = m_inputMap->pluginNames();
	m_pluginName = pluginName;
	m_universe = universe;
	setWindowTitle(str.sprintf("Route input universe %d thru...",
				   universe + 1));

	m_pluginCombo->addItem(KInputNone);

	for (it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
	{
		m_pluginCombo->addItem(*it);

		if (*it == m_pluginName)
		{
			/* Set the given plugin name as selected */
			m_pluginCombo->setCurrentIndex(
				m_pluginCombo->count() - 1);
			slotPluginActivated(m_pluginName);
		}
	}

	/* Set the given input line number as selected */
	m_input = input;
	m_inputCombo->setCurrentIndex(m_input);

	connect(m_pluginCombo, SIGNAL(currentIndexChanged(const QString&)),
		this, SLOT(slotPluginActivated(const QString&)));
	connect(m_inputCombo, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotInputActivated(int)));
}

InputPatchEditor::~InputPatchEditor()
{
}

void InputPatchEditor::slotPluginActivated(const QString& pluginName)
{
	QString str;

	Q_ASSERT(pluginName.isEmpty() == false);

	m_inputCombo->clear();

	/* Put the selected plugin's inputs to the input combo */
	for (int i = 0; i < m_inputMap->pluginInputs(pluginName); i++)
		m_inputCombo->addItem(str.sprintf("Input %d", i + 1));
	
	m_pluginName = pluginName;

	if (m_inputCombo->count() > 0)
		m_input = 0;
	else
		m_input = -1;
}

void InputPatchEditor::slotInputActivated(int line)
{
	m_input = line;
}
