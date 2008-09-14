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
#include "mididevice.h"
#include "midiinput.h"

#define KColumnNumber  0
#define KColumnName    1

ConfigureMIDIInput::ConfigureMIDIInput(QWidget* parent, MIDIInput* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;
	
	/* Setup UI controls */
	setupUi(this);
	m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

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
 * List of devices
 *****************************************************************************/

void ConfigureMIDIInput::slotRefreshClicked()
{
	Q_ASSERT(m_plugin != NULL);
	m_plugin->rescanDevices();
}

void ConfigureMIDIInput::refreshList()
{
	QString s;

	m_list->clear();

	for (int i = 0; i < m_plugin->m_devices.count(); i++)
	{
		MIDIDevice* device;
		QTreeWidgetItem* item;

		device = m_plugin->device(i);
		Q_ASSERT(device != NULL);

		item = new QTreeWidgetItem(m_list);
		item->setText(KColumnNumber, s.setNum(i + 1));
		item->setText(KColumnName, device->name());
	}
}

void ConfigureMIDIInput::slotDeviceAdded(MIDIDevice*)
{
	refreshList();
}

void ConfigureMIDIInput::slotDeviceRemoved(MIDIDevice* device)
{
	Q_ASSERT(device != NULL);

	for (int i = 0; i < m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_list->topLevelItem(i);
		Q_ASSERT(item != NULL);
		if (item->text(KColumnName) == device->name())
		{
			delete item;
			break;
		}
	}
}
