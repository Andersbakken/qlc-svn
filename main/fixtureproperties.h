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

#include <QDialog>

#include "ui_fixtureproperties.h"
#include "common/qlctypes.h"

class Fixture;

class FixtureProperties : public QDialog, public Ui_FixtureProperties
{
	Q_OBJECT

public:
	/** Constructor */
	FixtureProperties(QWidget* parent, t_fixture_id fxi_id);

	/** Destructor */
	~FixtureProperties();

protected slots:
	/** Name edited */
	void slotNameEdited(const QString& text);

	/** QDialog accept() slot for OK button clicks */
	void accept();

protected:
	Fixture* m_fxi;
};

#endif




























































