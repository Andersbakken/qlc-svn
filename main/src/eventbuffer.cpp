/*
  Q Light Controller
  eventbuffer.cpp
  
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

#include <unistd.h>
#include "eventbuffer.h"

EventBuffer::EventBuffer()
{
  m_headPos = 0;
  m_tailPos = 0;
  m_mutex = false;
}

EventBuffer::~EventBuffer()
{

}

void EventBuffer::ENTER_CRITICAL()
{
  while (m_mutex == true);
  m_mutex = true;
}

void EventBuffer::EXIT_CRITICAL()
{
  m_mutex = false;
}

Event* EventBuffer::get()
{
  Event* event;
  event = NULL;

  if (m_headPos == m_tailPos)
    {
      event = NULL;
    }
  else
    {
      ENTER_CRITICAL();

      event = m_buffer[m_headPos];
      m_buffer[m_headPos] = NULL;
      m_headPos = (m_headPos + 1) % EVENT_BUFFER_SIZE;

      EXIT_CRITICAL();
    }

  return event;
}

bool EventBuffer::put(Event* event)
{
  ENTER_CRITICAL();

  m_tailPos = (m_tailPos + 1) % EVENT_BUFFER_SIZE;

  EXIT_CRITICAL();

  if (m_tailPos == m_headPos)
    {
      return false;
    }

  ENTER_CRITICAL();

  m_buffer[m_tailPos] = event;

  EXIT_CRITICAL();
  
  return true;
}
