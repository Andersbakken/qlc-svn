/*
  Q Light Controller
  virtualconsoleproperties.cpp

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

#include <qspinbox.h>
#include <qcheckbox.h>

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "settings.h"
#include "app.h"

extern App* _app;

VirtualConsoleProperties::VirtualConsoleProperties(QWidget* parent,
						   const char* name)
  : UI_VirtualConsoleProperties(parent, name, true)
{
}

VirtualConsoleProperties::~VirtualConsoleProperties()
{
}

void VirtualConsoleProperties::init()
{
  QString config;

  // Grab keyboard
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_GRABKB, config) != -1
	&& config == Settings::trueValue())
    {
      m_grabKeyboard->setChecked(true);
    }
  else
    {
      m_grabKeyboard->setChecked(false);
    }

  // Key repeat
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_KEYREPEAT, config) != -1
	&& config == Settings::trueValue())
    {
      m_keyRepeat->setChecked(true);
    }
  else
    {
      m_keyRepeat->setChecked(false);
    }

  // Snap to Grid
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_SNAPGRID, config) != -1
	&& config == Settings::trueValue())
    {
      m_snapToGrid->setChecked(true);
      slotSnapToGridToggled(true);
    }
  else
    {
      m_snapToGrid->setChecked(false);
      slotSnapToGridToggled(false);
    }

  // Grid X
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_GRIDX, config) != -1
	&& config != "")
    {
      m_xSpin->setValue(config.toInt());
    }
  else
    {
      m_xSpin->setValue(10);
    }

  // Grid Y
  if (_app->settings()->get(KEY_VIRTUAL_CONSOLE_GRIDY, config) != -1
	&& config != "")
    {
      m_ySpin->setValue(config.toInt());
    }
  else
    {
      m_ySpin->setValue(10);
    }
}

void VirtualConsoleProperties::slotSnapToGridToggled(bool state)
{
  m_xSpin->setEnabled(state);
  m_ySpin->setEnabled(state);
}

void VirtualConsoleProperties::slotOKClicked()
{
  QString config;

  // Grab keyboard
  _app->settings()->set(KEY_VIRTUAL_CONSOLE_GRABKB,
			(m_grabKeyboard->isChecked()) ? Settings::trueValue() :
			Settings::falseValue());

  // Key repeat
  _app->settings()->set(KEY_VIRTUAL_CONSOLE_KEYREPEAT,
			(m_keyRepeat->isChecked()) ? Settings::trueValue() :
			Settings::falseValue());

  // Snap to Grid
  _app->settings()->set(KEY_VIRTUAL_CONSOLE_SNAPGRID,
			(m_snapToGrid->isChecked()) ? Settings::trueValue() :
			Settings::falseValue());

  // Grid X
  _app->settings()->set(KEY_VIRTUAL_CONSOLE_GRIDX, m_xSpin->value());

  // Grid Y
  _app->settings()->set(KEY_VIRTUAL_CONSOLE_GRIDY, m_ySpin->value());

  accept();
}

void VirtualConsoleProperties::slotCancelClicked()
{
  reject();
}

bool VirtualConsoleProperties::isGridEnabled()
{
  return m_snapToGrid->isChecked();
}

int VirtualConsoleProperties::gridX()
{
  return m_xSpin->value();
}

int VirtualConsoleProperties::gridY()
{
  return m_ySpin->value();
}
