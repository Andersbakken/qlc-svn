/*
  Q Light Controller
  device.cpp
  
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

#include "app.h"
#include "doc.h"
#include "device.h"
#include "containerview.h"
#include "functionsview.h"
#include "channelui.h"
#include "function.h"

extern App* _app;

static int _nextDeviceId = 1;

Device::Device(int address, DeviceClass* dc, const QString& name)
{
  m_id = (_nextDeviceId << 10);
  _nextDeviceId++;

  m_name = QString(name);
  m_address = address;

  m_deviceClass = dc;
  m_console = NULL;
  m_monitor = NULL;
  m_functionsView = NULL;
}

Device::~Device()
{
}

void Device::setName(QString name)
{
  m_name = name;
}

void Device::setDeviceClass(DeviceClass* dc)
{
  m_deviceClass = dc;
}

Function* Device::searchFunction(const QString &fname)
{
  Function* function;
  function = NULL;
  
  Function* f = NULL;
  for (f = m_functions.first(); f != NULL; f = m_functions.next())
    {
      if (f->name() == fname)
	{
	  function = f;
	  break;
	}
    }

  return function;
}

QList<Function> Device::functions() const 
{ 
  return m_functions;
}













