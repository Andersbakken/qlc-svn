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
#include <QMapIterator>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QMap>

#include "configureusbdmxout.h"

#ifdef WIN32
#include "usbdmxdevice-win32.h"
#include "usbdmxout-win32.h"
#else
#include "usbdmxdevice-unix.h"
#include "usbdmxout-unix.h"
#endif

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureUSBDMXOut::ConfigureUSBDMXOut(QWidget* parent, USBDMXOut* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;
	m_timer = NULL;
	m_testMod = 1;
	m_testUniverse = -1;

	setupUi(this);

	connect(m_testButton, SIGNAL(toggled(bool)),
		this, SLOT(slotTestToggled(bool)));
	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	refreshList();
}

ConfigureUSBDMXOut::~ConfigureUSBDMXOut()
{
	slotTestToggled(false);
}

/*****************************************************************************
 * Universe testing
 *****************************************************************************/

void ConfigureUSBDMXOut::slotTestToggled(bool state)
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
			m_testUniverse = item->text(0).toInt() - 1;

			/* Disable the listview so that the selection cannot
			   be changed during testing */
			m_list->setEnabled(false);
			
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
		
		m_list->setEnabled(true);

		/* Reset channel values to zero */
		if (m_testMod == 1)
		{
			m_testMod = 0;
			slotTestTimeout();
			m_testMod = 0;
		}
		
		m_testUniverse = -1;
	}
}

void ConfigureUSBDMXOut::slotTestTimeout()
{
	t_value values[512];

	if (m_testUniverse < 0)
		return;

	if (m_testMod == 0)
		for (t_channel i = 0; i < 512; i++)
			values[i] = 0;
	else
		for (t_channel i = 0; i < 512; i++)
			values[i] = 255;
	m_testMod = (m_testMod + 1) % 2;

	m_plugin->writeRange(m_testUniverse, 0, values, 512);
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureUSBDMXOut::slotRefreshClicked()
{
	refreshList();
}

void ConfigureUSBDMXOut::refreshList()
{
	int i = 0;

	m_list->clear();

	QMapIterator <t_output, USBDMXDevice*> it(m_plugin->m_devices);
	while (it.hasNext() == true)
	{
		it.next();
		
		QTreeWidgetItem* item = new QTreeWidgetItem(m_list);
		item->setText(0, QString("%1").arg(i++));
		item->setText(1, it.value()->name());
	}
}
