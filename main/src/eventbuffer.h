#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include <pthread.h>
#include <vector>
#include "event.h"

class EventBuffer
{
 public:
  EventBuffer(unsigned int size, unsigned int eventSize);
  ~EventBuffer();

  bool put(unsigned char* event);
  unsigned char* get();

  unsigned long elapsedTime() { return m_elapsedTime; }

 private:
  unsigned char* m_ring;
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

/*
template<class Token> class RingBuffer
{
 public:
  RingBuffer(unsigned int size) :
    buffer(size), m_filled(0), m_in(0), m_out(0)
    {
      pthread_mutex_init(&m_mutex, 0);
      pthread_cond_init(&m_nonEmpty, 0);
      pthread_cond_init(&m_nonFull, 0);
    } // constructor
  
  ~RingBuffer()
    {
      pthread_mutex_destroy(&m_mutex);
      pthread_cond_destroy(&m_nonEmpty);
      pthread_cond_destroy(&m_nonFull);
    } // destructor
  
  void put(Token& token)
    {
      pthread_mutex_lock(&m_mutex);
      if (m_filled == buffer.size())
	{
	  pthread_cond_wait(&m_nonFull, &m_mutex);
	}
      
      assert(m_filled < buffer.size());
      buffer[m_in] = token;
      m_in = (m_in + 1) % buffer.size();
      ++m_filled;
      pthread_cond_signal(&m_nonEmpty);
      pthread_mutex_unlock(&m_mutex);
    } // add
  
  Token get()
    {
      Token token;
      
      pthread_mutex_lock(&m_mutex);
      if (m_filled == 0)
	{
	  pthread_mutex_unlock(&m_mutex);
	  return NULL;
	  //pthread_cond_wait(&m_nonEmpty, &m_mutex);
	}
      
      assert(m_filled > 0);
      token = buffer[m_out];
      m_out = (m_out + 1) % buffer.size();
      --m_filled;
      pthread_cond_signal(&m_nonFull);
      pthread_mutex_unlock(&m_mutex);
      
      return token;
    } // get
  
 private:
  std::vector<Token> buffer;
  unsigned int m_filled; // # of objects in the buffer
  unsigned int m_in; // next token is stored here
  unsigned int m_out; // next token is taken from here
  pthread_mutex_t m_mutex;
  pthread_cond_t m_nonEmpty;
  pthread_cond_t m_nonFull;

}; // class RingBuffer

typedef RingBuffer<Event*> EventBuffer;
*/

#endif
