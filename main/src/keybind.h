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

#include "classes.h"
#include <qevent.h>

class KeyBind : public QObject
{
  Q_OBJECT

 public:
  KeyBind();
  KeyBind(QKeyEvent* e);
  KeyBind(KeyBind* kb, DMXWidgetBase* recv = NULL);
  ~KeyBind();

  enum PressAction { PressStart = 0, PressToggle = 1, PressNothing = 2 };
  enum ReleaseAction { ReleaseStop = 0, ReleaseNothing = 1 };

  void setReceiver(DMXWidgetBase* widget) { m_receiver = widget; }
  const DMXWidgetBase* receiver() { return (const DMXWidgetBase*) m_receiver; }

  static QString keyString(const QKeyEvent* e);
  QString keyString() { return keyString(m_keyEvent); }

  const QKeyEvent* keyEvent() { return (const QKeyEvent*) m_keyEvent; }
  void setKeyEvent(const QKeyEvent* e);

  void setPressAction(PressAction a) { m_pressAction = a; }
  PressAction pressAction() { return m_pressAction; }

  void setReleaseAction(ReleaseAction a) { m_releaseAction = a; }
  ReleaseAction releaseAction() { return m_releaseAction; }

 private:
  DMXWidgetBase* m_receiver;
  QKeyEvent* m_keyEvent;

  PressAction m_pressAction;
  ReleaseAction m_releaseAction;
};

#endif
