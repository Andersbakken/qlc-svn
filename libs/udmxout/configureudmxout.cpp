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

ConfigureUDMXOut::ConfigureUDMXOut(QWidget* parent, UDMXOut* plugin)
	: QDialog(parent)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;
	m_timer = NULL;
	m_testMod = 1;
	m_output = KOutputInvalid;

	setupUi(this);

	connect(m_testButton, SIGNAL(toggled(bool)),
		this, SLOT(slotTestToggled(bool)));
	connect(m_refreshButton, SIGNAL(clicked()),
		this, SLOT(slotRefreshClicked()));

	refreshList();
}

ConfigureUDMXOut::~ConfigureUDMXOut()
{
	slotTestToggled(false);
}

/*****************************************************************************
 * Universe testing
 *****************************************************************************/

void ConfigureUDMXOut::slotTestToggled(bool state)
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

void ConfigureUDMXOut::slotTestTimeout()
{
	t_value values[512];

	if (m_output == KOutputInvalid)
		return;

	if (m_testMod == 0)
		std::fill(values, values + 512, 0);
	else
		std::fill(values, values + 512, 255);
	m_testMod = (m_testMod + 1) % 2;

	m_plugin->writeRange(m_output, 0, values, 512);
}

/*****************************************************************************
 * refresh
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
