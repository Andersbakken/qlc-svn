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

#include "common/types.h"
#include "uic_addfixture.h"

class QString;
class QLCFixtureDef;
class QLCFixtureMode;

class AddFixture : public UI_AddFixture
{
	Q_OBJECT

 public: 
	/** Constructor */
	AddFixture(QWidget *parent);

	/** Destructor */
	~AddFixture();

	/** Initialize view components */
	void init();

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
	int multipleNumber() const { return m_multipleNumberValue; }

	/** Get the number of channels to leave between two fixtures */
	t_channel addressGap() const { return m_addressGapValue; }

	/** Get the number of channels to use (ONLY for generic dimmers) */
	t_channel channels() const { return m_channelsValue; }
  
 protected:
	/** Fill all known fixture definitions to the tree view */
	void fillTree();

	/** Fill all modes of the current fixture to the mode combo */
	void fillModeCombo(const QString& text = QString::null);

	/** Get the currently selected fixture pointer */
	QLCFixtureDef* selectedFixtureDef();

 protected slots:
	 /** Callback for channels spin value changes */
	 void slotChannelsChanged(int value);

	 /** Callback for mode selection changes */
	 void slotModeActivated(const QString& modeName);

	 /**
	  * Callback for tree view selection changes
	  */
	void slotSelectionChanged(QListViewItem* item);

	/**
	 * Callback for tree double clicks (same as select + OK)
	 */
	void slotTreeDoubleClicked(QListViewItem* item);

	/**
	 * Callback for friendly name editing
	 */
	void slotNameChanged(const QString &text);

	/**
	 * Callback for OK button clicks
	 */
	void slotOKClicked();

	/**
	 * Callback for Cancel button clicks
	 */
	void slotCancelClicked();

 protected:
	QLCFixtureDef* m_fixtureDef;
	QLCFixtureMode* m_mode;

	QString m_nameValue;

	t_channel m_addressValue;
	t_channel m_universeValue;
	int m_multipleNumberValue;
	t_channel m_addressGapValue;
	t_channel m_channelsValue;
};

#endif
