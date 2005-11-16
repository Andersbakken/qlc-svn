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

#include "vcxypadproperties.h"
#include "devicelist.h"
#include "app.h"
#include "device.h"
#include "doc.h"
#include "consolechannel.h"
#include "logicalchannel.h"
extern App* _app;

const int KColumnDevice     ( 0 );
const int KColumnChannel    ( 1 );
const int KColumnCapability ( 2 );
const int KColumnDMX        ( 3 );
const int KColumnUniverse   ( 4 );
const int KColumnID         ( 5 );

DeviceList::DeviceList(QWidget* parent, const char* name)
                  : UI_DeviceList(parent, name, true)
{
}


DeviceList::~DeviceList()
{
}

void DeviceList::init()
{
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
	  QString address;
	  address.sprintf("%.3d", dev->address() + 1);
	  
	  m_listView->setAllColumnsShowFocus(true);
	  m_listView->setColumnWidthMode(4,QListView::Manual);
	  m_listView->setColumnWidth(4,0);
	  
	  unsigned int n = 0;
	  while(n < dev->deviceClass()->channels()->count())
	    {
	       QListViewItem *newSubItem =  new QListViewItem(m_listView);
	       newSubItem->setText(KColumnDevice, " " + dev->name() + " ");

	       newSubItem->setText(KColumnCapability,
		        dev->deviceClass()->channels()->at(n)->name() + " ");

	       QString s;
	       s.sprintf("%.3d", dev->address() + n + 1);
	       newSubItem->setText(KColumnDMX, address);

	       s.sprintf("%.3d", dev->universe() + 1);
	       newSubItem->setText(KColumnUniverse, s);

	       s.sprintf("%.3d", n + 1);
	       newSubItem->setText(KColumnChannel, s);

	       s.sprintf("%d", dev->id());
	       newSubItem->setText(KColumnID, s);

	       n++;
	    }   
	}
     }	
     m_listView->setCurrentItem(m_listView->firstChild());
}

