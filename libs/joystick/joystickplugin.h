/*
  Q Light Controller
  joystickplugin.h

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

#ifndef JOYSTICKPLUGIN_H
#define JOYSTICKPLUGIN_H

#include <qobject.h>
#include <qlist.h>
#include "joystick.h"

class JoystickPlugin;
struct PluginInfo;

extern "C" QObject* create();
extern "C" void destroy(QObject* object);

class JoystickPlugin : public QObject
{
  Q_OBJECT

 public:
  JoystickPlugin();
  virtual ~JoystickPlugin();

  virtual void info(PluginInfo &p);

  virtual void init();

  QList<Joystick> *joystickList() { return &m_joystickList; }

  virtual Joystick* selectJoystick();

  virtual Joystick* search(QString &device);

 private:
  QList <Joystick> m_joystickList;
};

#endif
