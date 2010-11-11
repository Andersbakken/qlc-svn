/*
  Q Light Controller
  vcsliderproperties.cpp

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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QRadioButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#include "vcsliderproperties.h"
#include "selectinputchannel.h"
#include "inputpatch.h"
#include "vcslider.h"
#include "inputmap.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnName  0
#define KColumnType  1
#define KColumnRange 2
#define KColumnID    3

VCSliderProperties::VCSliderProperties(QWidget* parent, VCSlider* slider)
        : QDialog(parent)
{
    Q_ASSERT(slider != NULL);
    m_slider = slider;

    setupUi(this);

    /* Bus page connections */
    connect(m_switchToBusModeButton, SIGNAL(clicked()),
            this, SLOT(slotModeBusClicked()));

    /* Level page connections */
    connect(m_levelLowLimitSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelLowSpinChanged(int)));
    connect(m_levelHighLimitSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotLevelHighSpinChanged(int)));
    connect(m_levelCapabilityButton, SIGNAL(clicked()),
            this, SLOT(slotLevelCapabilityButtonClicked()));
    connect(m_levelList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotLevelListClicked(QTreeWidgetItem*)));
    connect(m_levelAllButton, SIGNAL(clicked()),
            this, SLOT(slotLevelAllClicked()));
    connect(m_levelNoneButton, SIGNAL(clicked()),
            this, SLOT(slotLevelNoneClicked()));
    connect(m_levelInvertButton, SIGNAL(clicked()),
            this, SLOT(slotLevelInvertClicked()));
    connect(m_levelByGroupButton, SIGNAL(clicked()),
            this, SLOT(slotLevelByGroupClicked()));
    connect(m_switchToLevelModeButton, SIGNAL(clicked()),
            this, SLOT(slotModeLevelClicked()));

    /*********************************************************************
     * General page
     *********************************************************************/

    /* Name */
    m_nameEdit->setText(m_slider->caption());

    /* Slider mode */
    m_sliderMode = m_slider->sliderMode();
    switch (m_sliderMode)
    {
    default:
    case VCSlider::Bus:
        slotModeBusClicked();
        break;
    case VCSlider::Level:
        slotModeLevelClicked();
        break;
    case VCSlider::Submaster:
        slotModeLevelClicked();
        break;
    }

    /* Slider movement (Qt understands inverted appearance vice versa) */
    if (m_slider->invertedAppearance() == true)
        m_sliderMovementInvertedRadio->setChecked(true);
    else
        m_sliderMovementNormalRadio->setChecked(true);

    /* Value display style */
    switch (m_slider->valueDisplayStyle())
    {
    default:
    case VCSlider::ExactValue:
        m_valueExactRadio->setChecked(true);
        break;
    case VCSlider::PercentageValue:
        m_valuePercentageRadio->setChecked(true);
        break;
    }

    /********************************************************************
     * External input
     ********************************************************************/
    m_inputUniverse = m_slider->inputUniverse();
    m_inputChannel = m_slider->inputChannel();
    updateInputSource();

    connect(m_autoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoDetectInputToggled(bool)));
    connect(m_chooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotChooseInputClicked()));

    /*********************************************************************
     * Bus page
     *********************************************************************/

    /* Bus combo contents */
    fillBusCombo();

    /* Bus value limit spins */
    m_busLowLimitSpin->setValue(m_slider->busLowLimit());
    m_busHighLimitSpin->setValue(m_slider->busHighLimit());

    /*********************************************************************
     * Level page
     *********************************************************************/

    /* Level limit spins */
    m_levelLowLimitSpin->setValue(m_slider->levelLowLimit());
    m_levelHighLimitSpin->setValue(m_slider->levelHighLimit());

    /* Tree widget columns */
    m_levelList->header()->setResizeMode(QHeaderView::ResizeToContents);

    /* Tree widget contents */
    levelUpdateFixtures();
    levelUpdateChannelSelections();
}

VCSliderProperties::~VCSliderProperties()
{
}

/*****************************************************************************
 * General page
 *****************************************************************************/

void VCSliderProperties::slotModeBusClicked()
{
    m_sliderMode = VCSlider::Bus;

    m_nameEdit->setEnabled(false);
    slotBusComboActivated(m_busCombo->currentIndex());
    m_sliderMovementInvertedRadio->setChecked(true);

    m_busValueRangeGroup->show();
    m_busGroup->show();

    m_levelValueRangeGroup->hide();
    m_levelList->hide();
    m_levelAllButton->hide();
    m_levelNoneButton->hide();
    m_levelInvertButton->hide();
    m_levelByGroupButton->hide();

    m_switchToBusModeButton->hide();
    m_switchToLevelModeButton->show();
    m_busSpacer->changeSize(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void VCSliderProperties::slotModeLevelClicked()
{
    m_sliderMode = VCSlider::Level;

    m_nameEdit->setEnabled(true);
    m_busValueRangeGroup->hide();
    m_busGroup->hide();
    m_sliderMovementNormalRadio->setChecked(true);

    m_levelValueRangeGroup->show();
    m_levelList->show();
    m_levelAllButton->show();
    m_levelNoneButton->show();
    m_levelInvertButton->show();
    m_levelByGroupButton->show();

    m_switchToLevelModeButton->hide();
    m_switchToBusModeButton->show();
    m_busSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void VCSliderProperties::slotModeSubmasterClicked()
{
    m_sliderMode = VCSlider::Submaster;
}

void VCSliderProperties::slotAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotInputValueChanged(quint32,quint32)));
    }
}

void VCSliderProperties::slotInputValueChanged(quint32 universe,
                                               quint32 channel)
{
    m_inputUniverse = universe;
    m_inputChannel = channel;
    updateInputSource();
}

void VCSliderProperties::slotChooseInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_inputUniverse = sic.universe();
        m_inputChannel = sic.channel();

        updateInputSource();
    }
}

void VCSliderProperties::updateInputSource()
{
    QLCInputProfile* profile;
    InputPatch* patch;
    QString uniName;
    QString chName;

    if (m_inputUniverse == InputMap::invalidUniverse() ||
            m_inputChannel == KInputChannelInvalid)
    {
        /* Nothing selected for input universe and/or channel */
        uniName = KInputNone;
        chName = KInputNone;
    }
    else
    {
        patch = _app->inputMap()->patch(m_inputUniverse);
        if (patch == NULL || patch->plugin() == NULL)
        {
            /* There is no patch for the given universe */
            uniName = KInputNone;
            chName = KInputNone;
        }
        else
        {
            profile = patch->profile();
            if (profile == NULL)
            {
                /* There is no profile. Display plugin
                   name and channel number. Boring. */
                uniName = patch->plugin()->name();
                chName = tr("%1: Unknown")
                         .arg(m_inputChannel + 1);
            }
            else
            {
                QLCInputChannel* ich;
                QString name;

                /* Display profile name for universe */
                uniName = QString("%1: %2")
                          .arg(m_inputUniverse + 1)
                          .arg(profile->name());

                /* User can input the channel number by hand,
                   so put something rational to the channel
                   name in those cases as well. */
                ich = profile->channel(m_inputChannel);
                if (ich != NULL)
                    name = ich->name();
                else
                    name = tr("Unknown");

                /* Display channel name */
                chName = QString("%1: %2")
                         .arg(m_inputChannel + 1).arg(name);
            }
        }
    }

    /* Display the gathered information */
    m_inputUniverseEdit->setText(uniName);
    m_inputChannelEdit->setText(chName);
}

/*****************************************************************************
 * Bus page
 *****************************************************************************/

void VCSliderProperties::fillBusCombo()
{
    m_busCombo->clear();
    m_busCombo->addItems(Bus::instance()->idNames());
    m_busCombo->setCurrentIndex(m_slider->bus());
}

void VCSliderProperties::slotBusComboActivated(int item)
{
    m_nameEdit->setText(Bus::instance()->name(item));
}

void VCSliderProperties::slotBusLowLimitSpinChanged(int value)
{
    /* Don't allow the low limit to get higher than the high limit */
    if (value >= m_busHighLimitSpin->value())
        m_busHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotBusHighLimitSpinChanged(int value)
{
    /* Don't allow the high limit to get lower than the low limit */
    if (value <= m_busLowLimitSpin->value())
        m_busLowLimitSpin->setValue(value - 1);
}

/*****************************************************************************
 * Level page
 *****************************************************************************/

void VCSliderProperties::levelUpdateFixtures()
{
    t_fixture_id id = 0;
    Fixture* fxi = NULL;

    for (id = 0; id < KFixtureArraySize; id++)
    {
        fxi = _app->doc()->fixture(id);
        if (fxi == NULL)
            continue;

        levelUpdateFixtureNode(id);
    }
}

void VCSliderProperties::levelUpdateFixtureNode(t_fixture_id id)
{
    QTreeWidgetItem* item;
    Fixture* fxi;
    QString str;

    fxi = _app->doc()->fixture(id);
    Q_ASSERT(fxi != NULL);

    item = levelFixtureNode(id);
    if (item == NULL)
    {
        item = new QTreeWidgetItem(m_levelList);
        item->setText(KColumnID, str.setNum(id));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable
                       | Qt::ItemIsTristate);
    }

    item->setText(KColumnName, fxi->name());
    item->setText(KColumnType, fxi->type());

    levelUpdateChannels(item, fxi);
}

QTreeWidgetItem* VCSliderProperties::levelFixtureNode(t_fixture_id id)
{
    QTreeWidgetItem* item;
    int i;

    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        item = m_levelList->topLevelItem(i);
        if (item->text(KColumnID).toInt() == id)
            return item;
    }

    return NULL;
}

void VCSliderProperties::levelUpdateChannels(QTreeWidgetItem* parent,
                                             Fixture* fxi)
{
    quint32 channels = 0;
    quint32 ch = 0;

    Q_ASSERT(parent != NULL);
    Q_ASSERT(fxi != NULL);

    channels = fxi->channels();
    for (ch = 0; ch < channels; ch++)
        levelUpdateChannelNode(parent, fxi, ch);
}

void VCSliderProperties::levelUpdateChannelNode(QTreeWidgetItem* parent,
                                                Fixture* fxi, quint32 ch)
{
    Q_ASSERT(parent != NULL);

    if (fxi == NULL)
        return;

    const QLCChannel* channel = fxi->channel(ch);
    if (channel == NULL)
        return;

    QTreeWidgetItem* item = levelChannelNode(parent, ch);
    if (item == NULL)
    {
        item = new QTreeWidgetItem(parent);
        item->setText(KColumnID, QString::number(ch));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(KColumnName, Qt::Unchecked);
    }

    item->setText(KColumnName, QString("%1:%2").arg(ch + 1)
                  .arg(channel->name()));
    item->setText(KColumnType, QLCChannel::groupToString(channel->group()));

    levelUpdateCapabilities(item, channel);
}

QTreeWidgetItem* VCSliderProperties::levelChannelNode(QTreeWidgetItem* parent,
                                                      quint32 ch)
{
    Q_ASSERT(parent != NULL);

    for (int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* item = parent->child(i);
        if (item->text(KColumnID).toUInt() == ch)
            return item;
    }

    return NULL;
}

void VCSliderProperties::levelUpdateCapabilities(QTreeWidgetItem* parent,
                                                 const QLCChannel* channel)
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(channel != NULL);

    QListIterator <QLCCapability*> it(channel->capabilities());
    while (it.hasNext() == true)
        levelUpdateCapabilityNode(parent, it.next());
}

void VCSliderProperties::levelUpdateCapabilityNode(QTreeWidgetItem* parent,
        QLCCapability* cap)
{
    QTreeWidgetItem* item;
    QString str;

    Q_ASSERT(parent != NULL);
    Q_ASSERT(cap != NULL);

    item = new QTreeWidgetItem(parent);
    item->setText(KColumnName, cap->name());
    item->setText(KColumnRange, str.sprintf("%.3d - %.3d",
                                            cap->min(), cap->max()));
    item->setFlags(item->flags() & (~Qt::ItemIsUserCheckable));
}

void VCSliderProperties::levelUpdateChannelSelections()
{
    /* Check all items that are present in the slider's list of
       controlled channels. We don't need to set other items off,
       because this function is run only during init when everything
       is off. */
    QListIterator <VCSlider::LevelChannel> it(m_slider->m_levelChannels);
    while (it.hasNext() == true)
    {
        VCSlider::LevelChannel lch(it.next());

        QTreeWidgetItem* fxiNode = levelFixtureNode(lch.fixture);
        if (fxiNode == NULL)
            continue;

        QTreeWidgetItem* chNode = levelChannelNode(fxiNode, lch.channel);
        if (chNode == NULL)
            continue;

        chNode->setCheckState(KColumnName, Qt::Checked);
    }
}

void VCSliderProperties::levelSelectChannelsByGroup(QString group)
{
    QTreeWidgetItem* fxi_item;
    QTreeWidgetItem* ch_item;
    int i;
    int j;

    /* Go thru only channel items. Fixture items get (partially) selected
       according to their children's state */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        for (j = 0; j < fxi_item->childCount(); j++)
        {
            ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->text(KColumnType) == group)
                ch_item->setCheckState(KColumnName,
                                       Qt::Checked);
        }
    }
}

void VCSliderProperties::slotLevelLowSpinChanged(int value)
{
    if (value >= m_levelHighLimitSpin->value())
        m_levelHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotLevelHighSpinChanged(int value)
{
    if (value <= m_levelLowLimitSpin->value())
        m_levelLowLimitSpin->setValue(value - 1);
}

void VCSliderProperties::slotLevelCapabilityButtonClicked()
{
    QTreeWidgetItem* item;
    QStringList list;

    item = m_levelList->currentItem();
    if (item == NULL || item->parent() == NULL ||
            item->parent()->parent() == NULL)
        return;

    list = item->text(KColumnRange).split("-");
    Q_ASSERT(list.size() == 2);

    m_levelLowLimitSpin->setValue(list[0].toInt());
    m_levelHighLimitSpin->setValue(list[1].toInt());
}

void VCSliderProperties::slotLevelListClicked(QTreeWidgetItem* item)
{
    /* Enable the capability button if a capability has been selected */
    if (item != NULL && item->parent() != NULL &&
            item->parent()->parent() != NULL)
    {
        m_levelCapabilityButton->setEnabled(true);
    }
    else
    {
        m_levelCapabilityButton->setEnabled(false);
    }
}

void VCSliderProperties::slotLevelAllClicked()
{
    QTreeWidgetItem* fxi_item;
    int i;

    /* Set all fixture items selected, their children should get selected
       as well because the fixture items are Controller items. */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        fxi_item->setCheckState(KColumnName, Qt::Checked);
    }
}

void VCSliderProperties::slotLevelNoneClicked()
{
    QTreeWidgetItem* fxi_item;
    int i;

    /* Set all fixture items unselected, their children should get unselected
       as well because the fixture items are Controller items. */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        fxi_item->setCheckState(KColumnName, Qt::Unchecked);
    }
}

void VCSliderProperties::slotLevelInvertClicked()
{
    QTreeWidgetItem* fxi_item;
    QTreeWidgetItem* ch_item;
    int i;
    int j;

    /* Go thru only channel items. Fixture items get (partially) selected
       according to their children's state */
    for (i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        for (j = 0; j < fxi_item->childCount(); j++)
        {
            ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->checkState(KColumnName) == Qt::Checked)
                ch_item->setCheckState(KColumnName,
                                       Qt::Unchecked);
            else
                ch_item->setCheckState(KColumnName,
                                       Qt::Checked);
        }
    }
}

void VCSliderProperties::slotLevelByGroupClicked()
{
    bool ok = false;
    QString group;

    group = QInputDialog::getItem(this,
                                  "Select channels by group",
                                  "Select a channel group",
                                  QLCChannel::groupList(), 0,
                                  false, &ok);

    if (ok == true)
        levelSelectChannelsByGroup(group);
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void VCSliderProperties::storeLevelChannels()
{
    /* Clear all channels from the slider first */
    m_slider->clearLevelChannels();

    /* Go thru all fixtures and their channels, add checked channels */
    for (int i = 0; i < m_levelList->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* fxi_item = m_levelList->topLevelItem(i);
        Q_ASSERT(fxi_item != NULL);

        t_fixture_id fxi_id = fxi_item->text(KColumnID).toUInt();

        for (int j = 0; j < fxi_item->childCount(); j++)
        {
            QTreeWidgetItem* ch_item = fxi_item->child(j);
            Q_ASSERT(ch_item != NULL);

            if (ch_item->checkState(KColumnName) == Qt::Checked)
            {
                quint32 ch_num = ch_item->text(KColumnID).toUInt();
                m_slider->addLevelChannel(fxi_id, ch_num);
            }
        }
    }
}

void VCSliderProperties::accept()
{
    /* Bus page */
    m_slider->setBus(m_busCombo->currentIndex());
    m_slider->setBusLowLimit(m_busLowLimitSpin->value());
    m_slider->setBusHighLimit(m_busHighLimitSpin->value());

    /* Level page */
    m_slider->setLevelLowLimit(m_levelLowLimitSpin->value());
    m_slider->setLevelHighLimit(m_levelHighLimitSpin->value());
    storeLevelChannels();

    /* Slider mode */
    m_slider->setSliderMode(VCSlider::SliderMode(m_sliderMode));
    if (m_sliderMode == VCSlider::Level)
        m_slider->setCaption(m_nameEdit->text());

    /* Value style */
    if (m_valueExactRadio->isChecked() == true)
        m_slider->setValueDisplayStyle(VCSlider::ExactValue);
    else
        m_slider->setValueDisplayStyle(VCSlider::PercentageValue);

    /* Slider movement */
    if (m_sliderMovementNormalRadio->isChecked() == true)
        m_slider->setInvertedAppearance(false);
    else
        m_slider->setInvertedAppearance(true);

    /* External input */
    m_slider->setInputSource(m_inputUniverse, m_inputChannel);

    /* Close dialog */
    QDialog::accept();
}
