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
#include "common/types.h"

class EventBuffer
{
public:
	/**
	 * Create a new EventBuffer instance.
	 *
	 * The buffer is a circular array of events that consists of
	 * t_buffer_data type values.
	 *
	 * Each value is an unsigned int, where bits 1-8 (byte) contain the DMX
	 * value and bits 9-24 (word) contain the channel (and universe).
	 *
	 * @param eventSize Tells the number of values (channels) per event
	 * @param buffersize Tells the max number of precalculated events
	 */
	EventBuffer(unsigned int eventSize, unsigned int bufferSize = 32);

	/**
	 * Destroy an EventBuffer instance
	 */
	~EventBuffer();
	
	/**
	 * Put data to the front of the ring buffer. This call will block
	 * (without busy waiting) if the buffer is full.
	 *
	 * @param event An array of values whose size is ALWAYS the same as
	 *              eventSize parameter given during construction.
	 */
	void put(t_buffer_data* event);
	
	/**
	 * Get the next value from rear of ring buffer if it is not empty
	 *
	 * @param event The next event taken from the rear of the list
	 * @return 0 for success, -1 if the list is empty
	 */
	int get(t_buffer_data* event);
	
	/**
	 * Make the ring buffer appear empty by setting the put and get
	 * positions (i.e. front & read, head & tail) to zero. This doesn't
	 * actually touch the buffer's contents but the result is exactly
	 * the same: the buffer appears empty.
	 */
	void purge();
	
	/**
	 * Get the buffer's event size i.e the number of channel values held
	 * in one event position.
	 *
	 * @return The buffer's event size
	 */
	unsigned int eventSize() const { return m_eventSize; }
	
protected:
	t_buffer_data* m_ring;
	
	unsigned int m_size;
	unsigned int m_eventSize;
	
	unsigned int m_filled; // Number of objects in the buffer
	unsigned int m_in; // Next token is stored here
	unsigned int m_out; // Next token is taken from here
	
	pthread_mutex_t m_mutex;
	pthread_cond_t m_nonEmpty;
	pthread_cond_t m_nonFull;
};

#endif
