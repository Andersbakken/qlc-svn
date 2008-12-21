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

#include <QMessageBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"

#include "fixtureproperties.h"
#include "addfixture.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

FixtureProperties::FixtureProperties(QWidget* parent, t_fixture_id fxi_id)
	: QDialog(parent)
{
	setupUi(this);

	Fixture* fxi = _app->doc()->fixture(fxi_id);
	Q_ASSERT(fxi != NULL);
	m_fxi = fxi;
	m_fixtureDef = m_fxi->fixtureDef();
	m_fixtureMode = m_fxi->fixtureMode();
	m_channels = m_fxi->channels();

	// Name
	m_nameEdit->setText(m_fxi->name());
	setWindowTitle(tr("Fixture properties - ") + m_fxi->name());
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));

	// Address
	m_addressSpin->setRange(1, 513 - m_fxi->channels());
	m_addressSpin->setValue(m_fxi->address() + 1);

	// Universe
	m_universeSpin->setRange(1, KUniverseCount);
	m_universeSpin->setValue(m_fxi->universe() + 1);

	// Make & Model
	connect(m_makeModelButton, SIGNAL(clicked()),
		this, SLOT(slotMakeModelClicked()));

	if (m_fixtureDef != NULL && m_fixtureMode != NULL)
	{
		m_makeModelEdit->setText(QString("%1 - %2")
					 .arg(m_fixtureDef->manufacturer())
					 .arg(m_fixtureDef->model()));
	}
	else
	{
		m_makeModelEdit->setText("Generic - Dimmer");
	}
}

FixtureProperties::~FixtureProperties()
{
}

void FixtureProperties::slotNameEdited(const QString& text)
{
	Q_ASSERT(m_fxi != NULL);
	setWindowTitle(tr("Fixture properties - ") + text);
}

void FixtureProperties::slotMakeModelClicked()
{
	AddFixture af(this, m_fxi->fixtureDef());
	af.setWindowTitle(tr("Change fixture definition"));
	if (af.exec() == QDialog::Accepted)
	{
		m_fixtureDef = af.fixtureDef();
		m_fixtureMode = af.mode();
		m_channels = af.channels();
		if (m_fixtureDef != NULL && m_fixtureMode != NULL)
		{
			m_makeModelEdit->setText(QString("%1 - %2")
						 .arg(m_fixtureDef->manufacturer())
						 .arg(m_fixtureDef->model()));
		}
		else
		{
			m_makeModelEdit->setText("Generic - Dimmer");
		}
	}
}

void FixtureProperties::accept()
{
	Q_ASSERT(m_fxi != NULL);

	// Name
	m_fxi->setName(m_nameEdit->text());

	// Address
	m_fxi->setAddress(m_addressSpin->value() - 1);

	// Universe
	m_fxi->setUniverse(m_universeSpin->value() - 1);

	// Make & Model
	if (m_fixtureDef == NULL && m_fixtureMode == NULL)
	{
		/* Generic dimmer */
		m_fxi->setChannels(m_channels);
	}
	else if (m_fixtureDef != m_fxi->fixtureDef() ||
		 m_fixtureMode != m_fxi->fixtureMode())
	{
		m_fxi->setFixtureDefinition(m_fixtureDef, m_fixtureMode);
	}

	QDialog::accept();
}
