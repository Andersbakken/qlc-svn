/*
  Q Light Controller
  fixtureproperties.h
  
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

#ifndef FIXTUREPROPERTIES_H
#define FIXTUREPROPERTIES_H

#include "uic_fixtureproperties.h"
#include "common/types.h"

class Fixture;

class FixtureProperties : public UI_FixtureProperties
{
	Q_OBJECT

 public:
	/** Constructor */
	FixtureProperties(QWidget* parent, t_fixture_id fixture);

	/** Destructor */
	~FixtureProperties();
   
	/** Dialog pre-initialisation */
	void init();
   
 protected slots:
	 /** Callback for OK button click */
	void slotOKClicked();

 protected:
	t_fixture_id m_fixture;
};

#endif




























































