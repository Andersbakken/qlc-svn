/*
  Q Light Controller
  dmxpatcheditor.cpp

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

#include "dmxmap.h"
#include "dmxpatcheditor.h"

DMXPatchEditor::DMXPatchEditor(QWidget* parent, DMXMap* dmxMap, int universe,
			       const QString& pluginName, int output)
	: QDialog(parent)
{
	QStringList::iterator it;
	QString str;

	Q_ASSERT(dmxMap != NULL);
	Q_ASSERT(universe < KUniverseCount);

	setupUi(this);

	m_dmxMap = dmxMap;
	m_pluginList = m_dmxMap->pluginNames();
	m_pluginName = pluginName;
	m_universe = universe;
	setWindowTitle(str.sprintf("Route universe %d thru...", universe + 1));

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

	/* Set the given output line number as selected */
	m_output = output;
	m_outputCombo->setCurrentIndex(m_output);

	connect(m_pluginCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotPluginActivated(const QString&)));
	connect(m_outputCombo, SIGNAL(activated(int)),
		this, SLOT(slotOutputActivated(int)));
}

DMXPatchEditor::~DMXPatchEditor()
{
}

void DMXPatchEditor::slotPluginActivated(const QString& pluginName)
{
	QString str;

	Q_ASSERT(pluginName.isEmpty() == false);

	m_outputCombo->clear();

	/* Put the selected plugin's outputs to the output combo */
	for (int i = 0; i < m_dmxMap->pluginOutputs(pluginName); i++)
		m_outputCombo->addItem(str.sprintf("Output %d", i + 1));

	m_pluginName = pluginName;
	m_output = 0;
}

void DMXPatchEditor::slotOutputActivated(int line)
{
	m_output = line;
}
