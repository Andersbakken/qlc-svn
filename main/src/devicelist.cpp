/*
  Q Light Controller
  devicelist.cpp
  
  Copyright (C) 2005, Stefan Krumm, Heikki Junnila
  
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

#include <qlistview.h>
#include <qpushbutton.h>

#include "common/logicalchannel.h"
#include "devicelist.h"
#include "app.h"
#include "device.h"
#include "doc.h"
#include "consolechannel.h"

extern App* _app;

const int KColumnDevice      ( 0 );
const int KColumnChannelName ( 1 );

const int KColumnDeviceID    ( 2 );
const int KColumnChannelNum  ( 3 );

DeviceList::DeviceList(QWidget* parent, const char* name)
	: UI_DeviceList(parent, name, true),
	m_deviceID ( KNoID ),
	m_channel  ( KChannelInvalid)
{
}


DeviceList::~DeviceList()
{
}

void DeviceList::init()
{
	unsigned int n;
	QString did;
	QString s;
	QListViewItem* item;
	LogicalChannel* channel;
	
	m_listView->clear();
	
	for (t_device_id i = 0; i < KDeviceArraySize; i++)
	{
		Device* dev = _app->doc()->device(i);
		if (!dev)
		{
			continue;
		}
		else
		{
			did.setNum(dev->id());
			
			for (n = 0;
			     n < dev->deviceClass()->channels()->count();
			     n++)
			{
				// Device name
				item = new QListViewItem(m_listView);
				item->setText(KColumnDevice, dev->name());

				// Channel name
				channel = 
					dev->deviceClass()->channels()->at(n);
				
				if (channel)
				{
					s.sprintf("%.3d: ", n + 1);
					s += channel->name();
					item->setText(KColumnChannelName, s);
				}
				else
				{
					delete item;
					continue;
				}
				
				// Relative channel number (not shown)
				s.sprintf("%.3d", n);
				item->setText(KColumnChannelNum, s);

				// Device ID (not shown)
				item->setText(KColumnDeviceID, did);
			}   
		}
	}
	
	m_listView->setSelected(m_listView->firstChild(), true);
}

void DeviceList::slotSelectionChanged(QListViewItem* item)
{
	if (item)
	{
		m_deviceID = 
		static_cast<t_device_id> (item->text(KColumnDeviceID).toInt());
		
		m_channel =
		static_cast<t_channel> (item->text(KColumnChannelNum).toInt());
		
		m_ok->setEnabled(true);
	}
	else
	{
		m_deviceID = KNoID;
		m_channel = KChannelInvalid;
		
		m_ok->setEnabled(false);
	}
}

void DeviceList::slotItemDoubleClicked(QListViewItem* item)
{
	slotSelectionChanged(item);
	accept();
}
