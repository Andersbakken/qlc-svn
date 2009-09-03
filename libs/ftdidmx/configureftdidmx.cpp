/*
  Q Light Controller
  configureftdidmx.cpp
  
  Copyright (c) Christopher Staite
 
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
#include <QMapIterator>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QMap>
#include <QSettings>

#include "ftdidmx.h"
#include "ftdidmxdevice.h"
#include "configureftdidmx.h"

#define KColumnName   0
#define KColumnOutput 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureFTDIDMXOut::ConfigureFTDIDMXOut(QWidget* parent, FTDIDMXOut* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

	setupUi(this);
	
	m_device->clear();
	m_device->addItem(QString(""), QVariant(-1));
	for (int i = 0; i < plugin->m_number_device_types; i++)
	{
		m_device->addItem(QString(plugin->m_device_types[i].name), QVariant(i));
	}
	m_device->setCurrentIndex(0);
	
	// Hide the pid/vid setters for Windows	
#ifdef WIN32
	typeContainer->setVisible(false);
#endif

	connect(m_refreshButton, SIGNAL(clicked()),
			this, SLOT(slotRefreshClicked()));
	connect(m_addTypeButton, SIGNAL(clicked()),
			this, SLOT(slotAddTypeClicked()));
	connect(m_device, SIGNAL(currentIndexChanged(int)),
			this, SLOT(slotDeviceTypeChanged(int)));
	connect(m_list, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
			this, SLOT(slotDeviceChanged(QTreeWidgetItem*, int)));

	refreshList();
}

ConfigureFTDIDMXOut::~ConfigureFTDIDMXOut()
{
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureFTDIDMXOut::slotRefreshClicked()
{
	refreshList();
}

void ConfigureFTDIDMXOut::slotAddTypeClicked()
{
	QString name = m_typeName->text();
	int vid = this->getIntHex(m_vid);
	int pid = this->getIntHex(m_pid);
	
	QSettings settings;
	int types = settings.value("/ftdidmx/types/number", QVariant(0)).toInt();
	
	settings.setValue(QString("/ftdidmx/types/vid%1").arg(types), QVariant(vid));
	settings.setValue(QString("/ftdidmx/types/pid%1").arg(types), QVariant(pid));
	settings.setValue(QString("/ftdidmx/types/interface%1").arg(types), QVariant(0));
	settings.setValue(QString("/ftdidmx/types/name%1").arg(types), QVariant(name));
	settings.setValue("/ftdidmx/types/number", QVariant(types + 1));
	
	FTDIDevice *oldDevices = m_plugin->m_device_types;
	FTDIDevice *newDevices = (FTDIDevice*)malloc(sizeof(FTDIDevice)*(m_plugin->m_number_device_types + 1));
	
	for (int i = 0; i < m_plugin->m_number_device_types; i++) {
		memcpy(&newDevices[i], &oldDevices[i], sizeof(FTDIDevice));
	}
	newDevices[m_plugin->m_number_device_types].name = name.toAscii();
	newDevices[m_plugin->m_number_device_types].vid = vid;
	newDevices[m_plugin->m_number_device_types].pid = pid;
	newDevices[m_plugin->m_number_device_types].type = 0;
	
	m_device->addItem(name, QVariant(m_plugin->m_number_device_types));
	
	m_plugin->m_device_types = newDevices;
	m_plugin->m_number_device_types++;
	free(oldDevices);
	
	m_vid->setText("");
	m_pid->setText("");
	m_typeName->setText("");
}

int ConfigureFTDIDMXOut::getIntHex(QLineEdit *e)
{
	QString value = e->displayText();
	if (value == QString(""))
		return 0;
	bool ok = false;
	int ret = value.toInt(&ok, 0);
	if (ok == false)
		ret = 0;
	return ret;
}

void ConfigureFTDIDMXOut::slotDeviceChanged(QTreeWidgetItem * current, int column)
{
	Q_UNUSED(column);

	if (current == NULL) {
		return;
	}

	QSettings settings;
	QVariant type = settings.value(QString("/ftdidmx/devices/type%1").arg(m_list->indexOfTopLevelItem(current)), QVariant(0));
	if (type.type() != QVariant::Int) {
		return;
	}
	
	m_device->setCurrentIndex(type.toInt() + 1);
}

void ConfigureFTDIDMXOut::slotDeviceTypeChanged(int index)
{
	QVariant d = m_device->itemData(index);
	
	QList<QTreeWidgetItem *> s = m_list->selectedItems();
	if (s.count() == 0) {
		return;
	}

	QSettings settings;
	int output = m_list->indexOfTopLevelItem(s.first());
	settings.setValue(QString("/ftdidmx/devices/type%1").arg(output), d);
	
	if (m_plugin->m_devices.contains(output)) {
		m_plugin->m_devices.value(output)->setType(m_plugin->m_device_types[d.toInt()].type);
	}
}

void ConfigureFTDIDMXOut::refreshList()
{
	m_list->clear();
	QSettings settings;
	
	int loadedDevices = settings.value("/ftdidmx/devices/number", QVariant(0)).toInt();

	for (int i = 0; i < loadedDevices; i++) {
		QString serial = settings.value(QString("/ftdidmx/devices/serial%1").arg(i)).toString();
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(KColumnName, serial);
		item->setText(KColumnOutput, QString("%1").arg(i));
	}

	t_output output = loadedDevices;	
	for (int i = 0; i < m_plugin->m_number_device_types; i++) {
#ifndef WIN32
		FT_SetVIDPID(m_plugin->m_device_types[i].vid, m_plugin->m_device_types[i].pid);
#endif

		DWORD devices;
		if (FT_CreateDeviceInfoList(&devices) != FT_OK)
			devices = MAX_NUM_DEVICES;
		
		char devString[devices][64];
		char *devStringPtr[devices + 1];
		
		for (unsigned int j = 0; j < devices; j++)
			devStringPtr[j] = devString[j];
		devStringPtr[devices] = NULL;
		
		FT_STATUS st = FT_ListDevices(devStringPtr, &devices,
									  FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
		if (st == FT_OK)
		{
			while (devices > 0)
			{
				devices--;
				
				QString serial = QString(devString[devices]);
				
				bool found = false;
				for (int j = 0; j < loadedDevices; j++) {
					if (settings.value(QString("/ftdidmx/devices/serial%1").arg(j)).toString() == serial) {
						found = true;
					}
				}
				if (!found) {
					QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
					item->setText(KColumnName, QString(serial));
					item->setText(KColumnOutput, QString("%1").arg(output));
					
					settings.setValue(QString("/ftdidmx/devices/serial%1").arg(output), QVariant(serial));
					settings.setValue(QString("/ftdidmx/devices/type%1").arg(output), QVariant(i));
					settings.setValue(QString("/ftdidmx/devices/vid%1").arg(output), QVariant(m_plugin->m_device_types[i].vid));
					settings.setValue(QString("/ftdidmx/devices/pid%1").arg(output), QVariant(m_plugin->m_device_types[i].pid));
					output++;
				}
			}
		}
	}
	settings.setValue("/ftdidmx/devices/number", QVariant(output));
}
