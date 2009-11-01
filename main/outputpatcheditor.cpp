/*
  Q Light Controller
  outputpatcheditor.cpp

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
#include <QSettings>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>

#include "outputpatcheditor.h"
#include "fixturemanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "monitor.h"
#include "app.h"

#define KColumnName   0
#define KColumnOutput 1

extern App* _app;

OutputPatchEditor::OutputPatchEditor(QWidget* parent, int universe,
				     const OutputPatch* patch)
	: QDialog(parent)
{
	QSettings settings;
	QVariant value;
	QString key;

	setupUi(this);

	Q_ASSERT(universe < _app->outputMap()->universes());
	m_universe = universe;
	setWindowTitle(tr("Mapping properties for output universe %1")
			.arg(universe + 1));

	m_originalPluginName = patch->pluginName();
	m_currentPluginName = patch->pluginName();

	m_originalOutput = patch->output();
	m_currentOutput = patch->output();

	/* Selection changes */
	connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						  QTreeWidgetItem*)),
		this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));

	/* Configure button */
	connect(m_configureButton, SIGNAL(clicked()),
		this, SLOT(slotConfigureClicked()));

	/* Zero-based DMX setting */
	connect(m_zeroBasedDMXCheckBox, SIGNAL(clicked()),
		this, SLOT(slotZeroBasedDMXClicked()));
	key = QString("/outputmap/universe%1/dmxzerobased").arg(universe);
	value = settings.value(key);
	if (value.isValid() == true)
		m_zeroBasedDMXCheckBox->setChecked(value.toBool());
	else
		m_zeroBasedDMXCheckBox->setChecked(false);
	m_originalDMXZeroBasedSetting = m_zeroBasedDMXCheckBox->isChecked();

	fillTree();
}

OutputPatchEditor::~OutputPatchEditor()
{
}

void OutputPatchEditor::reject()
{
	/* Revert changes to original values (stored when this dialog opens) */
	_app->outputMap()->setPatch(m_universe, m_originalPluginName,
				    m_originalOutput);

	storeDMXZeroBasedSetting(m_originalDMXZeroBasedSetting);

	QDialog::reject();
}

void OutputPatchEditor::fillTree()
{
	QTreeWidgetItem* pitem;
	QTreeWidgetItem* oitem;
	QString pluginName;
	int i;

	/* Disable check state change tracking when the tree is filled */
	disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		   this, SLOT(slotItemChanged(QTreeWidgetItem*)));

	m_tree->clear();

	/* Go thru available plugins and put them as the tree's root nodes. */
	QStringListIterator pit(_app->outputMap()->pluginNames());
	while (pit.hasNext() == true)
	{
		i = 0;

		pluginName = pit.next();
		pitem = new QTreeWidgetItem(m_tree);
		pitem->setText(KColumnName, pluginName);
		pitem->setText(KColumnOutput, QString("%1")
							.arg(KOutputInvalid));

		/* Go thru available outputs provided by each plugin and put
		   them as their parent plugin's leaf nodes. */
		QStringListIterator oit(
				_app->outputMap()->pluginOutputs(pluginName));
		while (oit.hasNext() == true)
		{
			oitem = new QTreeWidgetItem(pitem);
			oitem->setText(KColumnName, oit.next());
			oitem->setText(KColumnOutput, QString("%1").arg(i));
			oitem->setFlags(oitem->flags() |
					Qt::ItemIsUserCheckable);

			/* Select the currently mapped plugin output and
			   expand its parent node */
			if (m_currentPluginName == pluginName &&
			    m_currentOutput == i)
			{
				oitem->setCheckState(KColumnName, Qt::Checked);
				pitem->setExpanded(true);
			}
			else
			{
				int uni;

				oitem->setCheckState(KColumnName, Qt::Unchecked);
				uni = _app->outputMap()->mapping(pluginName, i);
				if (uni != -1)
				{
					/* If a mapping exists for this plugin
					   and output, make it impossible to
					   map it to another universe. */
					oitem->setFlags(oitem->flags()
							& (!Qt::ItemIsEnabled));
					oitem->setText(KColumnName,
						oitem->text(KColumnName) +
						QString(" (Mapped to universe %1)")
							.arg(uni + 1));
				}
			}

			i++;
		}

		/* If no outputs were appended to the plugin node, put a
		   "Nothing" node there. */
		if (i == 0)
		{
			oitem = new QTreeWidgetItem(pitem);
			oitem->setText(KColumnName, KOutputNone);
			oitem->setText(KColumnOutput,
				       QString("%1").arg(KOutputInvalid));
			oitem->setFlags(oitem->flags() & ~Qt::ItemIsEnabled);
			oitem->setFlags(oitem->flags() & ~Qt::ItemIsSelectable);
			oitem->setCheckState(KColumnName, Qt::Unchecked);
		}
	}

        /* Enable check state change tracking after the tree has been filled */
	connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotItemChanged(QTreeWidgetItem*)));
}

void OutputPatchEditor::slotCurrentItemChanged(QTreeWidgetItem* item)
{
	QString info;

	if (item == NULL)
	{
		/* Nothing selected */
		info = tr("No information");
	}
	else
	{
		QString plugin;
		t_output output;

		if (item->parent() != NULL)
		{
			/* Output node selected */
			plugin = item->parent()->text(KColumnName);
			output = item->text(KColumnOutput).toInt();
		}
		else
		{
			/* Plugin node selected */
			plugin = item->text(KColumnName);
			output = KOutputInvalid;
		}

		info = _app->outputMap()->pluginStatus(plugin, output);
	}

	/* Display information for the selected plugin or output */
	m_infoBrowser->setText(info);
}

void OutputPatchEditor::slotItemChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
		return;

	if (item->checkState(KColumnName) == Qt::Checked)
	{
		/* Temporarily disable this signal to prevent an endless loop */
		disconnect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			   this, SLOT(slotItemChanged(QTreeWidgetItem*)));

		/* Set all other items unchecked... */
		QTreeWidgetItemIterator it(m_tree);
		while ((*it) != NULL)
		{
			/* Don't touch the item that was just checked nor
			   any parent nodes. */
			if (*it != item && (*it)->childCount() == 0)
			{
				/* Set all the rest of the nodes unchecked */
				(*it)->setCheckState(KColumnName,
						     Qt::Unchecked);
			}

			/* Next one */
			++it;
		}

		/* Start listening to this signal once again */
		connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this, SLOT(slotItemChanged(QTreeWidgetItem*)));
	}
	else
	{
		/* Don't allow unchecking an item by clicking it. Only allow
		   changing the check state by checking another item. */
		item->setCheckState(KColumnName, Qt::Checked);
	}

	/* Store the selected plugin name & input */
	if (item->parent() != NULL)
	{
		m_currentPluginName = item->parent()->text(KColumnName);
		m_currentOutput = item->text(KColumnOutput).toInt();
	}
	else
	{
		m_currentPluginName = QString::null;
		m_currentOutput = KOutputInvalid;
	}

	/* Apply the patch immediately so that input data can be used in the
	   input profile editor */
	_app->outputMap()->setPatch(m_universe, m_currentPluginName,
				    m_currentOutput);
}


void OutputPatchEditor::slotConfigureClicked()
{
	QTreeWidgetItem* item;
	QString plugin;

	/* Find out the currently selected plugin */
	item = m_tree->currentItem();
	if (item == NULL)
		return;
	else if (item->parent() != NULL)
		plugin = item->parent()->text(KColumnName);
	else
		plugin = item->text(KColumnName);

	/* Configure the plugin */
	_app->outputMap()->configurePlugin(plugin);

	/* Refill the mapping tree in case configuration changed something */
	fillTree();
}

void OutputPatchEditor::slotZeroBasedDMXClicked()
{
	storeDMXZeroBasedSetting(m_zeroBasedDMXCheckBox->isChecked());
}

void OutputPatchEditor::storeDMXZeroBasedSetting(bool set)
{
	Q_ASSERT(_app->outputMap() != NULL);
	_app->outputMap()->setDMXZeroBased(m_universe, set);

	/* Update fixture manager so the setting is visible immediately */
	if (FixtureManager::instance() != NULL)
		FixtureManager::instance()->updateView();

	/* Update monitor so the setting is visible immediately */
	if (Monitor::instance() != NULL)
		Monitor::instance()->updateFixtureLabelStyles();
}

