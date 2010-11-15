/*
  Q Light Controller
  vcpropertieseditor.cpp

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

#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcfile.h"

#include "vcpropertieseditor.h"
#include "selectinputchannel.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "vcframe.h"
#include "app.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCPropertiesEditor::VCPropertiesEditor(QWidget* parent,
                                       const VCProperties& properties)
    : QDialog(parent)
{
    setupUi(this);

    m_properties = properties;

    /* Layout page */
    m_gridGroup->setChecked(properties.isGridEnabled());
    m_gridXSpin->setValue(properties.gridX());
    m_gridYSpin->setValue(properties.gridY());
    m_grabKeyboardCheck->setChecked(properties.isGrabKeyboard());
    m_keyRepeatOffCheck->setChecked(properties.isKeyRepeatOff());

    /* Fade page */
    m_fadeLowSpin->setValue(properties.fadeLowLimit());
    m_fadeHighSpin->setValue(properties.fadeHighLimit());
    updateFadeInputSource();

    /* Hold page */
    m_holdLowSpin->setValue(properties.holdLowLimit());
    m_holdHighSpin->setValue(properties.holdHighLimit());
    updateHoldInputSource();

    /* Grand Master page */
    switch (properties.grandMasterChannelMode())
    {
    default:
    case UniverseArray::GMIntensity:
        m_gmIntensityRadio->setChecked(true);
        break;
    case UniverseArray::GMAllChannels:
        m_gmAllChannelsRadio->setChecked(true);
        break;
    }

    switch (properties.grandMasterValueMode())
    {
    default:
    case UniverseArray::GMReduce:
        m_gmReduceRadio->setChecked(true);
        break;
    case UniverseArray::GMLimit:
        m_gmLimitRadio->setChecked(true);
        break;
    }

    updateGrandMasterInputSource();

    /* Blackout page*/
    updateBlackoutInputSource();
}

VCPropertiesEditor::~VCPropertiesEditor()
{
}

VCProperties VCPropertiesEditor::properties() const
{
    return m_properties;
}

/*****************************************************************************
 * Layout page
 *****************************************************************************/

void VCPropertiesEditor::slotGrabKeyboardClicked()
{
    m_properties.setGrabKeyboard(m_grabKeyboardCheck->isChecked());
}

void VCPropertiesEditor::slotKeyRepeatOffClicked()
{
    m_properties.setKeyRepeatOff(m_keyRepeatOffCheck->isChecked());
}

void VCPropertiesEditor::slotGridClicked()
{
    m_properties.setGridEnabled(m_gridGroup->isChecked());
}

void VCPropertiesEditor::slotGridXChanged(int value)
{
    m_properties.setGridX(value);
}

void VCPropertiesEditor::slotGridYChanged(int value)
{
    m_properties.setGridY(value);
}

/*****************************************************************************
 * Fade slider page
 *****************************************************************************/

void VCPropertiesEditor::slotFadeLimitsChanged()
{
    m_properties.setFadeLimits(m_fadeLowSpin->value(),
                               m_fadeHighSpin->value());
}

void VCPropertiesEditor::slotAutoDetectFadeInputToggled(bool checked)
{
    if (checked == true)
    {
        if (m_autoDetectHoldInputButton->isChecked() == true)
            m_autoDetectHoldInputButton->toggle();

        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotFadeInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotFadeInputValueChanged(quint32,quint32)));
    }
}

void VCPropertiesEditor::slotFadeInputValueChanged(quint32 universe,
                                                   quint32 channel)
{
    m_properties.setFadeInputSource(universe, channel);
    updateFadeInputSource();
}

void VCPropertiesEditor::slotChooseFadeInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_properties.setFadeInputSource(sic.universe(), sic.channel());
        updateFadeInputSource();
    }
}

void VCPropertiesEditor::updateFadeInputSource()
{
    QString uniName;
    QString chName;

    if (inputSourceNames(m_properties.fadeInputUniverse(),
                         m_properties.fadeInputChannel(),
                         uniName, chName) == true)
    {
        /* Display the gathered information */
        m_fadeInputUniverseEdit->setText(uniName);
        m_fadeInputChannelEdit->setText(chName);
    }
    else
    {
        m_fadeInputUniverseEdit->setText(KInputNone);
        m_fadeInputChannelEdit->setText(KInputNone);
    }
}

/*****************************************************************************
 * Hold slider page
 *****************************************************************************/

void VCPropertiesEditor::slotHoldLimitsChanged()
{
    m_properties.setHoldLimits(m_holdLowSpin->value(),
                               m_holdHighSpin->value());
}

void VCPropertiesEditor::slotAutoDetectHoldInputToggled(bool checked)
{
    if (checked == true)
    {
        if (m_autoDetectFadeInputButton->isChecked() == true)
            m_autoDetectFadeInputButton->toggle();

        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotHoldInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotHoldInputValueChanged(quint32,quint32)));
    }
}

void VCPropertiesEditor::slotHoldInputValueChanged(quint32 universe,
                                                   quint32 channel)
{
    m_properties.setHoldInputSource(universe, channel);
    updateHoldInputSource();
}

void VCPropertiesEditor::slotChooseHoldInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_properties.setHoldInputSource(sic.universe(), sic.channel());
        updateHoldInputSource();
    }
}

void VCPropertiesEditor::updateHoldInputSource()
{
    QString uniName;
    QString chName;

    if (inputSourceNames(m_properties.holdInputUniverse(),
                         m_properties.holdInputChannel(),
                         uniName, chName) == true)
    {
        /* Display the gathered information */
        m_holdInputUniverseEdit->setText(uniName);
        m_holdInputChannelEdit->setText(chName);
    }
    else
    {
        m_holdInputUniverseEdit->setText(KInputNone);
        m_holdInputChannelEdit->setText(KInputNone);
    }
}

/*****************************************************************************
 * Grand Master page
 *****************************************************************************/

void VCPropertiesEditor::slotGrandMasterIntensityToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterChannelMode(UniverseArray::GMIntensity);
    else
        m_properties.setGrandMasterChannelMode(UniverseArray::GMAllChannels);
}

void VCPropertiesEditor::slotGrandMasterReduceToggled(bool checked)
{
    if (checked == true)
        m_properties.setGrandMasterValueMode(UniverseArray::GMReduce);
    else
        m_properties.setGrandMasterValueMode(UniverseArray::GMLimit);
}

void VCPropertiesEditor::slotAutoDetectGrandMasterInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotGrandMasterInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotGrandMasterInputValueChanged(quint32,quint32)));
    }
}

void VCPropertiesEditor::slotGrandMasterInputValueChanged(quint32 universe,
                                                          quint32 channel)
{
    m_properties.setGrandMasterInputSource(universe, channel);
    updateGrandMasterInputSource();
}

void VCPropertiesEditor::slotChooseGrandMasterInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_properties.setGrandMasterInputSource(sic.universe(), sic.channel());
        updateGrandMasterInputSource();
    }
}

void VCPropertiesEditor::updateGrandMasterInputSource()
{
    QString uniName;
    QString chName;

    if (inputSourceNames(m_properties.grandMasterInputUniverse(),
                         m_properties.grandMasterInputChannel(),
                         uniName, chName) == true)
    {
        /* Display the gathered information */
        m_gmInputUniverseEdit->setText(uniName);
        m_gmInputChannelEdit->setText(chName);
    }
    else
    {
        m_gmInputUniverseEdit->setText(KInputNone);
        m_gmInputChannelEdit->setText(KInputNone);
    }
}

/*****************************************************************************
 * Blackout page
 *****************************************************************************/

void VCPropertiesEditor::slotAutoDetectBlackoutInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotBlackoutInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotBlackoutInputValueChanged(quint32,quint32)));
    }
}

void VCPropertiesEditor::slotBlackoutInputValueChanged(quint32 universe,
                                                       quint32 channel)
{
    m_properties.setBlackoutInputSource(universe, channel);
    updateBlackoutInputSource();
}

void VCPropertiesEditor::slotChooseBlackoutInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_properties.setBlackoutInputSource(sic.universe(), sic.channel());
        updateBlackoutInputSource();
    }
}

void VCPropertiesEditor::updateBlackoutInputSource()
{
    QString uniName;
    QString chName;

    if (inputSourceNames(m_properties.blackoutInputUniverse(),
                         m_properties.blackoutInputChannel(),
                         uniName, chName) == true)
    {
        /* Display the gathered information */
        m_blackoutInputUniverseEdit->setText(uniName);
        m_blackoutInputChannelEdit->setText(chName);
    }
    else
    {
        m_blackoutInputUniverseEdit->setText(KInputNone);
        m_blackoutInputChannelEdit->setText(KInputNone);
    }
}

/*****************************************************************************
 * Input Source helper
 *****************************************************************************/

bool VCPropertiesEditor::inputSourceNames(quint32 universe, quint32 channel,
                                          QString& uniName, QString& chName) const
{
    if (universe == InputMap::invalidUniverse() || channel == KInputChannelInvalid)
    {
        /* Nothing selected for input universe and/or channel */
        return false;
    }

    InputPatch* patch = _app->inputMap()->patch(universe);
    if (patch == NULL || patch->plugin() == NULL)
    {
        /* There is no patch for the given universe */
        return false;
    }

    QLCInputProfile* profile = patch->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number.
           Boring. */
        uniName = patch->plugin()->name();
        chName = tr("%1: Unknown").arg(channel + 1);
    }
    else
    {
        QLCInputChannel* ich;
        QString name;

        /* Display profile name for universe */
        uniName = QString("%1: %2").arg(universe + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ich = profile->channel(channel);
        if (ich != NULL)
            name = ich->name();
        else
            name = tr("Unknown");

        /* Display channel name */
        chName = QString("%1: %2").arg(channel + 1).arg(name);
    }

    return true;
}
