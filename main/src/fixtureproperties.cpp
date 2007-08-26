/*
  Q Light Controller
  fixtureproperties.cpp
  
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

#include <qlineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "fixtureproperties.h"

#include "common/qlcfixturedef.h"
#include "app.h"
#include "doc.h"
#include "fixture.h"

extern App* _app;

FixtureProperties::FixtureProperties(QWidget* parent, t_fixture_id fixture)
	: UI_FixtureProperties(parent, "Fixture Properties", true)
{
	m_fixture = fixture;
}

FixtureProperties::~FixtureProperties()
{
}

void FixtureProperties::init()
{
	Fixture* fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	// Name
	m_nameEdit->setText(fxi->name());

	// Address
	m_addressSpin->setRange(1, 513 - fxi->channels());
	m_addressSpin->setValue(fxi->address() + 1);

	// Universe
	m_universeSpin->setRange(1, KUniverseCount);
	m_universeSpin->setValue(fxi->universe() + 1);
}

void FixtureProperties::slotOKClicked()
{
	Fixture* fxi = _app->doc()->fixture(m_fixture);
	Q_ASSERT(fxi != NULL);

	// Name
	fxi->setName(m_nameEdit->text());
  
	// Address
	fxi->setAddress(m_addressSpin->value() - 1);

	// Universe
	fxi->setUniverse(m_universeSpin->value() - 1);
  
	accept();
}
