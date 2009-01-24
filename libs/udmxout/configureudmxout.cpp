/*
  Q Light Controller
  configureudmxout.cpp

  Copyright (c)	Lutz Hillebrand
		Heikki Junnila

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
#include <QTimer>
#include <QList>

#include "configureudmxout.h"
#include "udmxdevice.h"
#include "udmxout.h"

#define KColumnName   0
#define KColumnOutput 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureUDMXOut::ConfigureUDMXOut(QWidget* parent, UDMXOut* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

	setupUi(this);

	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));

	refreshList();
}

ConfigureUDMXOut::~ConfigureUDMXOut()
{
}

/*****************************************************************************
 * Refresh
 *****************************************************************************/

void ConfigureUDMXOut::slotRefreshClicked()
{
	m_plugin->rescanDevices();
	refreshList();
}

void ConfigureUDMXOut::refreshList()
{
	t_output i = 0;

	m_list->clear();

	QListIterator <UDMXDevice*> it(m_plugin->m_devices);
	while (it.hasNext() == true)
	{
		UDMXDevice* udev = it.next();

		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(KColumnName, udev->name());
		item->setText(KColumnOutput, QString("%1").arg(i++));
	}
}
