/*
  Q Light Controller
  configuremidiinput.cpp

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
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "configuremidiinput.h"
#include "configuremidiline.h"
#include "mididevice.h"
#include "midiinput.h"

#define KColumnNumber   0
#define KColumnName     1
#define KColumnChannel  2
#define KColumnFeedBack 3

ConfigureMIDIInput::ConfigureMIDIInput(QWidget* parent, MIDIInput* plugin)
        : QDialog(parent)
{
    QStringList headerLabels;

    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    setupUi(this);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    /* One needs to choose the particular output line for feedback only
       in windows, where input & output lines don't have the same ID. */
    headerLabels << tr("Input") << tr("Name") << tr("MIDI Channel");
#ifdef WIN32
    headerLabels << tr("Feedback line");
#endif
    m_tree->setHeaderLabels(headerLabels);

    /* Enable the configuration button only for windows because ALSA
       uses the same address for input and output. */
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(slotEditClicked()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));
    connect(m_refreshButton, SIGNAL(clicked()),
            this, SLOT(slotRefreshClicked()));

    /* Listen to device additions/removals */
    connect(plugin, SIGNAL(deviceRemoved(MIDIDevice*)),
            this, SLOT(slotDeviceRemoved(MIDIDevice*)));
    connect(plugin, SIGNAL(deviceAdded(MIDIDevice*)),
            this, SLOT(slotDeviceAdded(MIDIDevice*)));

    refreshList();
}

ConfigureMIDIInput::~ConfigureMIDIInput()
{
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void ConfigureMIDIInput::refreshList()
{
    int i = 1;

    m_tree->clear();

    QListIterator <MIDIDevice*> it(m_plugin->devices());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;
        MIDIDevice* dev;

        dev = it.next();
        Q_ASSERT(dev != NULL);

        item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnNumber, QString("%1").arg(i++));
        item->setText(KColumnName, dev->name());
        uchar channel = dev->midiChannel();
        if (channel < 16)
            item->setText(KColumnChannel, tr("Channel %1").arg(channel + 1));
        else
            item->setText(KColumnChannel, tr("Any Channel"));

#ifdef WIN32
        if (dev->feedBackId() != UINT_MAX)
        {
            item->setText(KColumnFeedBack,
                          MIDIDevice::feedBackNames()[dev->feedBackId()]);
        }
        else
        {
            item->setText(KColumnFeedBack, tr("None"));
        }
#endif
    }
}

void ConfigureMIDIInput::slotDeviceAdded(MIDIDevice* device)
{
    Q_UNUSED(device);
    refreshList();
}

void ConfigureMIDIInput::slotDeviceRemoved(MIDIDevice* device)
{
    QListIterator <QTreeWidgetItem*> it(m_tree->findItems(device->name(),
                                        Qt::MatchExactly,
                                        KColumnName));
    while (it.hasNext() == true)
        delete it.next();
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void ConfigureMIDIInput::slotRefreshClicked()
{
    Q_ASSERT(m_plugin != NULL);
    m_plugin->rescanDevices();
}

void ConfigureMIDIInput::slotEditClicked()
{
    QTreeWidgetItem* item;
    MIDIDevice* device;
    quint32 input;

    /* Get the currently selected tree widget item */
    item = m_tree->currentItem();
    if (item == NULL)
        return;

    /* Get the device represented by the selected item */
    input = item->text(KColumnNumber).toInt() - 1;
    device = m_plugin->device(input);
    Q_ASSERT(device != NULL);

    /* Configure the device */
    ConfigureMIDILine cml(this, device);
    if (cml.exec() == QDialog::Accepted)
        refreshList();
}
