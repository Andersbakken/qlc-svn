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
	m_timer = NULL;
	m_testMod = 1;
	m_output = KOutputInvalid;

	setupUi(this);

	for (unsigned int i = 0;
	     i < sizeof(known_devices) / sizeof(FTDIDevice);
	     i++)
	{
		QVariant v;
		v.setValue(known_devices[i]);
		m_device->addItem(QString(known_devices[i].name), v);
	}

	m_current_pid = known_devices[0].pid;
	m_current_vid = known_devices[0].vid;
	
	// Hide the pid/vid setters for Windows	
#ifdef WIN32
	label->setVisible(false);
	m_device->setVisible(false);
	label_2->setVisible(false);
	m_vid->setVisible(false);
	label_3->setVisible(false);
	m_pid->setVisible(false);
	gridLayout->removeItem(horizontalLayout);
#endif

	connect(m_testButton, SIGNAL(toggled(bool)),
		this, SLOT(slotTestToggled(bool)));
	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));
	connect(m_device, SIGNAL(currentIndexChanged(int)),
		this, SLOT(slotDeviceChanged(int)));

	m_plugin->setVIDPID(m_current_vid, m_current_pid);
	refreshList();
}

ConfigureFTDIDMXOut::~ConfigureFTDIDMXOut()
{
	slotTestToggled(false);
}

/*****************************************************************************
 * Universe testing
 *****************************************************************************/

void ConfigureFTDIDMXOut::slotTestToggled(bool state)
{
	QTreeWidgetItem* item = NULL;

	if (state == true)
	{
		item = m_list->currentItem();
		if (item == NULL)
		{
			/* If there is no selection, don't toggle the button */
			m_testButton->setDown(false);
		}
		else
		{
			/* Get the number of the universe to test */
			m_output = item->text(KColumnOutput).toInt();

			/* Open the output line for testing */
			m_plugin->open(m_output);

			/* Disable the listview so that the selection cannot
			   be changed during testing */
			m_list->setEnabled(false);
			m_buttonBox->setEnabled(false);
			
			/* Start a 1sec timer that blinks all channels of the
			   selected universe on and off */
			m_timer = new QTimer(this);
			connect(m_timer, SIGNAL(timeout()),
				this, SLOT(slotTestTimeout()));
			m_timer->start(1000);

			/* Do the first cycle already here, since the first
			   timeout occurs after one second */
			slotTestTimeout();
		}
	}
	else
	{
		delete m_timer;
		m_timer = NULL;

		/* Open the output line for testing */
		m_plugin->close(m_output);
		
		m_list->setEnabled(true);
		m_buttonBox->setEnabled(true);

		/* Reset channel values to zero */
		if (m_testMod == 1)
		{
			m_testMod = 0;
			slotTestTimeout();
			m_testMod = 0;
		}
		
		m_output = KOutputInvalid;
	}
}

void ConfigureFTDIDMXOut::slotTestTimeout()
{
	t_value values[512];

	if (m_output == KOutputInvalid)
		return;

	if (m_testMod == 0)
		for (t_channel i = 0; i < 512; i++)
			values[i] = 0;
	else
		for (t_channel i = 0; i < 512; i++)
			values[i] = 255;
	m_testMod = (m_testMod + 1) % 2;

	m_plugin->writeRange(m_output, 0, values, 512);
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureFTDIDMXOut::slotRefreshClicked()
{
	if (m_current_vid == 0 || m_current_pid == 0)
	{
		// Get the typed in VID/PID
		int vid = getIntHex(m_vid);
		int pid = getIntHex(m_pid);
		m_plugin->setVIDPID(vid, pid);
	}
	refreshList();
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

void ConfigureFTDIDMXOut::slotDeviceChanged(int index)
{
	QVariant d = m_device->itemData(index);
	FTDIDevice device = d.value<FTDIDevice>();
	
	m_current_vid = device.vid;
	m_current_pid = device.pid;
	
	if (m_current_vid == 0 || m_current_pid == 0)
	{
		int vid = getIntHex(m_vid);
		int pid = getIntHex(m_pid);

		if (vid != 0 && pid != 0)
		{
			m_plugin->setVIDPID(vid, pid);
			refreshList();
		}
	}
	else if (m_current_vid != 0 && m_current_pid != 0)
	{
		m_plugin->setVIDPID(m_current_vid, m_current_pid);
		refreshList();
	}
}

void ConfigureFTDIDMXOut::refreshList()
{
	t_output i = 0;

	m_list->clear();

	QMapIterator <t_output, FTDIDMXDevice*> it(m_plugin->m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(KColumnName, it.value()->name());
		item->setText(KColumnOutput, QString("%1").arg(i++));
	}
}
