/*
  Q Light Controller
  addfixture.cpp

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

#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <iostream>
#include <QLabel>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"

#include "addfixture.h"
#include "app.h"
#include "doc.h"

using namespace std;

extern App* _app;

static const int KColumnName    ( 0 );
static const int KColumnType    ( 1 );
static const int KColumnPointer ( 2 );

AddFixture::AddFixture(QWidget *parent) : QDialog(parent)
{
	m_addressValue = 0;
	m_universeValue = 0;
	m_amountValue = 1;
	m_gapValue = 0;
	m_channelsValue = 0;
	m_fixtureDef = NULL;
	m_mode = NULL;

	setupUi(this);

	m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

	connect(m_tree, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectionChanged()));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotTreeDoubleClicked(QTreeWidgetItem*)));
	connect(m_modeCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotModeActivated(const QString&)));
	connect(m_channelsSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotChannelsChanged(int)));
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));

	/* Fill fixture definition tree */
	fillTree();

	/* Simulate the first selection change (none) */
	slotSelectionChanged();
}

AddFixture::~AddFixture()
{
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

QTreeWidgetItem* AddFixture::findNode(const QString& text)
{
	QList <QTreeWidgetItem*> list = m_tree->findItems(text,
							  Qt::MatchExactly,
							  KColumnName);
	if (list.isEmpty() == true)
		return NULL;
	else
		return list.at(0);
}

/*****************************************************************************
 * Fillers
 *****************************************************************************/

void AddFixture::fillTree()
{
	QLCFixtureDef* fixtureDef;
	QTreeWidgetItem* parent;
	QTreeWidgetItem* node;
	QString str;

	/* Clear the tree of any previous data */
	m_tree->clear();

	/* Add all known fixture definitions. */
	QListIterator <QLCFixtureDef*> it(*_app->fixtureDefList());
	while (it.hasNext() == true)
	{
		fixtureDef = it.next();
		parent = NULL;

		/* Find an existing manufacturer parent node */
		parent = findNode(fixtureDef->manufacturer());

		/* If an existing manufacturer parent node was not found,
		   we must create one. Otherwise we use the found node
		   as the parent for the new item. */
		if (parent == NULL)
		{
			parent = new QTreeWidgetItem(m_tree);
			parent->setText(KColumnName,
					fixtureDef->manufacturer());
		}

		/* Create a new fixture node, under the parent node */
		node = new QTreeWidgetItem(parent);
		node->setText(KColumnName, fixtureDef->model());
		node->setText(KColumnType, fixtureDef->type());

		/* Store the fixture pointer into the tree */
		str.sprintf("%lu", (unsigned long) fixtureDef);
		node->setText(KColumnPointer, str);
	}

	/* Create a node & parent for generic dimmers */
	parent = new QTreeWidgetItem(m_tree);
	parent->setText(KColumnName, KXMLFixtureGeneric);
	node = new QTreeWidgetItem(parent);
	node->setText(KColumnName, KXMLFixtureGeneric);
	node->setText(KColumnType, KXMLFixtureDimmer);
	node->setText(KColumnPointer, "0");
}

void AddFixture::fillModeCombo(const QString& text)
{
	m_modeCombo->clear();

	if (m_fixtureDef == NULL)
	{
		m_modeCombo->setEnabled(false);
		m_modeCombo->addItem(text);
		m_modeCombo->setCurrentIndex(0);
	}
	else
	{
		m_modeCombo->setEnabled(true);

		QListIterator <QLCFixtureMode*> it(*m_fixtureDef->modes());
		while (it.hasNext() == true)
			m_modeCombo->addItem(it.next()->name());

		/* Select the first mode by default */
		m_modeCombo->setCurrentIndex(0);
		slotModeActivated(m_modeCombo->currentText());
	}
}

/*****************************************************************************
 * Slots
 *****************************************************************************/

void AddFixture::slotChannelsChanged(int value)
{
	/* Set the maximum possible address so that channels cannot
	   overflow beyond DMX's range of 512 channels */
	m_addressSpin->setRange(1, 513 - value);
}

void AddFixture::slotModeActivated(const QString& modeName)
{
	if (m_fixtureDef == NULL)
		return;

	m_mode = m_fixtureDef->mode(modeName);
	if (m_mode == NULL)
	{
		/* Generic dimmers don't have modes, so bail out */
		// slotSelectionChanged();
		return;
	}

	m_channelsSpin->setValue(m_mode->channels());

	/* Show all selected mode channels in the list */
	m_channelList->clear();
	for (int i = 0; i < m_mode->channels(); i++)
	{
		QLCChannel* channel = m_mode->channel(i);
		Q_ASSERT(channel != NULL);

		new QListWidgetItem(channel->name(), m_channelList);
	}
}

void AddFixture::slotSelectionChanged()
{
	QTreeWidgetItem* item;
	QString manuf;
	QString model;

	/* If there is no valid selection, i.e. the user has selected
	   only the manufacturer, don't let the user press OK */
	item = m_tree->currentItem();
	if (item == NULL || item->parent() == NULL)
	{
		/* Reset the selected fixture pointer */
		m_fixtureDef = NULL;
		fillModeCombo();
		
		if (m_nameEdit->isModified() == false)
			m_nameEdit->setText(QString::null);
		m_nameEdit->setEnabled(false);
		
		m_channelsSpin->setValue(0);
		m_channelList->clear();
		m_addressSpin->setEnabled(false);
		m_universeSpin->setEnabled(false);
		
		m_amountSpin->setEnabled(false);
		m_gapSpin->setEnabled(false);
		m_channelsSpin->setEnabled(false);
		
		m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
	}
	else
	{
		if (item->text(KColumnName) == KXMLFixtureGeneric &&
		    item->parent()->text(KColumnName) == KXMLFixtureGeneric)
		{
			/* Generic dimmer selected. User enters number of
			   channels. */
			m_fixtureDef = NULL;
			fillModeCombo(KXMLFixtureGeneric);
			m_channelsSpin->setEnabled(true);
			m_channelList->clear();

			/* Set the model name as the fixture's friendly name ONLY
			   if the user hasn't modified the friendly name field. */	
			if (m_nameEdit->isModified() == false)
				m_nameEdit->setText(KXMLFixtureDimmer +
						    QString("s")); // Plural :)
			m_nameEdit->setEnabled(true);
		}
		else
		{
			/* Specific fixture selected. Def contains num of chs */
			m_fixtureDef = (QLCFixtureDef*) 
				item->text(KColumnPointer).toULong();
			Q_ASSERT(m_fixtureDef != NULL);

			fillModeCombo();
			m_channelsSpin->setEnabled(false);
			
			/* Set the model name as the fixture's friendly name ONLY
			   if the user hasn't modified the friendly name field. */	
			if (m_nameEdit->isModified() == false)
				m_nameEdit->setText(m_fixtureDef->model());
			m_nameEdit->setEnabled(true);
		}

		/* Guide the user to edit the friendly name field */
		m_nameEdit->setSelection(0, m_nameEdit->text().length());
		m_nameEdit->setFocus();
		
		m_addressSpin->setEnabled(true);
		m_universeSpin->setEnabled(true);

		m_amountSpin->setEnabled(true);
		m_gapSpin->setEnabled(true);
		
		m_buttonBox->setStandardButtons(QDialogButtonBox::Ok |
						QDialogButtonBox::Cancel);
	}
}

void AddFixture::slotTreeDoubleClicked(QTreeWidgetItem* item)
{
	slotSelectionChanged();
	if (item != NULL && item->parent() != NULL)
		accept();
}

void AddFixture::slotNameEdited(const QString &text)
{
	/* If the user clears the text in the name field,
	   start substituting the name with the model again. */
	if (text.length() == 0)
		m_nameEdit->setModified(false);
}

void AddFixture::accept()
{
	m_nameValue = m_nameEdit->text();

	m_addressValue = m_addressSpin->value() - 1;
	m_universeValue = m_universeSpin->value() - 1;
	
	m_amountValue = m_amountSpin->value();
	m_gapValue = m_gapSpin->value();

	m_channelsValue = m_channelsSpin->value();
	
	QDialog::accept();
}
