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
#ifdef WIN32
#include "win32-mididevice.h"
#include "win32-midiinput.h"
#else
#include "unix-mididevice.h"
#include "unix-midiinput.h"
#endif

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
	QTreeWidgetItem* item;
	int i = 0;

	m_list->clear();

	QStringListIterator it(m_plugin->deviceNames());
	while (it.hasNext() == true)
	{
		item = new QTreeWidgetItem(m_list);
		item->setText(KColumnNumber, QString("%1").arg(i++));
		item->setText(KColumnName, it.next());
	}
}

void ConfigureMIDIInput::slotDeviceAdded(MIDIDevice* device)
{
	refreshList();
}

void ConfigureMIDIInput::slotDeviceRemoved(MIDIDevice* device)
{
	QListIterator <QTreeWidgetItem*> it(m_list->findItems(device->name(),
							      Qt::MatchExactly,
							      KColumnName));
	while (it.hasNext() == true)
		delete it.next();
}
