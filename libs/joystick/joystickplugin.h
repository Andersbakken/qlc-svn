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

#include "../common/plugin.h"
#include "joystick.h"
#include <qlist.h>

//
// Exported functions
//
extern "C" Plugin* create(int id);
extern "C" void destroy(Plugin* object);

//
// Class definition
//
class JoystickPlugin : public Plugin
{
  Q_OBJECT

 public:
  JoystickPlugin(int id);
  virtual ~JoystickPlugin();

  virtual bool open();
  virtual bool close();
  virtual bool isOpen();
  virtual void configure();
  virtual QString infoText();
  virtual void contextMenu(QPoint pos);

  virtual QList<Joystick> *joystickList() { return &m_joystickList; }

  virtual Joystick* selectJoystick();

  virtual Joystick* search(QString &device);

 protected slots:
  void slotContextMenuCallback(int);

 private:
  QList <Joystick> m_joystickList;
};

#endif
