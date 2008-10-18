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
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QDialog>
#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QDir>

#include <common/qlcinputchannel.h>
#include <common/qlcinputdevice.h>

#include "inputtemplateeditor.h"
#include "inputchanneleditor.h"

#define KColumnNumber 0
#define KColumnName   1
#define KColumnType   2

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
	connect(m_wizardButton, SIGNAL(clicked()),
		this, SLOT(slotWizardClicked()));

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
	QTreeWidgetItem* item;
	QLCInputChannel* ch;
	m_tree->clear();

	for (t_input_channel i = 0; i < m_deviceTemplate->channels(); i++)
	{
		ch = m_deviceTemplate->channel(i);
		Q_ASSERT(ch != NULL);

		item = new QTreeWidgetItem(m_tree);
		Q_ASSERT(item != NULL);

		updateChannelItem(item, ch);
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
		m_deviceTemplate->addChannel(channel);
	else
		delete channel;
}

void InputTemplateEditor::slotRemoveClicked()
{
	t_input_channel chnum;
	
	/* Remove all selected channels */
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());	
	while (it.hasNext() == true)
	{
		chnum = it.next()->text(KColumnNumber).toInt() - 1;

		/* Deletes also the channel object */
		m_deviceTemplate->removeChannel(chnum);
	}
}

void InputTemplateEditor::slotEditClicked()
{
	QLCInputChannel* channel;
	t_input_channel chnum;
	QTreeWidgetItem* item;
	
	item = m_tree->currentItem();
	if (item == NULL)
		return;

	/* Find the channel object associated to the selected tree item */
	chnum = item->text(KColumnNumber).toInt() - 1;
	channel = m_deviceTemplate->channel(chnum);
	Q_ASSERT(channel != NULL);

	/* Edit the channel and update its item if modifications were made */
	InputChannelEditor ice(this, channel);
	if (ice.exec() == QDialog::Accepted)
		updateChannelItem(item, channel);
}

void InputTemplateEditor::slotWizardClicked()
{
	/* TODO: A simple wizard that adds all such channels to the current
	   template that send data while the wizard dialog is open. i.e. the
	   user can just wiggle his sliders and buttons and all of them are
	   added to the template. */
}

/****************************************************************************
 * Device template
 ****************************************************************************/

const QLCInputDevice* InputTemplateEditor::deviceTemplate() const
{
	return m_deviceTemplate;
}
