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

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <qtimer.h>

#include "configureusbdmxout.h"
#include "usbdmxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureUSBDMXOut::ConfigureUSBDMXOut(QWidget* parent, USBDMXOut* plugin)
	: UI_ConfigureUSBDMXOut(parent, "Configure USBDMX", true)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;
	m_timer = NULL;
	m_testMod = 1;
	m_testUniverse = -1;

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
	QListViewItem* item = NULL;

	if (state == true)
	{
		item = m_listView->currentItem();
		if (item == NULL)
		{
			/* If there is no selection, don't toggle the button */
			m_testButton->setOn(false);
		}
		else
		{
			/* Get the number of the universe to test */
			m_testUniverse = item->text(0).toInt() - 1;

			/* Disable the listview so that the selection cannot
			   be changed during testing */
			m_listView->setEnabled(false);
			
			/* Start a 1sec timer that blinks all channels of the
			   selected universe on and off */
			m_timer = new QTimer(this);
			connect(m_timer, SIGNAL(timeout()),
				this, SLOT(slotTestTimeout()));
			m_timer->start(1000, false);

			/* Do the first cycle already here, since the first
			   timeout occurs after one second */
			slotTestTimeout();
		}
	}
	else
	{
		delete m_timer;
		m_timer = NULL;
		
		m_listView->setEnabled(true);

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

	m_plugin->writeRange(m_testUniverse, values, 512);

	m_testMod = (m_testMod + 1) % 2;
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureUSBDMXOut::slotRefreshClicked()
{
	m_plugin->open();
	refreshList();
}

void ConfigureUSBDMXOut::refreshList()
{
	QString s;

	m_listView->clear();

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		if (m_plugin->m_devices[i] >= 0)
		{
			new QListViewItem(m_listView,
					  s.sprintf("%.2d", i + 1),
					  s.sprintf("/dev/usbdmx%d", i));
		}
	}
}
