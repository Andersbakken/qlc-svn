/*
  Q Light Controller
  selectjoystick.h

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

#ifndef SELECTJOYSTICK_H
#define SELECTJOYSTICK_H

#include "uic_selectjoystick.h"

class JoystickPlugin;
class Joystick;

class SelectJoystick : public UI_SelectJoystick
{
  Q_OBJECT

 public:
  SelectJoystick(JoystickPlugin* plugin);
  virtual ~SelectJoystick();

  void initView();

  Joystick* joystick() { return m_currentJoystick; }

 public slots:
  void slotJoystickSelected(QListViewItem* item);
  void slotOKClicked();
  void slotCancelClicked();
  void slotCalibrateClicked();

 private:
  void fillList();

 private:
  JoystickPlugin* m_plugin;
  Joystick* m_currentJoystick;
};

#endif
