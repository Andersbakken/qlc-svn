/*
  Q Light Controller
  keybind.h
  
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

#ifndef KEYBIND_H
#define KEYBIND_H

#include <qobject.h>

class KeyBind : public QObject
{
  Q_OBJECT

 public:
  KeyBind();
  KeyBind(int key, int mod);
  KeyBind(KeyBind* kb);
  ~KeyBind();

  enum PressAction { PressStart = 0, PressToggle = 1, PressStepForward = 2,
		     PressStepBackward = 3, PressStop = 4, PressNothing = 5 };

  enum ReleaseAction { ReleaseStop = 0, ReleaseNothing = 1 };

  static QString keyString(int key, int mod);
  QString keyString() { return keyString(m_key, m_mod); }

  int key() { return m_key; }
  void setKey(int key, int mod);

  int mod() { return m_mod; }

  void setPressAction(PressAction a) { m_pressAction = a; }
  PressAction pressAction() { return m_pressAction; }

  void setReleaseAction(ReleaseAction a) { m_releaseAction = a; }
  ReleaseAction releaseAction() { return m_releaseAction; }

  bool valid() { return m_valid; }

  bool operator==(KeyBind*);

 private:
  int m_key; // Key
  int m_mod; // Modifier [shift|alt|control]

  bool m_valid; // Does this object contain a valid key or not

  PressAction m_pressAction;
  ReleaseAction m_releaseAction;
};

#endif
