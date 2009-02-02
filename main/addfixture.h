/*
  Q Light Controller
  addfixture.h

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

#ifndef ADDFIXTURE_H
#define ADDFIXTURE_H

#include <QWidget>
#include "common/qlctypes.h"
#include "ui_addfixture.h"

class QTreeWidgetItem;
class QString;
class QLCFixtureDef;
class QLCFixtureMode;

class AddFixture : public QDialog, public Ui_AddFixture
{
	Q_OBJECT

public:
	/** Constructor, pre-select $select, if not NULL */
	AddFixture(QWidget *parent, QLCFixtureDef* select = NULL);

	/** Destructor */
	~AddFixture();

private:
	Q_DISABLE_COPY(AddFixture)

	/*********************************************************************
	 * Value getters
	 *********************************************************************/
public:
	/** Get the selected QLCFixtureDef */
	QLCFixtureDef* fixtureDef() const { return m_fixtureDef; }

	/** Get the selected QLCFixtureMode */
	QLCFixtureMode* mode() const { return m_mode; }

	/** Get the assigned friendly name */
	QString name() const { return m_nameValue; }

	/** Get the assigned DMX address */
	t_channel address() const { return m_addressValue; }

	/** Get the assigned DMX universe */
	t_channel universe() const { return m_universeValue; }

	/** Get the number of similar fixtures to add */
	int amount() const { return m_amountValue; }

	/** Get the number of channels to leave between two fixtures */
	t_channel gap() const { return m_gapValue; }

	/** Get the number of channels to use (ONLY for generic dimmers) */
	t_channel channels() const { return m_channelsValue; }

protected:
	QLCFixtureDef* m_fixtureDef;
	QLCFixtureMode* m_mode;

	QString m_nameValue;

	t_channel m_addressValue;
	t_channel m_universeValue;
	int m_amountValue;
	t_channel m_gapValue;
	t_channel m_channelsValue;

	/*********************************************************************
	 * Helpers
	 *********************************************************************/
protected:
	/** Find a node that contains the given text */
	QTreeWidgetItem* findNode(const QString& text);

	/*********************************************************************
	 * Fillers
	 *********************************************************************/
protected:
	/** Fill all known fixture definitions to the tree view */
	void fillTree(QLCFixtureDef* select);

	/** Fill all modes of the current fixture to the mode combo */
	void fillModeCombo(const QString& text = QString::null);

	/** Find the next free address space for current fixture selection,
	    amount and address gap. Sets the address to address spin. */
	void findAddress();

	/*********************************************************************
	 * Slots
	 *********************************************************************/
protected slots:
	/** Callback for channels spin value changes */
	void slotChannelsChanged(int value);

	/** Callback for mode selection changes */
	void slotModeActivated(const QString& modeName);

	/** Callback for tree view selection changes */
	void slotSelectionChanged();

	/** Callback for tree double clicks (same as select + OK) */
	void slotTreeDoubleClicked(QTreeWidgetItem* item);

	/** Callback for friendly name editing */
	void slotNameEdited(const QString &text);

	/** Callback for fixture amount value changes */
	void slotAmountSpinChanged(int value);

	/** Callback for address gap value changes */
	void slotGapSpinChanged(int value);

	/** OK button pressed */
	void accept();
};

#endif
