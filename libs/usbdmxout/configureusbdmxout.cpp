/*
  Q Light Controller
  configureusbdmxout.cpp

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
#include <QPushButton>
#include <QString>
#include <QList>

#include "configureusbdmxout.h"

#ifdef WIN32
#include "usbdmxdevice-win32.h"
#include "usbdmxout-win32.h"
#else
#include "usbdmxdevice-unix.h"
#include "usbdmxout-unix.h"
#endif

#define KColumnName   0
#define KColumnOutput 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureUSBDMXOut::ConfigureUSBDMXOut(QWidget* parent, USBDMXOut* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

	setupUi(this);

	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));

	refreshList();
}

ConfigureUSBDMXOut::~ConfigureUSBDMXOut()
{
}

/*****************************************************************************
 * Refresh
 *****************************************************************************/

void ConfigureUSBDMXOut::slotRefreshClicked()
{
	m_plugin->rescanDevices();
	refreshList();
}

void ConfigureUSBDMXOut::refreshList()
{
	t_output i = 0;

	m_list->clear();

	QListIterator <USBDMXDevice*> it(m_plugin->m_devices);
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(KColumnName, it.next()->name());
		item->setText(KColumnOutput, QString("%1").arg(i++));
	}
}
