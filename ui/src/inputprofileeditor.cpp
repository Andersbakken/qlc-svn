/*
  Q Light Controller
  inputprofileeditor.cpp

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
#include <QTextBrowser>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QTabWidget>
#include <QSettings>
#include <QDialog>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QDir>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"

#include "inputchanneleditor.h"
#include "inputprofileeditor.h"
#include "inputmap.h"
#include "apputil.h"
#include "app.h"

extern App* _app;

#define SETTINGS_GEOMETRY "inputprofileeditor/geometry"

#define KColumnNumber 0
#define KColumnName   1
#define KColumnType   2
#define KColumnValues 3

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputProfileEditor::InputProfileEditor(QWidget* parent,
                                       QLCInputProfile* profile)
        : QDialog(parent)
{
    setupUi(this);

    /* Connect the buttons to slots */
    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddClicked()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(slotEditClicked()));
    connect(m_wizardButton, SIGNAL(clicked(bool)),
            this, SLOT(slotWizardClicked(bool)));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));

    if (profile == NULL)
    {
        m_profile = new QLCInputProfile();
    }
    else
    {
        m_profile = new QLCInputProfile(*profile);
        if ((QFile::permissions(m_profile->path()) &
                QFile::WriteUser) == 0)
        {
            QMessageBox::warning(this, tr("File not writable"),
                                 tr("You do not have permission to write to "
                                    "the file %1. You might not be able to "
                                    "save your modifications to the profile.")
                                 .arg(QDir::toNativeSeparators(
                                          m_profile->path())));
        }
    }

    /* Profile manufacturer & model */
    m_manufacturerEdit->setText(m_profile->manufacturer());
    m_modelEdit->setText(m_profile->model());

    /* Fill up the tree with profile's channels */
    fillTree();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

InputProfileEditor::~InputProfileEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    delete m_profile;
}

void InputProfileEditor::fillTree()
{
    m_tree->clear();

    QMapIterator <quint32,QLCInputChannel*> it(m_profile->channels());
    while (it.hasNext() == true)
    {
        it.next();
        updateChannelItem(new QTreeWidgetItem(m_tree), it.value());
    }
}

void InputProfileEditor::updateChannelItem(QTreeWidgetItem* item,
                                           QLCInputChannel* ch)
{
    quint32 num;

    Q_ASSERT(item != NULL);
    Q_ASSERT(ch != NULL);

    num = m_profile->channelNumber(ch);
    item->setText(KColumnNumber, QString("%1").arg(num + 1));
    item->setText(KColumnName, ch->name());
    item->setText(KColumnType, QLCInputChannel::typeToString(ch->type()));

    /* Display nice icons to indicate channel type */
    if (ch->type() == QLCInputChannel::Slider)
        item->setIcon(KColumnType, QIcon(":/slider.png"));
    else if (ch->type() == QLCInputChannel::Knob)
        item->setIcon(KColumnType, QIcon(":/knob.png"));
    else if (ch->type() == QLCInputChannel::Button)
        item->setIcon(KColumnType, QIcon(":/button.png"));
}

/****************************************************************************
 * OK & Cancel
 ****************************************************************************/

void InputProfileEditor::reject()
{
    /* Don't allow closing the dialog in any way when the wizard is on */
    if (m_buttonBox->isEnabled() == false)
        return;

    QDialog::reject();
}

void InputProfileEditor::accept()
{
    /* Don't allow closing the dialog in any way when the wizard is on */
    if (m_buttonBox->isEnabled() == false)
        return;

    m_profile->setManufacturer(m_manufacturerEdit->text());
    m_profile->setModel(m_modelEdit->text());

    /* Check that we have at least the bare necessities to save the profile */
    if (m_profile->manufacturer().isEmpty() == true ||
            m_profile->model().isEmpty() == true)
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

void InputProfileEditor::slotAddClicked()
{
    QLCInputChannel* channel = new QLCInputChannel();
    InputChannelEditor ice(this, m_profile, channel);
add:
    if (ice.exec() == QDialog::Accepted)
    {
        channel->setType(ice.type());
        channel->setName(ice.name());

        if (m_profile->channel(ice.channel()) == NULL)
        {
            m_profile->insertChannel(ice.channel(), channel);
            updateChannelItem(new QTreeWidgetItem(m_tree), channel);
        }
        else
        {
            QMessageBox::warning(this,
                                 tr("Channel already exists"),
                                 tr("Channel %1 already exists")
                                 .arg(ice.channel() + 1));
            goto add;
        }
    }
    else
    {
        delete channel;
    }
}

void InputProfileEditor::slotRemoveClicked()
{
    QList <QTreeWidgetItem*> selected;
    QTreeWidgetItem* next = NULL;
    quint32 chnum;

    /* Ask for confirmation if we're deleting more than one channel */
    selected = m_tree->selectedItems();
    if (selected.count() > 1)
    {
        int r;
        r = QMessageBox::question(this, tr("Delete channels"),
                                  tr("Delete all %1 selected channels?")
                                  .arg(selected.count()),
                                  QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::No)
            return;
    }

    /* Remove all selected channels */
    QMutableListIterator <QTreeWidgetItem*> it(selected);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;

        item = it.next();
        Q_ASSERT(item != NULL);

        /* Remove & Delete the channel object */
        chnum = item->text(KColumnNumber).toInt() - 1;
        m_profile->removeChannel(chnum);

        /* Choose the closest item below or above the removed items
           as the one that is selected after the removal */
        next = m_tree->itemBelow(item);
        if (next == NULL)
            next = m_tree->itemAbove(item);

        delete item;
    }

    m_tree->setCurrentItem(next);
}

void InputProfileEditor::slotEditClicked()
{
    QLCInputChannel* channel;
    quint32 chnum;
    QTreeWidgetItem* item;

    if (m_tree->selectedItems().count() == 1)
    {
        /* Just one item selected. Edit that. */
        item = m_tree->currentItem();
        if (item == NULL)
            return;

        /* Find the channel object associated to the selected item */
        chnum = item->text(KColumnNumber).toInt() - 1;
        channel = m_profile->channel(chnum);
        Q_ASSERT(channel != NULL);

        /* Edit the channel and update its item if necessary */
        InputChannelEditor ice(this, m_profile, channel);
edit:
        if (ice.exec() == QDialog::Accepted)
        {
            QLCInputChannel* another;
            another = m_profile->channel(ice.channel());

            if (another == NULL || another == channel)
            {
                if (ice.channel() != KInputChannelInvalid)
                    m_profile->remapChannel(channel, ice.channel());
                if (ice.name().isEmpty() == false)
                    channel->setName(ice.name());
                if (ice.type() != QLCInputChannel::NoType)
                    channel->setType(ice.type());

                updateChannelItem(item, channel);
            }
            else
            {
                QMessageBox::warning(this,
                                     tr("Channel already exists"),
                                     tr("Channel %1 already exists")
                                     .arg(ice.channel() + 1));
                goto edit;
            }
        }
    }
    else if (m_tree->selectedItems().count() > 1)
    {
        /* Multiple channels selected. Apply changes to all of them */
        InputChannelEditor ice(this, NULL, NULL);
        if (ice.exec() == QDialog::Accepted)
        {
            QListIterator <QTreeWidgetItem*>
            it(m_tree->selectedItems());
            while (it.hasNext() == true)
            {
                item = it.next();
                Q_ASSERT(item != NULL);

                chnum = item->text(KColumnNumber).toInt() - 1;
                channel = m_profile->channel(chnum);
                Q_ASSERT(channel != NULL);

                /* Set only name and type and only if they
                   have been modified. */
                if (ice.name().isEmpty() == false)
                    channel->setName(ice.name());
                if (ice.type() != QLCInputChannel::NoType)
                    channel->setType(ice.type());

                updateChannelItem(item, channel);
            }
        }
    }
}

void InputProfileEditor::slotWizardClicked(bool checked)
{
    if (checked == true)
    {
        QMessageBox::information(this, tr("Channel wizard activated"),
                                 tr("You have enabled the input channel wizard. After "
                                    "clicking OK, wiggle your mapped input profile's "
                                    "controls. They should appear into the list. "
                                    "Click the wizard button again to stop channel "
                                    "auto-detection.\n\nNote that the wizard cannot "
                                    "tell the difference between a knob and a slider "
                                    "so you will have to do the change manually."));

        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32, quint32, uchar)),
                this,
                SLOT(slotInputValueChanged(quint32, quint32, uchar)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32, quint32, uchar)),
                   this,
                   SLOT(slotInputValueChanged(quint32, quint32, uchar)));
    }

    m_buttonBox->setEnabled(!checked);
    m_tab->setTabEnabled(0, !checked);
}

void InputProfileEditor::slotInputValueChanged(quint32 universe,
                                               quint32 channel,
                                               uchar value)
{
    Q_UNUSED(universe);

    /* Get a list of items that represent the given channel. Basically
       the list should always contain just one item. */
    QList <QTreeWidgetItem*> list(m_tree->findItems(
                                      QString("%1").arg(channel + 1),
                                      Qt::MatchExactly, KColumnNumber));
    if (list.size() == 0)
    {
        /* No channel items found. Create a new channel to the
           profile and display it also in the tree widget */
        QTreeWidgetItem* item;
        QLCInputChannel* ch;

        ch = new QLCInputChannel();
        ch->setName(tr("Button %1").arg(channel + 1));
        ch->setType(QLCInputChannel::Button);
        m_profile->insertChannel(channel, ch);

        item = new QTreeWidgetItem(m_tree);
        updateChannelItem(item, ch);
        m_tree->scrollToItem(item);
    }
    else
    {
        QTreeWidgetItem* old;
        QStringList values;

        /* Existing channel & item found. Modify their contents. */
        old = list.first();
        values = old->data(KColumnValues, Qt::UserRole).toStringList();

        /* No need to collect any more values, since this channel has
           been judged to be a slider when count == 3 (see below). */
        if (values.count() > 3)
            return;
        else if (values.contains(QString("%1").arg(value)) == false)
        {
            values << QString("%1").arg(value);
            values.sort();
            old->setData(KColumnValues, Qt::UserRole, values);
        }

        /* Change the channel type only the one time when its value
           count goes over 2. I.e. when a channel can have more than
           two distinct values, it can no longer be a button. */
        if (values.count() == 3)
        {
            QLCInputChannel* ch;
            ch = m_profile->channel(channel);
            Q_ASSERT(ch != NULL);

            ch->setType(QLCInputChannel::Slider);
            ch->setName(tr("Slider %1").arg(channel + 1));
            updateChannelItem(old, ch);
        }
    }
}

/****************************************************************************
 * Profile
 ****************************************************************************/

const QLCInputProfile* InputProfileEditor::profile() const
{
    return m_profile;
}
