/*
  Q Light Controller
  inputtemplateeditor.cpp

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
#include <QTextBrowser>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QDialog>
#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QList>
#include <QDir>

#include <common/qlcinputchannel.h>
#include <common/qlcinputdevice.h>

#include "inputtemplateeditor.h"
#include "inputchanneleditor.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

#define KColumnNumber 0
#define KColumnName   1
#define KColumnType   2
#define KColumnValues 3

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputTemplateEditor::InputTemplateEditor(QWidget* parent,
					 QLCInputDevice* deviceTemplate)
	: QDialog(parent)
{
	setupUi(this);

	/* Set icons to buttons */
	m_addButton->setIcon(QIcon(":/edit_add.png"));
	m_removeButton->setIcon(QIcon(":/edit_remove.png"));
	m_editButton->setIcon(QIcon(":/edit.png"));
	m_wizardButton->setIcon(QIcon(":/wizard.png"));

	/* Connect the buttons to slots */
	connect(m_addButton, SIGNAL(clicked()),
		this, SLOT(slotAddClicked()));
	connect(m_removeButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveClicked()));
	connect(m_editButton, SIGNAL(clicked()),
		this, SLOT(slotEditClicked()));
	connect(m_wizardButton, SIGNAL(clicked(bool)),
		this, SLOT(slotWizardClicked(bool)));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));

	if (deviceTemplate == NULL)
	{
		m_deviceTemplate = new QLCInputDevice(this);
	}
	else
	{
		m_deviceTemplate = new QLCInputDevice(*deviceTemplate);
		if ((QFile::permissions(m_deviceTemplate->path()) &
		     QFile::WriteUser) == 0)
		{
			QMessageBox::warning(this, tr("File not writable"),
				tr("You do not have permission to write to "
				   "the file %1. You might not be able to "
				   "save your modifications to the template.")
				   .arg(QDir::toNativeSeparators(
						m_deviceTemplate->path())));
		}
	}

	/* Device manufacturer & model */
	m_manufacturerEdit->setText(m_deviceTemplate->manufacturer());
	m_modelEdit->setText(m_deviceTemplate->model());

	/* Fill up the tree with template channels */
	fillTree();
}

InputTemplateEditor::~InputTemplateEditor()
{
	delete m_deviceTemplate;
}

void InputTemplateEditor::fillTree()
{
	m_tree->clear();

	QMapIterator <t_channel, QLCInputChannel*>
		it(m_deviceTemplate->channels());
	while (it.hasNext() == true)
	{
		it.next();
		updateChannelItem(new QTreeWidgetItem(m_tree), it.value());
	}
}

void InputTemplateEditor::updateChannelItem(QTreeWidgetItem* item,
					    QLCInputChannel* ch)
{
	Q_ASSERT(item != NULL);
	Q_ASSERT(ch != NULL);

	item->setText(KColumnNumber, QString("%1").arg(ch->channel() + 1));
	item->setText(KColumnName, ch->name());
	item->setText(KColumnType, QLCInputChannel::typeToString(ch->type()));

	/* Display nice icons to indicate channel type */
	if (ch->type() == QLCInputChannel::Slider)
		item->setIcon(KColumnType, QIcon(":/slider.png"));
	else if (ch->type() == QLCInputChannel::Knob)
		item->setIcon(KColumnType, QIcon(":/knob.png"));
	else if (ch->type() == QLCInputChannel::Button)
		item->setIcon(KColumnType, QIcon(":/button.png"));
}

/****************************************************************************
 * OK & Cancel
 ****************************************************************************/

void InputTemplateEditor::accept()
{
	m_deviceTemplate->setManufacturer(m_manufacturerEdit->text());
	m_deviceTemplate->setModel(m_modelEdit->text());

	if (m_deviceTemplate->manufacturer().isEmpty() == true ||
	    m_deviceTemplate->model().isEmpty() == true)
	{
		QMessageBox::warning(this, tr("Missing information"),
			     tr("Manufacturer and/or model name is missing."));
	}
	else
	{
		QDialog::accept();
	}
}

/****************************************************************************
 * Editing
 ****************************************************************************/

void InputTemplateEditor::slotAddClicked()
{
	QLCInputChannel* channel = new QLCInputChannel(m_deviceTemplate);
	InputChannelEditor ice(this, channel);
	if (ice.exec() == QDialog::Accepted)
	{
		m_deviceTemplate->addChannel(channel);
		updateChannelItem(new QTreeWidgetItem(m_tree), channel);
	}
	else
	{
		delete channel;
	}
}

void InputTemplateEditor::slotRemoveClicked()
{
	QList <QTreeWidgetItem*> selected;
	QTreeWidgetItem* next = NULL;
	t_input_channel chnum;

	/* Ask for confirmation if we're deleting more than one channel */
	selected = m_tree->selectedItems();
	if (selected.count() > 1)
	{
		int r;
		r = QMessageBox::question(this, tr("Delete channels"),
		      tr("Delete all %1 selected channels?")
		      .arg(selected.count()),
		      QMessageBox::Yes | QMessageBox::No);
		if (r == QMessageBox::No)
			return;
	}

	/* Remove all selected channels */
	QMutableListIterator <QTreeWidgetItem*> it(selected);
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;

		item = it.next();
		Q_ASSERT(item != NULL);

		/* Delete the channel object */
		chnum = item->text(KColumnNumber).toInt() - 1;
		m_deviceTemplate->removeChannel(chnum);

		/* Choose the closest item below or above the removed items
		   as the one that is selected after the removal */
		next = m_tree->itemBelow(item);
		if (next == NULL)
			next = m_tree->itemAbove(item);

		delete item;
	}

	m_tree->setCurrentItem(next);
}

void InputTemplateEditor::slotEditClicked()
{
	QLCInputChannel* channel;
	t_input_channel chnum;
	QTreeWidgetItem* item;

	if (m_tree->selectedItems().count() == 1)
	{
		/* Just one item selected. Edit that. */
		item = m_tree->currentItem();
		if (item == NULL)
			return;

		/* Find the channel object associated to the selected item */
		chnum = item->text(KColumnNumber).toInt() - 1;
		channel = m_deviceTemplate->channel(chnum);
		Q_ASSERT(channel != NULL);

		/* Edit the channel and update its item if necessary */
		InputChannelEditor ice(this, channel);
		if (ice.exec() == QDialog::Accepted)
		{
			if (ice.channel() != KInputChannelInvalid)
				channel->setChannel(ice.channel());
			if (ice.name() != QString::null)
				channel->setName(ice.name());
			if (ice.type() != QLCInputChannel::NoType)
				channel->setType(ice.type());

                        updateChannelItem(item, channel);
		}
	}
	else if (m_tree->selectedItems().count() > 1)
	{
		/* Multiple channels selected. Apply changes to all of them */
		InputChannelEditor ice(this, NULL);
		if (ice.exec() == QDialog::Accepted)
		{
			QListIterator <QTreeWidgetItem*> 
						it(m_tree->selectedItems());
			while (it.hasNext() == true)
			{
				item = it.next();
				Q_ASSERT(item != NULL);

				chnum = item->text(KColumnNumber).toInt() - 1;
				channel = m_deviceTemplate->channel(chnum);
				Q_ASSERT(channel != NULL);

				/* Set only name and type and only if they
				   have been modified. */
				if (ice.name() != QString::null)
					channel->setName(ice.name());
				if (ice.type() != QLCInputChannel::NoType)
					channel->setType(ice.type());

				updateChannelItem(item, channel);
			}
		}
	}
}

void InputTemplateEditor::slotWizardClicked(bool checked)
{
	if (checked == true)
	{
		connect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
	else
	{
		disconnect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
}

void InputTemplateEditor::slotInputValueChanged(t_input_universe universe,
						t_input_channel channel,
						t_input_value value)
{
	Q_UNUSED(universe);

	/* Get a list of items that represent the given channel. Basically
	   the list should always contain just one item. */
	QList <QTreeWidgetItem*> list(m_tree->findItems(
					QString("%1").arg(channel + 1),
					Qt::MatchExactly, KColumnNumber));
	if (list.count() == 0)
	{
		/* No channel items found. Create a new channel to the
		   template and display it also in the tree widget */
		QTreeWidgetItem* item;
		QLCInputChannel* ch;

		ch = new QLCInputChannel(m_deviceTemplate);
		ch->setChannel(channel);
		ch->setName(tr("Button %1").arg(channel + 1));
		ch->setType(QLCInputChannel::Button);
		m_deviceTemplate->addChannel(ch);

		item = new QTreeWidgetItem(m_tree);
		updateChannelItem(item, ch);
		m_tree->scrollToItem(item);
	}
	else
	{
		/* Existing channel & item found. Modify their contents. */
		QStringList values;
		values = list.first()->text(KColumnValues).split(",");

		if (values.contains(QString("%1").arg(value)) == false)
		{
			values << QString("%1").arg(value);
			values.sort();
			list.first()->setText(KColumnValues, values.join(","));
		}

		/* Change the channel type only the one time when its value
		   count goes over 2. I.e. when a channel can have more than
		   two distinct values, it can no longer be a button. */
		if (values.count() == 3)
		{
			QLCInputChannel* ch;
			ch = m_deviceTemplate->channel(channel);
			Q_ASSERT(ch != NULL);

			ch->setType(QLCInputChannel::Slider);
			ch->setName(tr("Slider %1").arg(channel + 1));
			updateChannelItem(list.first(), ch);
		}
	}
}

/****************************************************************************
 * Device template
 ****************************************************************************/

const QLCInputDevice* InputTemplateEditor::deviceTemplate() const
{
	return m_deviceTemplate;
}
