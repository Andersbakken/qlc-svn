/*
  Q Light Controller
  fixturelist.cpp
  
  Copyright (c) Stefan Krumm, Heikki Junnila
  
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

#include "common/qlcchannel.h"
#include "fixturelist.h"
#include "consolechannel.h"
#include "app.h"
#include "fixture.h"
#include "doc.h"

extern App* _app;

const int KColumnFixtureName ( 0 );
const int KColumnChannelName ( 1 );

const int KColumnFixtureID   ( 2 );
const int KColumnChannelNum  ( 3 );

FixtureList::FixtureList(QWidget* parent)
	: UI_FixtureList(parent, "Fixture List", true),
	  m_fixture ( KNoID ),
	  m_channel ( KChannelInvalid)
{
}


FixtureList::~FixtureList()
{
}

void FixtureList::init()
{
	QListViewItem* item = NULL;
	QLCChannel* channel = NULL;
	Fixture* fxi = NULL;
	unsigned int n = 0;
	QString fxi_id;
	QString s;
	
	m_listView->clear();
	
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		fxi = _app->doc()->fixture(i);
		if (fxi == NULL)
			continue;

		fxi_id.setNum(fxi->id());
		
		for (n = 0; n < fxi->channels(); n++)
		{
			// Create a new item for a channel
			item = new QListViewItem(m_listView);

			// Fixture name
			item->setText(KColumnFixtureName, fxi->name());
			
			// Channel name
			channel = fxi->channel(n);
			if (channel != NULL)
			{
				s.sprintf("%.3d: ", n + 1);
				s += channel->name();
				item->setText(KColumnChannelName, s);
			}
			else
			{
				delete item;
				break;
			}
			
			// Relative channel number (not shown)
			s.sprintf("%.3d", n);
			item->setText(KColumnChannelNum, s);
			
			// Fixture ID (not shown)
			item->setText(KColumnFixtureID, fxi_id);
		}   
	}
	
	m_listView->setSelected(m_listView->firstChild(), true);
}

void FixtureList::slotSelectionChanged(QListViewItem* item)
{
	if (item != NULL)
	{
		m_fixture = static_cast<t_fixture_id> (
			item->text(KColumnFixtureID).toInt());
		
		m_channel = static_cast<t_channel> (
			item->text(KColumnChannelNum).toInt());
		
		m_ok->setEnabled(true);
	}
	else
	{
		m_fixture = KNoID;
		m_channel = KChannelInvalid;
		
		m_ok->setEnabled(false);
	}
}

void FixtureList::slotItemDoubleClicked(QListViewItem* item)
{
	slotSelectionChanged(item);
	accept();
}
