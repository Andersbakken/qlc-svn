/*
  Q Light Controller - Fixture Definition Editor
  editcapability.cpp

  Copyright (C) Heikki Junnila

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

#include <QSpinBox>
#include <QLineEdit>

#include "common/qlccapability.h"
#include "editcapability.h"

EditCapability::EditCapability(QWidget* parent, const QLCCapability* cap)
	: QDialog(parent)
{
	m_capability = new QLCCapability(cap);

	setupUi(this);

	m_minSpin->setValue(m_capability->min());
	m_maxSpin->setValue(m_capability->max());
	m_descriptionEdit->setText(m_capability->name());
	m_minSpin->setFocus();
	m_minSpin->selectAll();

	connect(m_minSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotMinSpinChanged(int)));
	connect(m_maxSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotMaxSpinChanged(int)));
	connect(m_descriptionEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotDescriptionEdited(const QString&)));
}

EditCapability::~EditCapability()
{
	if (m_capability != NULL)
		delete m_capability;
}

void EditCapability::slotMinSpinChanged(int value)
{
	m_capability->setMin(value);
}

void EditCapability::slotMaxSpinChanged(int value)
{
	m_capability->setMax(value);
}

void EditCapability::slotDescriptionEdited(const QString& text)
{
	m_capability->setName(text);
}

