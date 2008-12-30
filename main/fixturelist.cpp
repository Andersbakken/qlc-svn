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

#include <QTreeWidgetItem>
#include <QTreeWidget>

#include "common/qlcchannel.h"

#include "consolechannel.h"
#include "fixturelist.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnFixtureName 0
#define KColumnChannelName 1
#define KColumnFixtureID   2
#define KColumnChannelNum  3

/*****************************************************************************
 * Initialization
 *****************************************************************************/
FixtureList::FixtureList(QWidget* parent) : QDialog(parent)
{
	m_fixture = KNoID;
	m_channel = KChannelInvalid;

	setupUi(this);

	init();
}

FixtureList::~FixtureList()
{
}

void FixtureList::init()
{
	QTreeWidgetItem* item;
	
	m_listView->clear();

	connect(m_listView, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectionChanged()));
	connect(m_listView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotItemDoubleClicked()));
	
	for (t_fixture_id fxi_id = 0; fxi_id < KFixtureArraySize; fxi_id++)
	{
		Fixture* fxi = _app->doc()->fixture(fxi_id);
		if (fxi == NULL)
			continue;

		for (unsigned int n = 0; n < fxi->channels(); n++)
		{
			const QLCChannel* channel;
			QString s;

			// Create a new item for a channel
			item = new QTreeWidgetItem(m_listView);

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
			item->setText(KColumnFixtureID,
				      QString("%1").arg(fxi_id));
		}   
	}
	
	/* Select the first item */
	item = m_listView->topLevelItem(0);
	if (item != NULL)
		item->setSelected(true);
}

/*****************************************************************************
 * Fixture list slots
 *****************************************************************************/

void FixtureList::slotSelectionChanged()
{
	QTreeWidgetItem* item = m_listView->currentItem();
	if (item != NULL)
	{
		m_fixture = static_cast<t_fixture_id> (
			item->text(KColumnFixtureID).toInt());
		
		m_channel = static_cast<t_channel> (
			item->text(KColumnChannelNum).toInt());
	}
	else
	{
		m_fixture = KNoID;
		m_channel = KChannelInvalid;
	}
}

void FixtureList::slotItemDoubleClicked()
{
	slotSelectionChanged();
	accept();
}
