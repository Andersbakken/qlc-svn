/*
  Q Light Controller
  deviceproperties.cpp
  
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

#include <qmessagebox.h>

#include "common/deviceclass.h"
#include "app.h"
#include "doc.h"
#include "deviceproperties.h"
#include "device.h"
#include "dmxaddresstool.h"

extern App* _app;

DeviceProperties::DeviceProperties(Device* device) 
  : UI_DeviceProperties(NULL, NULL, true)
{
  ASSERT(device != NULL);
  m_device = device;
} 

DeviceProperties::~DeviceProperties()
{
}

/**
 * Dialog pre-initialisation
 */
void DeviceProperties::init()
{
  // Name
  m_deviceNameEdit->setText(m_device->name());

  // Address
  m_addressSpin->setRange(1,
			  513 - m_device->deviceClass()->channels()->count());
  m_addressSpin->setValue(m_device->address() + 1);

  // Universe
  m_universeSpin->setRange(1, KUniverseCount);
  m_universeSpin->setValue(m_device->universe() + 1);
}

/**
 * Callback for DIP button click
 */
void DeviceProperties::slotDIPClicked()
{
  DMXAddressTool* dat = new DMXAddressTool(_app);
  dat->setAddress(m_addressSpin->value());
  if (dat->exec() == QDialog::Accepted)
    {
      if (dat->address() > 0)
	{
	  m_addressSpin->setValue(dat->address());
	}
      else
	{
	  m_addressSpin->setValue(1);
	}
    }

  delete dat;
}

/**
 * Callback for OK button click
 */
void DeviceProperties::slotOKClicked()
{
  // Name
  m_device->setName(m_deviceNameEdit->text());
  
  // Address
  m_device->setAddress(m_addressSpin->value() - 1);

  // Universe
  m_device->setUniverse(m_universeSpin->value() - 1);
  
  accept();
}
