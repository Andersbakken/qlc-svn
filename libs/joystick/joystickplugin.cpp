/*
  Q Light Controller
  joystickplugin.cpp

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

#include "joystickplugin.h"
#include "joystick.h"
#include "../common/plugininfo.h"
#include "selectjoystick.h"

#define JS_MAX_NUM         32

// The only two exported functions

extern "C" QObject* create()
{
  return new JoystickPlugin;
}

extern "C" void destroy(QObject* object)
{
  delete object;
}

///////////////////////////////////////
// class JoystickPlugin implementation

JoystickPlugin::JoystickPlugin()
{
}

JoystickPlugin::~JoystickPlugin()
{
  while (m_joystickList.isEmpty() == false)
    {
      delete m_joystickList.take(0);
    }
}

void JoystickPlugin::info(PluginInfo &p)
{
  p.name = QString("Generic joystick plugin");
  p.type = QString(JOYSTICK_TYPE_STRING);
  p.version = 0x00000100; /* (Version 0.0.1-0 == 0x00 00 01 00) */
}

void JoystickPlugin::init()
{
  unsigned int i = 0;
  char fileName[256];
  Joystick* j = NULL;

  for (i = 0; i < JS_MAX_NUM; i++)
    {
      sprintf(fileName, "/dev/js%d", i);
      j = new Joystick();
      j->create((const char*) fileName);

      if (j->valid() == true)
	{
	  m_joystickList.append(j);
	}
      else
	{
	  delete j;
	  j = NULL;
	}
    }
}

Joystick* JoystickPlugin::selectJoystick()
{
  SelectJoystick* sj = new SelectJoystick(this);
  sj->initView();

  if (sj->exec() == QDialog::Accepted)
    {
      return sj->joystick();
    }
  else
    {
      return NULL;
    }
}

Joystick* JoystickPlugin::search(QString &device)
{
  for (Joystick* j = m_joystickList.first(); j != NULL; j = m_joystickList.next())
    {
      if (j->fdName() == device)
	{
	  return j;
	}
    }
  
  return NULL;
}
