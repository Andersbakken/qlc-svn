/*
  Q Light Controller
  eventbuffer.h
  
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

#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include "event.h"

#define EVENT_BUFFER_SIZE 256

class EventBuffer
{
 public:
  EventBuffer();
  ~EventBuffer();

  bool put(Event*); /* false if buffer is full */
  Event* get(void); /* null if buffer is empty */

 private:
  Event* m_buffer[EVENT_BUFFER_SIZE];

  bool m_mutex;

  void ENTER_CRITICAL();
  void EXIT_CRITICAL();

  int m_headPos;
  int m_tailPos;
};

#endif
