/*
  Q Light Controller
  vcxypadfixture.cpp

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

#include <QCheckBox>
#include <QSpinBox>
#include <QDialog>
#include <QString>

#include "vcxypadfixtureeditor.h"
#include "vcxypadfixture.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixtureEditor::VCXYPadFixtureEditor(QWidget* parent,
			   QList <VCXYPadFixture> fixtures) : QDialog(parent)
{
	setupUi(this);

	m_fixtures = fixtures;

	/* Take initial values from the first fixture */
	if (fixtures.count() > 0)
	{
		VCXYPadFixture fxi = fixtures.first();

		m_xMin->setValue(int(fxi.xMin() * 100));
		m_xMax->setValue(int(fxi.xMax() * 100));
		m_xReverse->setChecked(fxi.xReverse());

		m_yMin->setValue(int(fxi.yMin() * 100));
		m_yMax->setValue(int(fxi.yMax() * 100));
		m_yReverse->setChecked(fxi.yReverse());
	}
}

VCXYPadFixtureEditor::~VCXYPadFixtureEditor()
{
}

void VCXYPadFixtureEditor::slotXMinChanged(int value)
{
	if (value >= m_xMax->value())
		m_xMax->setValue(value + 1);
}

void VCXYPadFixtureEditor::slotXMaxChanged(int value)
{
	if (value <= m_xMin->value())
		m_xMin->setValue(value - 1);
}

void VCXYPadFixtureEditor::slotYMinChanged(int value)
{
	if (value >= m_yMax->value())
		m_yMax->setValue(value + 1);
}

void VCXYPadFixtureEditor::slotYMaxChanged(int value)
{
	if (value <= m_yMin->value())
		m_yMin->setValue(value - 1);
}

void VCXYPadFixtureEditor::accept()
{
	/* Put dialog values to all fixtures */
	QMutableListIterator <VCXYPadFixture> it(m_fixtures);
	while (it.hasNext() == true)
	{
		VCXYPadFixture fxi(it.next());

		fxi.setX(m_xMin->value() / 100.0, m_xMax->value() / 100.0,
			 m_xReverse->isChecked());
		fxi.setY(m_yMin->value() / 100.0, m_yMax->value() / 100.0,
			 m_yReverse->isChecked());

		it.setValue(fxi);
	}

	QDialog::accept();
}
