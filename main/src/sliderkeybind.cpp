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

#include <ctype.h>
#include <qnamespace.h>
#include <qevent.h>

#include "sliderkeybind.h"
#include "virtualconsole.h"
#include "app.h"

extern App* _app;


//
// Constructor
//
SliderKeyBind::SliderKeyBind() : QObject()
{
  m_keyUp = Key_unknown;
  m_keyDown = Key_unknown;
  m_modUp = NoButton;
  m_modDown = NoButton;
  m_validUp = false;
  m_validDown = false;

  connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
	  this, SLOT(slotSliderKeyPressed(QKeyEvent*)));
}


//
// Construct with key and modifier
//
SliderKeyBind::SliderKeyBind(const int keyUp, const int modUp, const int keyDown, const int modDown) : QObject()
{
  m_keyUp = keyUp;
  m_modUp = modUp;
  m_keyDown = keyDown;
  m_modDown = modDown;

  if (keyUp >= 0 && keyUp < Key_unknown)
    {
      m_validUp = true;
    }
  else
    {
      m_validUp = false;
    }

  if (keyDown >= 0 && keyDown < Key_unknown)
    {
      m_validDown = true;
    }
  else
    {
      m_validDown = false;
    }

  connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
	  this, SLOT(slotSliderKeyPressed(QKeyEvent*)));
}


//
// Copy constructor
//
SliderKeyBind::SliderKeyBind(const SliderKeyBind* skb) : QObject()
{
  ASSERT(skb != NULL);
  m_keyUp = skb->keyUp();
  m_modUp = skb->modUp();
  m_keyDown = skb->keyDown();
  m_modDown = skb->modDown();
  m_validUp = skb->validUp();
  m_validDown = skb->validDown();


  connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
	  this, SLOT(slotSliderKeyPressed(QKeyEvent*)));
}


//
// Destructor
//
SliderKeyBind::~SliderKeyBind()
{
}


//
// Set the key up
//
void SliderKeyBind::setKeyUp(int keyUp)
{
  if (keyUp >= 0 && keyUp <= Key_unknown)
    {
      m_keyUp = keyUp;
      m_validUp = true;
    }
  else
    {
      keyUp = Key_unknown;
      m_validUp = false;
    }
}


//
// Set modifier up
//
void SliderKeyBind::setModUp(int modUp)
{
  m_modUp = modUp;
}

//
// Set the key down
//
void SliderKeyBind::setKeyDown(int keyDown)
{
  if (keyDown >= 0 && keyDown <= Key_unknown)
    {
      m_keyDown = keyDown;
      m_validDown = true;
    }
  else
    {
      keyDown = Key_unknown;
      m_validDown = false;
    }
}


//
// Set modifier down
//
void SliderKeyBind::setModDown(int modDown)
{
  m_modDown = modDown;
}

//
// Output the up key binding as a string
//
void SliderKeyBind::keyStringUp(int keyUp, int modUp, QString &string)
{
  QString modString = QString::null;
  QString keyString = QString::null;

  if (keyUp >= Key_F1 && keyUp <= Key_F35)
    {
      // Function keys
      keyString.sprintf("F%d", keyUp - Key_F1 + 1);
    }
  else if (keyUp >= Key_0 && keyUp <= Key_9)
    {
      // Number keys
      keyString.sprintf("%d", keyUp - Key_0);
    }
  else if (keyUp >= Key_A && keyUp <= Key_Z)
    {
      // A-Z
      keyString.sprintf("%c", 'A' + keyUp - Key_A);
    }
  else
    {
      switch(keyUp)
	{
	case Key_Exclam:
	  keyString.sprintf("!");
	  break;
	case Key_QuoteDbl:
	  keyString.sprintf("\"");
	  break;
	case Key_NumberSign:
	  keyString.sprintf("Unknown");
	  break;
	case Key_Dollar:
	  keyString.sprintf("$");
	  break;
	case Key_Percent:
	  keyString.sprintf("%%");
	  break;
	case Key_Ampersand:
	  keyString.sprintf("&");
	  break;
	case Key_Apostrophe:
	  keyString.sprintf("'");
	  break;
	case Key_ParenLeft:
	  keyString.sprintf("(");
	  break;
	case Key_ParenRight:
	  keyString.sprintf(")");
	  break;
	case Key_Asterisk:
	  keyString.sprintf("*");
	  break;
	case Key_Plus:
	  keyString.sprintf("+");
	  break;
	case Key_Comma:
	  keyString.sprintf(",");
	  break;
	case Key_Minus:
	  keyString.sprintf("-");
	  break;
	case Key_Period:
	  keyString.sprintf(".");
	  break;
	case Key_Slash:
	  keyString.sprintf("/");
	  break;
	case Key_Colon:
	  keyString.sprintf(":");
	  break;
        case Key_Semicolon:
	  keyString.sprintf(";");
	  break;
        case Key_Less:
	  keyString.sprintf("<");
	  break;
        case Key_Equal:
	  keyString.sprintf("/");
	  break;
        case Key_Greater:
	  keyString.sprintf(">");
	  break;
        case Key_Question:
	  keyString.sprintf("?");
	  break;
	case Key_BracketLeft:
	  keyString.sprintf("?");
	  break;
        case Key_Backslash:
	  keyString.sprintf("?");
	  break;
	case Key_BracketRight:
	  keyString.sprintf("?");
	  break;
	case Key_AsciiCircum:
	  keyString.sprintf("?");
	  break;
	case Key_Underscore:
	  keyString.sprintf("_");
	  break;
	case Key_QuoteLeft:
	  keyString.sprintf("`");
	  break;
	case Key_BraceLeft:
	  keyString.sprintf("{");
	  break;
	case Key_Bar:
	  keyString.sprintf("|");
	  break;
	case Key_BraceRight:
	  keyString.sprintf("}");
	  break;
	case Key_AsciiTilde:
	  keyString.sprintf("~");
	  break;
	case Key_At:
	  keyString.sprintf("@");
	  break;
	case Key_Space:
	  keyString.sprintf("Space");
	  break;
	case Key_Escape:
	  keyString.sprintf("Escape");
	  break;
	case Key_Return:
	  keyString.sprintf("Return");
	  break;
	case Key_Enter:
	  keyString.sprintf("Enter");
	  break;
	case Key_Insert:
	  keyString.sprintf("Insert");
	  break;
	case Key_Delete:
	  keyString.sprintf("Delete");
	  break;
	case Key_Pause:
	  keyString.sprintf("Pause");
	  break;
	case Key_Home:
	  keyString.sprintf("Home");
	  break;
	case Key_End:
	  keyString.sprintf("End");
          break;
	case Key_PageUp:
	  keyString.sprintf("PageUp");
	  break;
	case Key_PageDown:
	  keyString.sprintf("PageDown");
	  break;
	case Key_Left:
	  keyString.sprintf("Left");
          break;
	case Key_Right:
	  keyString.sprintf("Right");
          break;
	case Key_Up:
	  keyString.sprintf("Up");
          break;
	case Key_Down:
	  keyString.sprintf("Down");
          break;
	case Key_Shift:
	  keyString.sprintf("Shift +");
	  break;
	case Key_Alt:
	  keyString.sprintf("Alt +");
	  break;
	case Key_Control:
	  keyString.sprintf("Control +");
	  break;
	case 0:
	  keyString.sprintf("None");
	  break;
	case Key_unknown:
	  keyString.sprintf("Unknown");
	  break;
	default:
	  keyString.sprintf("Code %d", keyUp);
	  break;
	}
    }

  if (modUp & ShiftButton)
    {
      modString += QString("Shift + ");
    }

  if (modUp & AltButton)
    {
      modString += QString("Alt + ");
    }

  if (modUp & ControlButton)
    {
      modString += QString("Control + ");
    }

  if (keyUp <= 0 || keyUp > Key_unknown)
    {
      string = QString("None");
    }
  else
    {
      string = QString(modString + keyString);
    }
}

//
// Output the down key binding as a string
//
void SliderKeyBind::keyStringDown(int keyDown, int modDown, QString &string)
{
  QString modString = QString::null;
  QString keyString = QString::null;

  if (keyDown >= Key_F1 && keyDown <= Key_F35)
    {
      // Function keys
      keyString.sprintf("F%d", keyDown - Key_F1 + 1);
    }
  else if (keyDown >= Key_0 && keyDown <= Key_9)
    {
      // Number keys
      keyString.sprintf("%d", keyDown - Key_0);
    }
  else if (keyDown >= Key_A && keyDown <= Key_Z)
    {
      // A-Z
      keyString.sprintf("%c", 'A' + keyDown - Key_A);
    }
  else
    {
      switch(keyDown)
	{
	case Key_Exclam:
	  keyString.sprintf("!");
	  break;
	case Key_QuoteDbl:
	  keyString.sprintf("\"");
	  break;
	case Key_NumberSign:
	  keyString.sprintf("Unknown");
	  break;
	case Key_Dollar:
	  keyString.sprintf("$");
	  break;
	case Key_Percent:
	  keyString.sprintf("%%");
	  break;
	case Key_Ampersand:
	  keyString.sprintf("&");
	  break;
	case Key_Apostrophe:
	  keyString.sprintf("'");
	  break;
	case Key_ParenLeft:
	  keyString.sprintf("(");
	  break;
	case Key_ParenRight:
	  keyString.sprintf(")");
	  break;
	case Key_Asterisk:
	  keyString.sprintf("*");
	  break;
	case Key_Plus:
	  keyString.sprintf("+");
	  break;
	case Key_Comma:
	  keyString.sprintf(",");
	  break;
	case Key_Minus:
	  keyString.sprintf("-");
	  break;
	case Key_Period:
	  keyString.sprintf(".");
	  break;
	case Key_Slash:
	  keyString.sprintf("/");
	  break;
	case Key_Colon:
	  keyString.sprintf(":");
	  break;
        case Key_Semicolon:
	  keyString.sprintf(";");
	  break;
        case Key_Less:
	  keyString.sprintf("<");
	  break;
        case Key_Equal:
	  keyString.sprintf("/");
	  break;
        case Key_Greater:
	  keyString.sprintf(">");
	  break;
        case Key_Question:
	  keyString.sprintf("?");
	  break;
	case Key_BracketLeft:
	  keyString.sprintf("?");
	  break;
        case Key_Backslash:
	  keyString.sprintf("?");
	  break;
	case Key_BracketRight:
	  keyString.sprintf("?");
	  break;
	case Key_AsciiCircum:
	  keyString.sprintf("?");
	  break;
	case Key_Underscore:
	  keyString.sprintf("_");
	  break;
	case Key_QuoteLeft:
	  keyString.sprintf("`");
	  break;
	case Key_BraceLeft:
	  keyString.sprintf("{");
	  break;
	case Key_Bar:
	  keyString.sprintf("|");
	  break;
	case Key_BraceRight:
	  keyString.sprintf("}");
	  break;
	case Key_AsciiTilde:
	  keyString.sprintf("~");
	  break;
	case Key_At:
	  keyString.sprintf("@");
	  break;
	case Key_Space:
	  keyString.sprintf("Space");
	  break;
	case Key_Escape:
	  keyString.sprintf("Escape");
	  break;
	case Key_Return:
	  keyString.sprintf("Return");
	  break;
	case Key_Enter:
	  keyString.sprintf("Enter");
	  break;
	case Key_Insert:
	  keyString.sprintf("Insert");
	  break;
	case Key_Delete:
	  keyString.sprintf("Delete");
	  break;
	case Key_Pause:
	  keyString.sprintf("Pause");
	  break;
	case Key_Home:
	  keyString.sprintf("Home");
	  break;
	case Key_End:
	  keyString.sprintf("End");
          break;
	case Key_PageUp:
	  keyString.sprintf("PageUp");
	  break;
	case Key_PageDown:
	  keyString.sprintf("PageDown");
	  break;
	case Key_Left:
	  keyString.sprintf("Left");
          break;
	case Key_Right:
	  keyString.sprintf("Right");
          break;
	case Key_Up:
	  keyString.sprintf("Up");
          break;
	case Key_Down:
	  keyString.sprintf("Down");
          break;
	case Key_Shift:
	  keyString.sprintf("Shift +");
	  break;
	case Key_Alt:
	  keyString.sprintf("Alt +");
	  break;
	case Key_Control:
	  keyString.sprintf("Control +");
	  break;
	case 0:
	  keyString.sprintf("None");
	  break;
	case Key_unknown:
	  keyString.sprintf("Unknown");
	  break;
	default:
	  keyString.sprintf("Code %d", keyDown);
	  break;
	}
    }

  if (modDown & ShiftButton)
    {
      modString += QString("Shift + ");
    }

  if (modDown & AltButton)
    {
      modString += QString("Alt + ");
    }

  if (modDown & ControlButton)
    {
      modString += QString("Control + ");
    }

  if (keyDown <= 0 || keyDown > Key_unknown)
    {
      string = QString("None");
    }
  else
    {
      string = QString(modString + keyString);
    }
}

//
// Comparison between two KeyBind objects
//
bool SliderKeyBind::operator==(SliderKeyBind* skb)
{
  if (m_validUp == skb->validUp())
    {
    }
  else if (m_validDown == skb->validDown())
    {
    }
  else
    {
      return false;
    }

  if (m_keyUp == skb->keyUp() && m_modUp == skb->modUp())
    {
    }
  else if (m_keyDown == skb->keyDown() && m_modDown == skb->modDown())
    {
    }
  else
    {
      return false;
    }

  return true;
}

void SliderKeyBind::slotSliderKeyPressed(QKeyEvent* e)
{
  if (e->key() == m_keyUp && (e->state() == m_modUp))
    {
      emit pressedUp();
    }
  else if (e->key() == m_keyDown && (e->state() == m_modDown))
    {
      emit pressedDown();
    }
}
