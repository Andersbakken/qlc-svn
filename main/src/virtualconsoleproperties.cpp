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

#include "app.h"
#include "virtualconsoleproperties.h"
#include "virtualconsole.h"

extern App* _app;

VirtualConsoleProperties::VirtualConsoleProperties(QWidget* parent)
	: UI_VirtualConsoleProperties(parent,
				      "Virtual Console Properties",
				      true)
{
	m_grabKeyboard = false;
	m_keyRepeatOff = false;
	m_gridEnabled = false;
	m_gridX = 1;
	m_gridY = 1;
	m_fadeLowLimit = 0;
	m_fadeHighLimit = 0;
	m_holdLowLimit = 0;
	m_holdHighLimit = 0;
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

	m_fadeLowSpin->setValue(m_fadeLowLimit);
	m_fadeHighSpin->setValue(m_fadeHighLimit);

	m_holdLowSpin->setValue(m_holdLowLimit);
	m_holdHighSpin->setValue(m_holdHighLimit);
}

/*****************************************************************************
 * Slider value ranges
 *****************************************************************************/

void VirtualConsoleProperties::setFadeLimits(t_bus_value low, t_bus_value high)
{
	m_fadeLowLimit = low;
	m_fadeHighLimit = high;
}

void VirtualConsoleProperties::fadeLimits(t_bus_value& low, t_bus_value& high)
{
	low = m_fadeLowLimit;
	high = m_fadeHighLimit;
}

void VirtualConsoleProperties::setHoldLimits(t_bus_value low, t_bus_value high)
{
	m_holdLowLimit = low;
	m_holdHighLimit = high;
}

void VirtualConsoleProperties::holdLimits(t_bus_value& low, t_bus_value& high)
{
	low = m_holdLowLimit;
	high = m_holdHighLimit;
}

void VirtualConsoleProperties::slotGridEnabledCheckToggled(bool state)
{
	m_gridXSpin->setEnabled(state);
	m_gridYSpin->setEnabled(state);
}

void VirtualConsoleProperties::accept()
{
	m_grabKeyboard = m_grabKeyboardCheck->isChecked();
	m_keyRepeatOff = m_keyRepeatOffCheck->isChecked();

	m_gridEnabled = m_gridEnabledCheck->isChecked();
	m_gridX = m_gridXSpin->value();
	m_gridY = m_gridYSpin->value();

	m_fadeLowLimit = m_fadeLowSpin->value();
	m_fadeHighLimit = m_fadeHighSpin->value();

	m_holdLowLimit = m_holdLowSpin->value();
	m_holdHighLimit = m_holdHighSpin->value();

	UI_VirtualConsoleProperties::accept();
}
