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

  bool put(t_value* event);
  t_value* get();

  unsigned long elapsedTime() { return m_elapsedTime; }

 private:
  t_value* m_ring;
  unsigned int m_size;
  unsigned int m_eventSize;

  unsigned int m_filled; // # of objects in the buffer
  unsigned int m_in; // next token is stored here
  unsigned int m_out; // next token is taken from here

  unsigned long m_elapsedTime;

  pthread_mutex_t m_mutex;
  pthread_cond_t m_nonEmpty;
  pthread_cond_t m_nonFull;
};

#endif
