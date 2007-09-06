/*
  Q Light Controller
  virtualconsoleproperties.cpp

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

#include <qspinbox.h>
#include <qcheckbox.h>

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "app.h"

extern App* _app;

VirtualConsoleProperties::VirtualConsoleProperties(QWidget* parent)
	: UI_VirtualConsoleProperties(parent,
				      "Virtual Console Properties",
				      true)
{
}

VirtualConsoleProperties::~VirtualConsoleProperties()
{
}

void VirtualConsoleProperties::init()
{
	m_grabKeyboardCheck->setChecked(m_grabKeyboard);
	m_keyRepeatOffCheck->setChecked(m_keyRepeatOff);

	m_gridEnabledCheck->setChecked(m_gridEnabled);
	m_gridXSpin->setValue(m_gridX);
	m_gridYSpin->setValue(m_gridY);

	slotGridEnabledCheckToggled(m_gridEnabled);
}

void VirtualConsoleProperties::slotGridEnabledCheckToggled(bool state)
{
	m_gridXSpin->setEnabled(state);
	m_gridYSpin->setEnabled(state);
}

void VirtualConsoleProperties::slotOKClicked()
{
	m_grabKeyboard = m_grabKeyboardCheck->isChecked();
	m_keyRepeatOff = m_keyRepeatOffCheck->isChecked();

	m_gridEnabled = m_gridEnabledCheck->isChecked();
	m_gridX = m_gridXSpin->value();
	m_gridY = m_gridYSpin->value();

	accept();
}

void VirtualConsoleProperties::slotCancelClicked()
{
	reject();
}
