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
      // Function keys
      key.sprintf("F%d", e->key() - Key_F1 + 1);
    }
  else if (e->key() >= Key_0 && e->key() <= Key_9)
    {
      // Number keys
      key.sprintf("%d", e->key() - Key_0);
    }
  else if (e->key() >= Key_A && e->key() <= Key_Z)
    {
      // A-Z
      key.sprintf("%c", 'A' + e->key() - Key_A);
    }
  else
    {
      switch(e->key())
	{
	case Key_Exclam:
	  key.sprintf("!");
	  break;
	case Key_QuoteDbl:
	  key.sprintf("\"");
	  break;
	case Key_NumberSign:
	  key.sprintf("Unknown");
	  break;
	case Key_Dollar:
	  key.sprintf("$");
	  break;
	case Key_Percent:
	  key.sprintf("\%");
	  break;
	case Key_Ampersand:
	  key.sprintf("&");
	  break;
	case Key_Apostrophe:
	  key.sprintf("'");
	  break;
	case Key_ParenLeft:
	  key.sprintf("(");
	  break;
	case Key_ParenRight:
	  key.sprintf(")");
	  break;
	case Key_Asterisk:
	  key.sprintf("*");
	  break;
	case Key_Plus:
	  key.sprintf("+");
	  break;
	case Key_Comma:
	  key.sprintf(",");
	  break;
	case Key_Minus:
	  key.sprintf("-");
	  break;
	case Key_Period:
	  key.sprintf(".");
	  break;
	case Key_Slash:
	  key.sprintf("/");
	  break;
	case Key_Colon:
	  key.sprintf(":");
	  break;
        case Key_Semicolon:
	  key.sprintf(";");
	  break;
        case Key_Less:
	  key.sprintf("<");
	  break;
        case Key_Equal:
	  key.sprintf("/");
	  break;
        case Key_Greater:
	  key.sprintf(">");
	  break;
        case Key_Question:
	  key.sprintf("?");
	  break;
	case Key_BracketLeft:
	  key.sprintf("?");
	  break;
        case Key_Backslash:
	  key.sprintf("?");
	  break;
	case Key_BracketRight:
	  key.sprintf("?");
	  break;
	case Key_AsciiCircum:
	  key.sprintf("?");
	  break;
	case Key_Underscore:
	  key.sprintf("_");
	  break;
	case Key_QuoteLeft:
	  key.sprintf("`");
	  break;
	case Key_BraceLeft:
	  key.sprintf("{");
	  break;
	case Key_Bar:
	  key.sprintf("|");
	  break;
	case Key_BraceRight:
	  key.sprintf("}");
	  break;
	case Key_AsciiTilde:
	  key.sprintf("~");
	  break;
	case Key_At:
	  key.sprintf("@");
	  break;
	case Key_Space:
	  key.sprintf("Space");
	  break;
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
	case 0:
	case Key_unknown:
	  key.sprintf("Unknown");
	  break;
	default:
	  key.sprintf("Code %d", e->key());
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

// Comparison between two KeyBind objects
bool KeyBind::operator=(KeyBind* kb)
{
  bool result = true;

  if (kb->receiver() != this->receiver())
    {
      return false;
    }

  if (this->keyEvent() && kb->keyEvent() && 
      kb->keyEvent()->key() == this->keyEvent()->key())
    {
      result = true;
    }
  else if (this->keyEvent() == NULL && kb->keyEvent() == NULL)
    {
      result = true;
    }
  else
    {
      return false;
    }

  if (this->pressAction() == kb->pressAction() &&
      this->releaseAction() == kb->releaseAction())
    {
      result = true;
    }
  else
    {
      return false;
    }

  return result;
}
