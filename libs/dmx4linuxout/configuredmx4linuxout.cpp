/*
  Q Light Controller
  configuredmx4linuxout.cpp
  
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

#include "configuredmx4linuxout.h"
#include "dmx4linuxout.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>

ConfigureDMX4LinuxOut::ConfigureDMX4LinuxOut(DMX4LinuxOut* plugin) 
  : UI_ConfigureDMX4LinuxOut(NULL, NULL, true)
{
  ASSERT(plugin != NULL);
  m_plugin = plugin;

  m_deviceEdit->setText(m_plugin->deviceName());
}

ConfigureDMX4LinuxOut::~ConfigureDMX4LinuxOut()
{

}

QString ConfigureDMX4LinuxOut::device()
{
  return m_deviceEdit->text();
}

void ConfigureDMX4LinuxOut::slotActivateClicked()
{
  m_plugin->activate();

  if (m_plugin->isOpen())
    {
      m_statusLabel->setText("Active");
    }
  else
    {
      m_statusLabel->setText("Not Active");
    }
}
