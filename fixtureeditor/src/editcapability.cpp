/*
  Q Light Controller
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

#include <qwidget.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include "editcapability.h"
#include "common/qlccapability.h"

EditCapability::EditCapability(QWidget* parent, QLCCapability* cap)
	: UI_EditCapability(parent)
{
	m_capability = new QLCCapability(cap);
}

EditCapability::~EditCapability()
{
	if (m_capability != NULL)
		delete m_capability;
}

void EditCapability::init()
{
	m_minSpin->setValue(m_capability->min());
	m_maxSpin->setValue(m_capability->max());
	m_nameEdit->setText(m_capability->name());
}

void EditCapability::slotMinSpinChanged(int value)
{
	m_capability->setMin(value);
}

void EditCapability::slotMaxSpinChanged(int value)
{
	m_capability->setMax(value);
}

void EditCapability::slotNameChanged(const QString& text)
{
	m_capability->setName(text);
}

