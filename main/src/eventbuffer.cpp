/*
  Q Light Controller
  eventbuffer.cpp

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

#include "assert.h"
#include "eventbuffer.h"

#include <qapplication.h>
#include <malloc.h>
#include <string.h>

/**
 * \brief Constructor
 *
 * The buffer is a circular list that contains events,
 * which, contain value pairs representing a channel and its value:
 *
 * <ev1 [ch1 val1] ... [chn valn]> ... <evn [ch1 val1] ... [chn valn]>
 *
 * \param eventSize Tells the number of values per event
 * \param buffersize Tells the number of events i.e. buffer size.
 * \param byteSize Tells the size of individual values to store
 */

// eventSize = number of values per event
// bufferSize = maximum event count per buffer
EventBuffer::EventBuffer(unsigned int eventSize,
			 unsigned int bufferSize,
			 unsigned int byteSize)
  :
  m_ring       ( NULL ),
  m_size       ( bufferSize * (eventSize * byteSize) ),
  m_eventSize  ( eventSize * byteSize ),
  m_byteSize   ( byteSize ),
  m_filled     ( 0 ),
  m_in         ( 0 ),
  m_out        ( 0 )
{

  m_ring = new t_value[m_size];

  pthread_mutex_init(&m_mutex, 0);
  pthread_cond_init(&m_nonEmpty, 0);
  pthread_cond_init(&m_nonFull, 0);
}

/**
 * \brief Destructor
 */
EventBuffer::~EventBuffer()
{
  delete [] m_ring;

  pthread_mutex_destroy(&m_mutex);
  pthread_cond_destroy(&m_nonEmpty);
  pthread_cond_destroy(&m_nonFull);
}


/**
 * Put a new value to the front of the buffer if it is not full.
 *
 * \param ev Event to put into the buffer's tail
 */
int EventBuffer::put(t_value* ev)
{
  pthread_mutex_lock(&m_mutex);
  if (m_filled == m_size)
    {
      pthread_cond_wait(&m_nonFull, &m_mutex);
    }

  assert(m_filled < m_size);
  memcpy(m_ring + m_in, ev, m_eventSize);
  m_in = (m_in + m_eventSize) % m_size;
  m_filled += m_eventSize;
  pthread_cond_signal(&m_nonEmpty);
  pthread_mutex_unlock(&m_mutex);

  return 0;
}


/**
 * Get the next value from rear of list if it is not empty
 *
 * \param event The next event taken from the head of the list
 */
int EventBuffer::get(t_value* event)
{
  pthread_mutex_lock(&m_mutex);
  if (m_filled == 0)
    {
      pthread_mutex_unlock(&m_mutex);
      return -1;
    }

  assert(m_filled > 0);
  memcpy(event, m_ring + m_out, m_eventSize);
  m_out = (m_out + m_eventSize) % m_size;
  m_filled -= m_eventSize;
  pthread_cond_signal(&m_nonFull);
  pthread_mutex_unlock(&m_mutex);

  return 0;
}


/**
 * Empty the list by setting the put and get positions
 * to zero. This doesn't actually touch the contents but the
 * result is exactly the same: the buffer looks empty.
 */
void EventBuffer::purge()
{
  pthread_mutex_lock(&m_mutex);

  m_filled = 0;
  m_in = 0;
  m_out = 0;

  pthread_mutex_unlock(&m_mutex);
}
