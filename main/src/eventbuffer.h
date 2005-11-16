/*
  Q Light Controller
  eventbuffer.h

  Copyright (C) Heikki Junnila

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

#include <pthread.h>
#include "types.h"

class EventBuffer
{
 public:
  EventBuffer(unsigned int eventSize,
	      unsigned int bufferSize = 32);
  ~EventBuffer();

  // Put data to buffer
  int put(t_buffer_data* event);

  // Read data from buffer
  int get(t_buffer_data* event);

  // Clear the buffer
  void purge();

  // Eventsize == channel count
  unsigned int eventSize() const { return m_eventSize; }

 private:
  t_buffer_data* m_ring;

  unsigned int m_size;
  unsigned int m_eventSize;

  unsigned int m_filled; // # of objects in the buffer
  unsigned int m_in; // next token is stored here
  unsigned int m_out; // next token is taken from here

  pthread_mutex_t m_mutex;
  pthread_cond_t m_nonEmpty;
  pthread_cond_t m_nonFull;
};

#endif
