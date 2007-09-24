/*
  Q Light Controller
  vcsliderproperties.h

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

#ifndef VCSLIDERPROPERTIES_H
#define VCSLIDERPROPERTIES_H

#include "uic_vcsliderproperties.h"
#include "common/types.h"

class VCSlider;
class Fixture;
class QLCChannel;
class QLCCapability;
class QCheckListItem;
class QListViewItem;

class VCSliderProperties : public UI_VCSliderProperties
{
	Q_OBJECT

public:
	VCSliderProperties(QWidget* parent, VCSlider* slider);
	~VCSliderProperties();

	void init();

	/*********************************************************************
	 * Bus page
	 *********************************************************************/
protected:
	/** Fill all buses to the combo on bus page */
	void fillBusCombo();

protected slots:
	/** Callback for bus low limit spin value changes */
	void slotBusLowLimitSpinChanged(int value);

	/** Callback for bus high limit spin value changes */
	void slotBusHighLimitSpinChanged(int value);

	/*********************************************************************
	 * Level page
	 *********************************************************************/
protected:
	/** Update fixtures to the listview on the level page */
	void levelUpdateFixtures();

	/** Update a fixture node in the listview on the level page */
	void levelUpdateFixtureNode(t_fixture_id id);

	/** Get a fixture node from the listview on the level page */
	QCheckListItem* levelFixtureNode(t_fixture_id id);

	/** Update fixture channels to the listview on the level page */
	void levelUpdateChannels(QCheckListItem* parent, Fixture* fxi);

	/** Update a fixture channel node to the listview on the level page */
	void levelUpdateChannelNode(QCheckListItem* parent,
				    Fixture* fxi,
				    t_channel ch);

	/** Update a channel's capabilities */
	void levelUpdateCapabilities(QCheckListItem* parent,
				     QLCChannel* channel);

	/** Update a channel's capability node */
	void levelUpdateCapabilityNode(QCheckListItem* parent,
				       QLCCapability* cap);

	/** Get a fixture channel node from the listview on the level page */
	QCheckListItem* levelChannelNode(QCheckListItem* parent, t_channel ch);

	/** 
	 * Select all channels matching the given group name from the listview
	 * on the level page
	 */
	void levelSelectChannelsByGroup(QString group);

protected slots:
	/** Callback for All button clicks */
	void slotLevelAllClicked();

	/** Callback for None button clicks */
	void slotLevelNoneClicked();

	/** Callback for Invert button clicks */
	void slotLevelInvertClicked();

	/** Callback for low level limit spin value changes */
	void slotLevelLowSpinChanged(int value);

	/** Callback for high level limit spin value changes */
	void slotLevelHighSpinChanged(int value);

	/** Callback for "set limits by capability" button clicks */
	void slotLevelCapabilityButtonClicked();

	/** Callback for level list item clicks */
	void slotLevelListClicked(QListViewItem* item);

	/** Callback for "channel selection by group" button clicks */
	void slotLevelByGroupButtonClicked();
	
	/*********************************************************************
	 * OK & Cancel
	 *********************************************************************/
protected slots:
	/** Callback for OK button clicks */
	void accept();

protected:
	/** The slider, whose properties are being edited */
	VCSlider* m_slider;
};

#endif
