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

#include <QDialog>

#include "ui_vcsliderproperties.h"
#include "qlctypes.h"

class QTreeWidgetItem;

class QLCCapability;
class QLCChannel;
class VCSlider;
class Fixture;

class VCSliderProperties : public QDialog, public Ui_VCSliderProperties
{
    Q_OBJECT

public:
    VCSliderProperties(QWidget* parent, VCSlider* slider);
    ~VCSliderProperties();

private:
    Q_DISABLE_COPY(VCSliderProperties)

    /*********************************************************************
     * General page
     *********************************************************************/
protected slots:
    void slotModeBusClicked();
    void slotModeLevelClicked();
    void slotModeSubmasterClicked();

    void slotAutoDetectInputToggled(bool checked);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseInputClicked();

protected:
    void updateInputSource();

protected:
    quint32 m_inputUniverse;
    quint32 m_inputChannel;
    int m_sliderMode;

    /*********************************************************************
     * Bus page
     *********************************************************************/
protected:
    /** Fill all buses to the combo on bus page */
    void fillBusCombo();

protected slots:
    /** Callback for bus combo item activations */
    void slotBusComboActivated(int item);

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
    QTreeWidgetItem* levelFixtureNode(t_fixture_id id);

    /** Update fixture channels to the listview on the level page */
    void levelUpdateChannels(QTreeWidgetItem* parent, Fixture* fxi);

    /** Update a fixture channel node to the listview on the level page */
    void levelUpdateChannelNode(QTreeWidgetItem* parent,
                                Fixture* fxi,
                                quint32 ch);

    /** Update a channel's capabilities */
    void levelUpdateCapabilities(QTreeWidgetItem* parent,
                                 const QLCChannel* channel);

    /** Update a channel's capability node */
    void levelUpdateCapabilityNode(QTreeWidgetItem* parent,
                                   QLCCapability* cap);

    /** Get a fixture channel node from the listview on the level page */
    QTreeWidgetItem* levelChannelNode(QTreeWidgetItem* parent, quint32 ch);

    /** Update channel selections from the slider's level channel list */
    void levelUpdateChannelSelections();

    /**
     * Select all channels matching the given group name from the listview
     * on the level page
     */
    void levelSelectChannelsByGroup(QString group);

protected slots:
    /** Callback for low level limit spin value changes */
    void slotLevelLowSpinChanged(int value);

    /** Callback for high level limit spin value changes */
    void slotLevelHighSpinChanged(int value);

    /** Callback for "set limits by capability" button clicks */
    void slotLevelCapabilityButtonClicked();

    /** Callback for level list item clicks */
    void slotLevelListClicked(QTreeWidgetItem* item);

    /** Callback for All button clicks */
    void slotLevelAllClicked();

    /** Callback for None button clicks */
    void slotLevelNoneClicked();

    /** Callback for Invert button clicks */
    void slotLevelInvertClicked();

    /** Callback for "channel selection by group" button clicks */
    void slotLevelByGroupClicked();

    /*********************************************************************
     * OK & Cancel
     *********************************************************************/
protected:
    /** Store selected channels to the slider */
    void storeLevelChannels();

protected slots:
    /** Callback for OK button clicks */
    void accept();

protected:
    /** The slider, whose properties are being edited */
    VCSlider* m_slider;
};

#endif
