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
#include <QByteArray>
#include <QSettings>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

#include "outputpatch.h"
#include "addfixture.h"
#include "outputmap.h"
#include "apputil.h"
#include "doc.h"
#include "app.h"

#define SETTINGS_GEOMETRY "addfixture/geometry"

#define KColumnName 0

AddFixture::AddFixture(QWidget* parent,
                       const QLCFixtureDefCache& fixtureDefCache,
                       const Doc& doc,
                       const OutputMap& outputMap,
                       const QString& selectManufacturer,
                       const QString& selectModel,
                       const QString& selectMode,
                       const QString& selectName,
                       int selectUniverse,
                       int selectAddress,
                       int selectChannels)
        : QDialog(parent),
        m_fixtureDefCache(fixtureDefCache),
        m_doc(doc),
        m_outputMap(outputMap)
{
    m_addressValue = 0;
    m_universeValue = 0;
    m_amountValue = 1;
    m_gapValue = 0;
    m_channelsValue = 1;
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
    connect(m_universeCombo, SIGNAL(activated(int)),
            this, SLOT(slotUniverseActivated(int)));
    connect(m_addressSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotAddressChanged(int)));
    connect(m_channelsSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotChannelsChanged(int)));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_gapSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotGapSpinChanged(int)));
    connect(m_amountSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotAmountSpinChanged(int)));

    /* Fill fixture definition tree */
    fillTree(selectManufacturer, selectModel);

    /* Fill universe combo with available universes */
    m_universeCombo->addItems(m_outputMap.universeNames());

    /* Simulate first selection and find the next free address */
    slotSelectionChanged();

    if (selectAddress == -1 && selectUniverse == -1)
    {
        slotUniverseActivated(0);
        findAddress();
    }
    else
    {
        m_universeCombo->setCurrentIndex(selectUniverse);
        slotUniverseActivated(selectUniverse);

        OutputPatch* op = m_outputMap.patch(selectUniverse);
        if (op != NULL && op->isDMXZeroBased() == true)
            m_addressSpin->setValue(selectAddress);
        else
            m_addressSpin->setValue(selectAddress + 1);
        m_addressValue = selectAddress;

        m_multipleGroup->setEnabled(false);
    }

    if (selectName.isEmpty() == false)
    {
        m_nameEdit->setText(selectName);
        slotNameEdited(selectName);
    }

    if (selectMode.isEmpty() == false)
    {
        int index = m_modeCombo->findText(selectMode);
        if (index != -1)
        {
            m_modeCombo->setCurrentIndex(index);
            slotModeActivated(m_modeCombo->itemText(index));
        }
    }
    else
    {
        m_channelsSpin->setValue(selectChannels);
    }

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

AddFixture::~AddFixture()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/*****************************************************************************
 * Fillers
 *****************************************************************************/

void AddFixture::fillTree(const QString& selectManufacturer,
                          const QString& selectModel)
{
    QTreeWidgetItem* parent;
    QTreeWidgetItem* child;
    QString manuf;
    QString model;

    /* Clear the tree of any previous data */
    m_tree->clear();

    /* Add all known fixture definitions to the tree */
    QStringListIterator it(m_fixtureDefCache.manufacturers());
    while (it.hasNext() == true)
    {
        manuf = it.next();
        parent = new QTreeWidgetItem(m_tree);
        parent->setText(KColumnName, manuf);

        QStringListIterator modit(m_fixtureDefCache.models(manuf));
        while (modit.hasNext() == true)
        {
            model = modit.next();
            child = new QTreeWidgetItem(parent);
            child->setText(KColumnName, model);

            if (manuf == selectManufacturer &&
                    model == selectModel)
            {
                parent->setExpanded(true);
                m_tree->setCurrentItem(child);
            }
        }
    }

    /* Sort the tree A-Z BEFORE appending a generic entry */
    m_tree->sortItems(0, Qt::AscendingOrder);

    /* Create a parent and a child for generic dimmer device */
    parent = new QTreeWidgetItem(m_tree);
    parent->setText(KColumnName, KXMLFixtureGeneric);
    child = new QTreeWidgetItem(parent);
    child->setText(KColumnName, KXMLFixtureGeneric);

    /* Select generic dimmer by default */
    if (selectManufacturer == KXMLFixtureGeneric &&
            selectModel == KXMLFixtureGeneric)
    {
        parent->setExpanded(true);
        m_tree->setCurrentItem(child);
    }
}

void AddFixture::fillModeCombo(const QString& text)
{
    m_modeCombo->clear();

    if (m_fixtureDef == NULL)
    {
        m_modeCombo->setEnabled(false);
        m_modeCombo->addItem(text);
        m_modeCombo->setCurrentIndex(0);
        m_mode = NULL;
    }
    else
    {
        m_modeCombo->setEnabled(true);

        QListIterator <QLCFixtureMode*> it(m_fixtureDef->modes());
        while (it.hasNext() == true)
            m_modeCombo->addItem(it.next()->name());

        /* Select the first mode by default */
        m_modeCombo->setCurrentIndex(0);
        slotModeActivated(m_modeCombo->currentText());
    }
}

void AddFixture::findAddress()
{
    /* Find the next free address space for x fixtures, each taking y
       channels, leaving z channels gap in-between. */
    quint32 address = m_doc.findAddress(
                            (m_channelsValue + m_gapValue) * m_amountValue);

    /* Set the address only if the channel space was really found */
    if (address != QLCChannel::invalid())
    {
        m_universeCombo->setCurrentIndex(address >> 9);

        OutputPatch* op = m_outputMap.patch(m_universeValue);
        if (op != NULL && op->isDMXZeroBased() == true)
            m_addressSpin->setValue(address & 0x01FF);
        else
            m_addressSpin->setValue((address & 0x01FF) + 1);
    }
}

void AddFixture::updateMaximumAmount()
{
    m_amountSpin->setRange(1, (512 - m_addressSpin->value()) /
                           (m_channelsSpin->value() + m_gapSpin->value()));
}

/*****************************************************************************
 * Slots
 *****************************************************************************/

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

    m_channelsSpin->setValue(m_mode->channels().size());

    /* Show all selected mode channels in the list */
    m_channelList->clear();
    for (int i = 0; i < m_mode->channels().size(); i++)
    {
        QLCChannel* channel = m_mode->channel(i);
        Q_ASSERT(channel != NULL);

        new QListWidgetItem(
            QString("%1: %2").arg(i + 1).arg(channel->name()),
            m_channelList);
    }
}

void AddFixture::slotUniverseActivated(int universe)
{
    int value = m_addressSpin->value();
    bool zeroBaseChanged = true;

    OutputPatch* op1 = m_outputMap.patch(m_universeValue);
    OutputPatch* op2 = m_outputMap.patch(universe);
    if (op1 != NULL && op2 != NULL &&
        op1->isDMXZeroBased() == op2->isDMXZeroBased())
    {
        zeroBaseChanged = false;
    }

    m_universeValue = universe;

    /* Adjust the available address range */
    slotChannelsChanged(m_channelsValue);

    /* If the zero-based setting is changed, change also the current address
       setting accordingly (e.g. x in 0-511 is x+1 in 1-512 & vice versa) */
    if (zeroBaseChanged == true)
    {
        OutputPatch* op = m_outputMap.patch(universe);
        if (op != NULL && op->isDMXZeroBased() == true)
            m_addressSpin->setValue(value - 1);
        else
            m_addressSpin->setValue(value + 1);
    }
}

void AddFixture::slotAddressChanged(int value)
{
    OutputPatch* op = m_outputMap.patch(m_universeCombo->currentIndex());
    if (op != NULL && op->isDMXZeroBased() == true)
        m_addressValue = value;
    else
        m_addressValue = value - 1;

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotChannelsChanged(int value)
{
    m_channelsValue = value;

    /* Set the maximum possible address so that channels cannot overflow
       beyond DMX's range of 512 channels */
    OutputPatch* op = m_outputMap.patch(m_universeValue);
    if (op != NULL && op->isDMXZeroBased() == true)
        m_addressSpin->setRange(0, 512 - value);
    else
        m_addressSpin->setRange(1, 513 - value);

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotNameEdited(const QString &text)
{
    /* If the user clears the text in the name field,
       start substituting the name with the model again. */
    if (text.length() == 0)
        m_nameEdit->setModified(false);
    else
        m_nameEdit->setModified(true);
    m_nameValue = text;
}

void AddFixture::slotAmountSpinChanged(int value)
{
    m_amountValue = value;
}

void AddFixture::slotGapSpinChanged(int value)
{
    m_gapValue = value;

    /* Set the maximum number of fixtures */
    updateMaximumAmount();
}

void AddFixture::slotSelectionChanged()
{
    /* If there is no valid selection (user has selected only a
       manufacturer or nothing at all) don't let him press OK. */
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL || item->parent() == NULL)
    {
        /* Reset the selected fixture pointer */
        m_fixtureDef = NULL;

        /* Since there is no m_fixtureDef, mode combo is cleared */
        fillModeCombo();

        /* Clear the name box unless it has been modified by user */
        if (m_nameEdit->isModified() == false)
            m_nameEdit->setText(QString());
        m_nameEdit->setEnabled(false);

        m_channelsSpin->setValue(0);
        m_channelList->clear();
        m_addressSpin->setEnabled(false);
        m_universeCombo->setEnabled(false);

        m_amountSpin->setEnabled(false);
        m_gapSpin->setEnabled(false);
        m_channelsSpin->setEnabled(false);

        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

        return;
    }

    /* Item & its parent should be valid here */
    QString manuf(item->parent()->text(KColumnName));
    QString model(item->text(KColumnName));
    if (manuf == KXMLFixtureGeneric && model == KXMLFixtureGeneric)
    {
        /* Generic dimmer selected. User enters number of channels. */
        m_fixtureDef = NULL;
        m_mode = NULL;
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
        /* Specific fixture definition selected. */
        m_fixtureDef = m_fixtureDefCache.fixtureDef(manuf, model);
        Q_ASSERT(m_fixtureDef != NULL);

        /* Put fixture def's modes to the mode combo */
        fillModeCombo();

        /* Fixture def contains number of channels, so disable the
           spin box to prevent user from modifying it. */
        m_channelsSpin->setEnabled(false);

        /* Set the model name as the fixture's friendly name ONLY
           if the user hasn't modified the friendly name field. */
        if (m_nameEdit->isModified() == false)
            m_nameEdit->setText(m_fixtureDef->model());
        m_nameEdit->setEnabled(true);
    }

    /* Set the maximum number of fixtures */
    updateMaximumAmount();

    /* Guide the user to edit the friendly name field */
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    m_nameEdit->setFocus();

    m_addressSpin->setEnabled(true);
    m_universeCombo->setEnabled(true);

    m_amountSpin->setEnabled(true);
    m_gapSpin->setEnabled(true);

    /* OK is again possible */
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok |
                                    QDialogButtonBox::Cancel);
}

void AddFixture::slotTreeDoubleClicked(QTreeWidgetItem* item)
{
    /* Select and accept (click OK for the user) */
    slotSelectionChanged();
    if (item != NULL && item->parent() != NULL)
        accept();
}
