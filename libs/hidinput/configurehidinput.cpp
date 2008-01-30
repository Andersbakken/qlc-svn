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

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <qtimer.h>

#include "configurehidinput.h"
#include "hidinput.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureHIDInput::ConfigureHIDInput(QWidget* parent, HIDInput* plugin)
	: UI_ConfigureHIDInput(parent, "Configure HIDInput", true)
{
	Q_ASSERT(plugin != NULL);
	m_plugin = plugin;

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
	m_plugin->open();
	refreshList();
}

void ConfigureHIDInput::refreshList()
{
	HIDDevice* dev = NULL;
	QString s;

	m_listView->clear();

	for (unsigned int i = 0; i < m_plugin->m_devices.count(); i++)
	{
		dev = m_plugin->device(i);
		Q_ASSERT(dev != NULL);

		s.setNum(i + 1);
		new QListViewItem(m_listView, s, dev->name());
	}
}

