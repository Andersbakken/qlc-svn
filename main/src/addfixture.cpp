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

#include <qlistview.h>
#include <qheader.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <assert.h>
#include <qmessagebox.h>

#include "common/settings.h"
#include "common/qlcfixturedef.h"
#include "common/qlcfixturemode.h"

#include "addfixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

static const int KColumnName    ( 0 );
static const int KColumnType    ( 1 );
static const int KColumnPointer ( 2 );

AddFixture::AddFixture(QWidget *parent)
	: UI_AddFixture(parent, "Add fixture", true),
	
	  m_addressValue        ( 0 ),
	  m_universeValue       ( 0 ),
	  m_multipleNumberValue ( 1 ),
	  m_addressGapValue     ( 0 ),
	  m_channelsValue       ( 0 )
{
}

AddFixture::~AddFixture()
{
}

void AddFixture::init()
{
	fillTree();

	m_ok->setEnabled(false);
}

/*****************************************************************************
 * Fillers
 *****************************************************************************/

void AddFixture::fillTree()
{
	QPtrListIterator <QLCFixtureDef> it(*_app->fixtureDefList());
	QLCFixtureDef* fixtureDef = NULL;
	QListViewItem* parent = NULL;
	QListViewItem* node = NULL;
	QString str;

	/* Clear the tree of any previous data */
	m_tree->clear();

	/* Add all known fixture definitions. */
	while ( (fixtureDef = *it) != NULL )
	{
		parent = NULL;

		/* Search for an existing manufacturer parent node from
		   the tree. If such is found, it will be used as the
		   parent for the current fixture. If not, the manufacturer
		   will be added as a new parent and then used for the
		   current fixture. */
		for (node = m_tree->firstChild(); node != NULL;
		     node = node->nextSibling())
		{
			if (node->text(KColumnName) == fixtureDef->manufacturer())
			{
				parent = node;
				break;
			}
		}

		/* If an existing manufacturer parent node was not found,
		   we must create one. Otherwise we use the found node
		   as the parent for the new item. */
		if (parent == NULL)
			parent = new QListViewItem(m_tree,
						   fixtureDef->manufacturer());

		/* Create a new fixture node, under the parent node */
		node = new QListViewItem(parent);
		node->setText(KColumnName, fixtureDef->model());
		node->setText(KColumnType, fixtureDef->type());

		/* Store the fixture pointer into the tree */
		str.sprintf("%d", (unsigned long) fixtureDef);
		node->setText(KColumnPointer, str);

		++it;
	}

	/* Create a node & parent for generic dimmers */
	parent = new QListViewItem(m_tree);
	parent->setText(KColumnName, KXMLFixtureGeneric);
	node = new QListViewItem(parent);
	node->setText(KColumnName, KXMLFixtureGeneric);
	node->setText(KColumnType, KXMLFixtureGeneric);
	node->setText(KColumnPointer, "0");
}

void AddFixture::fillModeCombo(const QString& text)
{
	QLCFixtureMode* mode = NULL;

	m_modeCombo->clear();

	if (m_fixtureDef == NULL)
	{
		m_modeCombo->setEnabled(false);
		m_modeCombo->insertItem(text);
		m_modeCombo->setCurrentItem(0);
	}
	else
	{
		m_modeCombo->setEnabled(true);

		QPtrListIterator <QLCFixtureMode> it(*m_fixtureDef->modes());
		while ( (mode = *it) != NULL )
		{
			m_modeCombo->insertItem(mode->name());
			++it;
		}

		/* Select the first mode by default */
		m_modeCombo->setCurrentItem(0);
		slotModeActivated(m_modeCombo->currentText());
	}
}

/*****************************************************************************
 * Slots
 *****************************************************************************/

void AddFixture::slotChannelsChanged(int value)
{
	m_addressSpin->setRange(1, 513 - value);
}

void AddFixture::slotModeActivated(const QString& modeName)
{
	QLCFixtureMode* mode = NULL;

	if (m_fixtureDef == NULL)
		return;

	m_mode = m_fixtureDef->mode(modeName);
	if (m_mode == NULL)
	{
		slotSelectionChanged(NULL);
		return;
	}

	m_channelsSpin->setValue(m_mode->channels());
}

void AddFixture::slotSelectionChanged(QListViewItem* item)
{
	QString manuf;
	QString model;

	/* If there is no valid selection, i.e. the user has selected
	   only the manufacturer, don't let the user press OK */
	if (item == NULL || item->parent() == NULL)
	{
		/* Reset the selected fixture pointer */
		m_fixtureDef = NULL;
		fillModeCombo();
		
		if (m_nameEdit->isModified() == false)
			m_nameEdit->setText(QString::null);
		m_nameEdit->setEnabled(false);
		
		m_channelsSpin->setValue(0);
		m_addressSpin->setEnabled(false);
		m_universeSpin->setEnabled(false);
		m_multipleNumberSpin->setEnabled(false);
		m_addressGapSpin->setEnabled(false);
		m_channelsSpin->setEnabled(false);
		
		m_ok->setEnabled(false);
	}
	else
	{
		if (item->text(KColumnName) == KXMLFixtureGeneric &&
		    item->text(KColumnType) == KXMLFixtureGeneric)
		{
			m_fixtureDef = NULL;
			fillModeCombo(KXMLFixtureGeneric);
			m_channelsSpin->setEnabled(true);

			/* Set the model name as the fixture's friendly name ONLY
			   if the user hasn't modified the friendly name field. */	
			if (m_nameEdit->isModified() == false)
				m_nameEdit->setText(KXMLFixtureGeneric);
			m_nameEdit->setEnabled(true);
		}
		else
		{
			/* Get the selected fixture pointer */
			m_fixtureDef = (QLCFixtureDef*) 
				item->text(KColumnPointer).toULong();
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
		m_multipleNumberSpin->setEnabled(true);
		m_addressGapSpin->setEnabled(true);
		
		m_ok->setEnabled(true);
	}
}

void AddFixture::slotTreeDoubleClicked(QListViewItem* item)
{
	slotSelectionChanged(item);

	if (item != NULL && item->parent() != NULL)
		slotOKClicked();
}

void AddFixture::slotNameChanged(const QString &text)
{
	m_nameValue = text;
	if (text.length() == 0)
	{
		/* If the user clears the text in the name field,
		   start substituting the name with the model again. */
		m_nameEdit->clearModified();
	}
}

void AddFixture::slotOKClicked()
{
	m_addressValue = m_addressSpin->value() - 1;
	m_universeValue = m_universeSpin->value() - 1;
	
	m_multipleNumberValue = m_multipleNumberSpin->value();
	m_addressGapValue = m_addressGapSpin->value();

	m_channelsValue = m_channelsSpin->value();
	
	accept();
}

void AddFixture::slotCancelClicked()
{
	reject();
}
