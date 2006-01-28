/*
  Q Light Controller
  configuremidiout.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#include "configuresoundtolight.h"
#include "soundtolight.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <unistd.h>

ConfigureSoundToLight::ConfigureSoundToLight(SoundToLight* plugin)
  : UI_ConfigureSoundToLight(NULL, NULL, true)
{
  ASSERT(plugin != NULL);
  m_plugin = plugin;

  m_deviceEdit->setText(m_plugin->deviceName());

  updateStatus();
}

ConfigureSoundToLight::~ConfigureSoundToLight()
{

}

QString ConfigureSoundToLight::device()
{
  return m_deviceEdit->text();
}

void ConfigureSoundToLight::slotActivateClicked()
{
  if (m_plugin->deviceName() != m_deviceEdit->text())
    {
      m_plugin->setDeviceName(m_deviceEdit->text());
    }

  m_plugin->activate();

  ::usleep(10);  // Allow the activation signal get passed to doc

  updateStatus();
}

void ConfigureSoundToLight::updateStatus()
{
  if (m_plugin->isOpen())
    {
      m_statusLabel->setText("Active");
      m_deviceEdit->setEnabled(false);
      m_activate->setEnabled(false);
    }
  else
    {
      m_statusLabel->setText("Not Active");
      m_deviceEdit->setEnabled(true);
      m_activate->setEnabled(true);
    }
}


