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

#include <QCheckBox>
#include <QSpinBox>

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "app.h"

extern App* _app;

VirtualConsoleProperties::VirtualConsoleProperties(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

#if defined(__APPLE__) || defined (WIN32)
	m_hardwareTimerRadio->setEnabled(false);
	m_softwareTimerRadio->setChecked(true);
	m_hardwareTimerRadio->setToolTip(tr("Not available"));
#endif
}

VirtualConsoleProperties::~VirtualConsoleProperties()
{
}

/*****************************************************************************
 * Grid
 *****************************************************************************/

void VirtualConsoleProperties::setGrid(bool enabled, int x, int y)
{
	m_gridGroup->setChecked(enabled);
	m_gridXSpin->setValue(x);
	m_gridYSpin->setValue(y);
}

bool VirtualConsoleProperties::isGridEnabled()
{
	return m_gridGroup->isChecked();
}

int VirtualConsoleProperties::gridX()
{
	return m_gridXSpin->value();
}

int VirtualConsoleProperties::gridY()
{
	return m_gridYSpin->value();
}

/*****************************************************************************
 * Key repeat
 *****************************************************************************/

void VirtualConsoleProperties::setKeyRepeatOff(bool set)
{
	m_keyRepeatOffCheck->setChecked(set);
}

bool VirtualConsoleProperties::isKeyRepeatOff()
{
	return m_keyRepeatOffCheck->isChecked();
}

/*****************************************************************************
 * Grab keyboard
 *****************************************************************************/

void VirtualConsoleProperties::setGrabKeyboard(bool set)
{
	m_grabKeyboardCheck->setChecked(set);
}

bool VirtualConsoleProperties::isGrabKeyboard()
{
	return m_grabKeyboardCheck->isChecked();
}

/*****************************************************************************
 * Slider value ranges
 *****************************************************************************/

void VirtualConsoleProperties::setFadeLimits(t_bus_value low, t_bus_value high)
{
	m_fadeLowSpin->setValue(low);
	m_fadeHighSpin->setValue(high);
}

t_bus_value VirtualConsoleProperties::fadeLowLimit()
{
	return m_fadeLowSpin->value();
}

t_bus_value VirtualConsoleProperties::fadeHighLimit()
{
	return m_fadeHighSpin->value();
}

void VirtualConsoleProperties::setHoldLimits(t_bus_value low, t_bus_value high)
{
	m_holdLowSpin->setValue(low);
	m_holdHighSpin->setValue(high);
}

t_bus_value VirtualConsoleProperties::holdLowLimit()
{
	return m_holdLowSpin->value();
}

t_bus_value VirtualConsoleProperties::holdHighLimit()
{
	return m_holdHighSpin->value();
}

/*****************************************************************************
 * Timer type
 *****************************************************************************/

void VirtualConsoleProperties::setTimerType(FunctionConsumer::TimerType type)
{
	if (type == FunctionConsumer::RTCTimer)
		m_hardwareTimerRadio->setChecked(true);
	else
		m_softwareTimerRadio->setChecked(true);
}

FunctionConsumer::TimerType VirtualConsoleProperties::timerType()
{
	if (m_hardwareTimerRadio->isChecked() == true)
		return FunctionConsumer::RTCTimer;
	else
		return FunctionConsumer::NanoSleepTimer;
}
