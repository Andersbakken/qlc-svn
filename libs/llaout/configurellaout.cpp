/*
  Q Light Controller
  configurellaout.cpp
  
  Copyright (C) Simon Newton,
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
#include <QDialog>
#include <QString>
#include <QTimer>

#include "configurellaout.h"
#include "llaout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureLlaOut::ConfigureLlaOut(QWidget* parent, LlaOut* plugin) 
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

	setupUi(this);

	m_timer = NULL;
	m_testUniverse = -1;
	m_testMod = 0;
}

ConfigureLlaOut::~ConfigureLlaOut()
{
	slotTestToggled(false);
}

/*****************************************************************************
 * Universe testing
 *****************************************************************************/

void ConfigureLlaOut::slotTestToggled(bool state)
{
	QTreeWidgetItem* item;

	if (state == true)
	{
		item = m_listView->currentItem();
		if (item == NULL)
		{
			/* If there is no selection, don't toggle the button */
			m_testButton->setChecked(false);
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
			m_timer->start(1000);

			/* Do the first cycle already here, since the first
			   timeout occurs after one second */
			m_testMod = 1;
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

void ConfigureLlaOut::slotTestTimeout()
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
 * Refresh
 *****************************************************************************/

void ConfigureLlaOut::slotRefreshClicked()
{
	m_plugin->open();
	refreshList();
}

void ConfigureLlaOut::refreshList()
{
}
