/*
  Q Light Controller
  keybind.cpp
  
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

#include "keybind.h"
#include "dmxwidgetbase.h"
#include <ctype.h>

KeyBind::KeyBind() : QObject()
{
  m_keyEvent = NULL;
  m_receiver = NULL;
  m_pressAction = PressStart;
  m_releaseAction = ReleaseNothing;
}

KeyBind::KeyBind(QKeyEvent* e) : QObject()
{
  m_keyEvent = new QKeyEvent(QEvent::KeyPress, e->key(), e->ascii(), e->state());
 
  m_receiver = NULL;
  m_pressAction = PressStart;
  m_releaseAction = ReleaseNothing;
}

KeyBind::KeyBind(KeyBind* kb, DMXWidgetBase* recv) : QObject()
{
  ASSERT(kb != NULL);
  m_keyEvent = new QKeyEvent(QEvent::KeyPress, kb->keyEvent()->key(), 
			     kb->keyEvent()->ascii(), kb->keyEvent()->state());
  
  m_receiver = recv;
  m_pressAction = kb->pressAction();
  m_releaseAction = kb->releaseAction();
}

KeyBind::~KeyBind()
{
}

void KeyBind::setKeyEvent(const QKeyEvent* e)
{
  if (m_keyEvent != NULL)
    {
      delete m_keyEvent;
      m_keyEvent = NULL;
    }

  m_keyEvent = new QKeyEvent(QEvent::KeyPress, e->key(), e->ascii(), e->state());
}

QString KeyBind::keyString(const QKeyEvent* e)
{
  QString mod = QString::null;
  QString key = QString::null;

  if (e == NULL)
    {
      return QString::null;
    }

  if (e->key() >= Key_F1 && e->key() <= Key_F12)
    {
      key.sprintf("F%d", e->key() - Key_F1 + 1);
    }
  else if (e->key() < Key_Escape)
    {
      if (e->key() == Key_Space)
	{
	  key.sprintf("Space");
	}
      else
	{
	  key.sprintf("%c", toupper(e->ascii()));
	}
    }
  else
    {
      switch(e->key())
	{
	case Key_Escape:
	  key.sprintf("Escape");
	  break;
	case Key_Return:
	  key.sprintf("Return");
	  break;
	case Key_Enter:
	  key.sprintf("Enter");
	  break;
	case Key_Insert:
	  key.sprintf("Insert");
	  break;
	case Key_Delete:
	  key.sprintf("Delete");
	  break;
	case Key_Pause:
	  key.sprintf("Pause");
	  break;
	case Key_Home:
	  key.sprintf("Home");
	  break;
	case Key_End:
	  key.sprintf("End");
          break;
	case Key_PageUp:
	  key.sprintf("PageUp");
	  break;
	case Key_PageDown:
	  key.sprintf("PageDown");
	  break;
	case Key_Left:
	  key.sprintf("Left");
          break;
	case Key_Right:
	  key.sprintf("Right");
          break;
	case Key_Up:
	  key.sprintf("Up");
          break;
	case Key_Down:
	  key.sprintf("Down");
          break;
	case Key_Shift:
	  key.sprintf("Shift +");
	  break;
	case Key_Alt:
	  key.sprintf("Alt +");
	  break;
	case Key_Control:
	  key.sprintf("Control +");
	  break;
	default:
	  key.sprintf("Unknown");
	  break;
	}
    }

  if (e->state() & ShiftButton)
    {
      mod += QString("Shift + ");
    }
  
  if (e->state() & AltButton)
    {
      mod += QString("Alt + ");
    }

  if (e->state() & ControlButton)
    {
      mod += QString("Control + ");
    }

  return QString(mod + key);
}
