/*
  Q Light Controller
  function.cpp
  
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

#include "function.h"
#include "dmxdevice.h"
#include "deviceclass.h"

unsigned long Function::_nextFunctionId = 0;

Function::Function() : QObject()
{
  m_id = _nextFunctionId;
  _nextFunctionId++;

  m_name = QString::null;
  m_type = Function::Undefined;
  m_deviceClass = NULL;
  m_device = NULL;

  m_running = false;
}

Function::~Function()
{
  emit destroyed();
}

QString Function::name() const
{
   return QString(m_name);
}

void Function::setName(QString name)
{
  m_name = QString(name);
}

DeviceClass* Function::deviceClass() const
{
  return m_deviceClass;
}

void Function::setDeviceClass(DeviceClass* dc)
{
  m_deviceClass = dc;
  m_device = NULL;
}

DMXDevice* Function::device() const
{
  return m_device;
}

void Function::setDevice(DMXDevice* device)
{
  m_device = device;
  m_deviceClass = NULL;
}

Function::Type Function::type() const
{
  return m_type;
}

bool Function::unRegisterFunction(Feeder* feeder)
{
  emit unRegistered(this);

  return true;
}

bool Function::registerFunction(Feeder* feeder)
{
  return true;
}

QString Function::typeString() const
{
  switch (m_type)
    {
    case Collection:
      return QString("Collection");
      break;

    case Scene:
      return QString("Scene");
      break;

    case Chaser:
      return QString("Chaser");
      break;

    case Sequence:
      return QString("Sequence");
      break;

    default:
      return QString("Undefined");
      break;
    }
}
