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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QToolButton>
#include <QComboBox>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QIcon>

#include "common/qlcinputdevice.h"
#include "common/qlctypes.h"

#include "inputpatcheditor.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

#define KColumnName  0
#define KColumnInput 1

InputPatchEditor::InputPatchEditor(QWidget* parent, t_input_universe universe,
				   const QString& pluginName, t_input input)
	: QDialog(parent)
{
	/* Setup UI controls */
	setupUi(this);

	m_configureButton->setIcon(QIcon(":/configure.png"));
	connect(m_configureButton, SIGNAL(clicked()),
		this, SLOT(slotConfigureClicked()));

	connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						  QTreeWidgetItem*)),
		this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));
	connect(m_deviceCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotDeviceComboActivated(const QString&)));

	m_configureDevicesButton->setIcon(QIcon(":/input.png"));
	//m_deviceGroup->setEnabled(false);
	
	/* Universe */
	Q_ASSERT(universe < inputMap->universes());
	m_universe = universe;
	this->setWindowTitle(tr("Mapping properties for input universe %1")
			     .arg(universe + 1));
	m_tree->setToolTip(tr("Patch input data from the selected plugin input"
			      " to be available in input universe %1")
			      .arg(universe + 1));

	/* Selected plugin & input */
	m_pluginName = pluginName;
	m_input = input;
	
	fillTree();
	fillDeviceCombo();
}

InputPatchEditor::~InputPatchEditor()
{
}

/****************************************************************************
 * Plugin tree
 ****************************************************************************/

void InputPatchEditor::fillTree()
{
	QTreeWidgetItem* pitem;
	QTreeWidgetItem* iitem;
	QString pluginName;
	int i;
	
	m_tree->clear();

	/* Add an empty item so that user can choose not to assign any plugin
	   to an input universe */
	pitem = new QTreeWidgetItem(m_tree);
	pitem->setText(KColumnName, KInputNone);
	pitem->setText(KColumnInput, QString("%1").arg(KInputInvalid));
	
	/* Set "Nothing" selected if there is no valid input selected */
	if (m_input == KInputInvalid)
		m_tree->setCurrentItem(pitem);
	
	/* Go thru available plugins and put them as the tree's root nodes. */
	QStringListIterator pit(_app->inputMap()->pluginNames());
	while (pit.hasNext() == true)
	{
		i = 0;

		pluginName = pit.next();
		pitem = new QTreeWidgetItem(m_tree);
		pitem->setText(KColumnName, pluginName);
		pitem->setText(KColumnInput, QString("%1").arg(KInputInvalid));

		/* Go thru available inputs provided by each plugin and put
		   them as their parent plugin's leaf nodes. */
		QStringListIterator iit(_app->inputMap()->pluginInputs(
								pluginName));
		while (iit.hasNext() == true)
		{
			iitem = new QTreeWidgetItem(pitem);
			iitem->setText(KColumnName, iit.next());
			iitem->setText(KColumnInput, QString("%1").arg(i));
			
			/* Select that plugin's line that is currently mapped */
			if (m_pluginName == pluginName && m_input == i)
				m_tree->setCurrentItem(iitem);

			i++;
		}
		
		/* If no inputs were appended to the plugin node, put a
		   "Nothing" node there. */
		if (i == 0)
		{
			iitem = new QTreeWidgetItem(pitem);
			iitem->setText(KColumnName, KInputNone);
			iitem->setText(KColumnInput,
				       QString("%1").arg(KInputInvalid));
		}
	}
}

void InputPatchEditor::slotCurrentItemChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
	{
		m_input = KInputInvalid;
		m_inputName = QString::null;
		m_pluginName = QString::null;
		m_configureButton->setEnabled(false);
	}
	else
	{
		m_input = item->text(KColumnInput).toInt();
		if (m_input != KInputInvalid)
		{
			m_inputName = item->text(KColumnName);
			m_pluginName = item->parent()->text(KColumnName);
		}
		else
		{
			m_inputName = QString::null;
			m_pluginName = item->text(KColumnName);
		}
		
		if (m_pluginName == KInputNone)
			m_configureButton->setEnabled(false);
		else
			m_configureButton->setEnabled(true);
	}
	
	updateInputInfo();
}

/****************************************************************************
 * Information view
 ****************************************************************************/

void InputPatchEditor::updateInputInfo()
{
	QString status;
	status = _app->inputMap()->pluginStatus(m_pluginName, m_input);
	m_infoBrowser->setText(status);
}

void InputPatchEditor::slotConfigureClicked()
{
	_app->inputMap()->configurePlugin(m_pluginName);
}

/****************************************************************************
 * Templates
 ****************************************************************************/

void InputPatchEditor::fillDeviceCombo()
{
	m_deviceCombo->clear();

	/* Get a list of available input device templates & their file paths */
	QStringList names, paths;
	QLCInputDevice::availableTemplates(&names, &paths);
	Q_ASSERT(names.count() == files.count());

	/* Add an option to have no template at all */
	m_deviceCombo->addItem(tr("Nothing"));

	/* Insert available input device templates and their file paths
	   to the device combo. */
	QStringListIterator nit(names);
	QStringListIterator pit(names);
	while (nit.hasNext() == true && pit.hasNext() == true)
		m_deviceCombo->addItem(nit.next(), QVariant(pit.next()));
}

void InputPatchEditor::slotDeviceComboActivated(const QString& name)
{
}
