/*
  Q Light Controller
  configuremidiout.cpp

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
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "configuremididevice.h"
#include "configuremidiout.h"
#include "mididevice.h"
#include "midiout.h"

#define KColumnNumber      0
#define KColumnName        1
#define KColumnMIDIChannel 2
#define KColumnMode        3

ConfigureMIDIOut::ConfigureMIDIOut(QWidget* parent, MIDIOut* plugin)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);
    m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(m_refreshButton, SIGNAL(clicked()),
            this, SLOT(slotRefreshClicked()));
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(slotEditClicked()));
    connect(m_list, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));

    /* Listen to device additions/removals */
    connect(plugin, SIGNAL(deviceRemoved(MIDIDevice*)),
            this, SLOT(slotDeviceRemoved(MIDIDevice*)));
    connect(plugin, SIGNAL(deviceAdded(MIDIDevice*)),
            this, SLOT(slotDeviceAdded(MIDIDevice*)));

    refreshList();
}

ConfigureMIDIOut::~ConfigureMIDIOut()
{
}

/*****************************************************************************
 * List of devices
 *****************************************************************************/

void ConfigureMIDIOut::slotRefreshClicked()
{
    Q_ASSERT(m_plugin != NULL);
    m_plugin->rescanDevices();
}

void ConfigureMIDIOut::refreshList()
{
    QTreeWidgetItem* item;
    MIDIDevice* dev;
    int i = 1;

    m_list->clear();

    QListIterator <MIDIDevice*> it(m_plugin->m_devices);
    while (it.hasNext() == true)
    {
        dev = it.next();

        item = new QTreeWidgetItem(m_list);
        item->setText(KColumnNumber, QString("%1").arg(i++));
        item->setText(KColumnName, dev->name());
        item->setText(KColumnMIDIChannel,
                      QString("%1").arg(dev->midiChannel() + 1));
        item->setText(KColumnMode,
                      MIDIDevice::modeToString(dev->mode()));
    }
}

void ConfigureMIDIOut::slotEditClicked()
{
    QTreeWidgetItem* item;
    MIDIDevice* device;

    item = m_list->currentItem();
    if (item == NULL)
        return;

    device = m_plugin->device(item->text(KColumnNumber).toInt() - 1);
    if (device == NULL)
        return;

    ConfigureMIDIDevice cmd(this, device);
    if (cmd.exec() == QDialog::Accepted)
    {
        /* Update the tree item */
        item->setText(KColumnMIDIChannel,
                      QString("%1").arg(device->midiChannel() + 1));
        item->setText(KColumnMode,
                      MIDIDevice::modeToString(device->mode()));

        /* Save as global settings */
        device->saveSettings();
    }
}

void ConfigureMIDIOut::slotDeviceAdded(MIDIDevice* device)
{
    Q_UNUSED(device);
    refreshList();
}

void ConfigureMIDIOut::slotDeviceRemoved(MIDIDevice* device)
{
    QListIterator <QTreeWidgetItem*> it(m_list->findItems(device->name(),
                                        Qt::MatchExactly,
                                        KColumnName));
    while (it.hasNext() == true)
        delete it.next();
}
