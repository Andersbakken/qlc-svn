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

class QKeyEvent;

class KeyBind : public QObject
{
  Q_OBJECT

 public:
  KeyBind();
  KeyBind(const int key, const int mod);
  KeyBind(const KeyBind* kb);
  ~KeyBind();

  enum PressAction { PressStart = 0, PressToggle = 1, PressStepForward = 6,
		     PressStepBackward = 3, PressStop = 4, PressNothing = 5,
		     PressFlash = 2 };

  enum ReleaseAction { ReleaseStop = 0, ReleaseNothing = 1 };

  static void keyString(int key, int mod, QString &string);
  void keyString(QString &string) { return keyString(m_key, m_mod, string); }

  int key() const { return m_key; }
  void setKey(int key);

  int mod() const { return m_mod; }
  void setMod(int mod);

  void setPressAction(PressAction a) { m_pressAction = a; }
  PressAction pressAction() const { return m_pressAction; }

  void setReleaseAction(ReleaseAction a) { m_releaseAction = a; }
  ReleaseAction releaseAction() const { return m_releaseAction; }

  bool valid() const { return m_valid; }

  bool operator==(KeyBind*);

 signals:
  void pressed();
  void released();

 public slots:
  void slotKeyPressed(QKeyEvent* e);
  void slotKeyReleased(QKeyEvent* e);

 private:
  int m_key; // Key
  int m_mod; // Modifier [shift|alt|control]

  bool m_valid; // Does this object contain a valid key or not

  PressAction m_pressAction;
  ReleaseAction m_releaseAction;
};

#endif
