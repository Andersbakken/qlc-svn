/*
  Q Light Controller
  configurehidinput.cpp
  
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
#include <QTreeWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "configurehidinput.h"
#include "hidinput.h"

#define KColumnNumber  0
#define KColumnEnabled 1
#define KColumnName    2

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureHIDInput::ConfigureHIDInput(QWidget* parent, HIDInput* plugin)
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
	connect(plugin, SIGNAL(deviceRemoved(HIDDevice*)),
		this, SLOT(slotDeviceRemoved(HIDDevice*)));
	connect(plugin, SIGNAL(deviceAdded(HIDDevice*)),
		this, SLOT(slotDeviceAdded(HIDDevice*)));

	refreshList();
}

ConfigureHIDInput::~ConfigureHIDInput()
{
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureHIDInput::slotRefreshClicked()
{
	Q_ASSERT(m_plugin != NULL);
	m_plugin->rescanDevices();
}

void ConfigureHIDInput::refreshList()
{
	QString s;

	m_list->clear();

	disconnect(m_list, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		   this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));

	for (int i = 0; i < m_plugin->m_devices.count(); i++)
	{
		HIDDevice* dev;
		QTreeWidgetItem* item;

		dev = m_plugin->device(i);
		Q_ASSERT(dev != NULL);

		item = new QTreeWidgetItem(m_list);
		item->setText(KColumnNumber, s.setNum(i + 1));
		item->setText(KColumnName, dev->name());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

		if (dev->isEnabled() == true)
			item->setCheckState(KColumnEnabled, Qt::Checked);
		else
			item->setCheckState(KColumnEnabled, Qt::Unchecked);
	}

	connect(m_list, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void ConfigureHIDInput::slotItemChanged(QTreeWidgetItem* item, int column)
{
	if (column == KColumnEnabled)
	{
		HIDDevice* dev;
		dev = m_plugin->device(item->text(KColumnNumber).toInt() - 1);
		Q_ASSERT(dev != NULL);
		dev->setEnabled(item->checkState(column));
	}
}

void ConfigureHIDInput::slotDeviceAdded(HIDDevice*)
{
	refreshList();
}

void ConfigureHIDInput::slotDeviceRemoved(HIDDevice* device)
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
