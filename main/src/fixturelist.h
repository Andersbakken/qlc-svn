/*
  Q Light Controller
  fixturelist.h
  
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

#ifndef FIXTURELIST_H
#define FIXTURELIST_H

#include "common/types.h"
#include "uic_fixturelist.h"

class FixtureList : public UI_FixtureList
{
	Q_OBJECT

 public:
	FixtureList(QWidget* parent);
	~FixtureList();

	void init();
	
	/** Return the ID of the selected channel's fixture */
	t_fixture_id selectedFixtureID() const { return m_fixture; }

	/** Return the selected channel number */
	t_channel selectedChannel() const { return m_channel; }

 public slots:
	void slotSelectionChanged(QListViewItem* item);
        void slotItemDoubleClicked(QListViewItem* item);

 protected:
	t_fixture_id m_fixture;
	t_channel m_channel;
};

#endif
