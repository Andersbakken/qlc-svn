/*
  Q Light Controller
  functionconsumer.cpp

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

#include <stdlib.h>
#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "functionconsumer.h"
#include "function.h"
#include "eventbuffer.h"
#include "app.h"
#include "doc.h"

extern App* _app;

t_value FunctionConsumer::KNoSetMask ( 0xff00 );

FunctionConsumer::FunctionConsumer() : QThread()
{
  m_running = 0;
  m_fd = 0;
}

FunctionConsumer::~FunctionConsumer()
{
  stop();
}

//
// Clear function producer list to stop all running functions
//
void FunctionConsumer::purge()
{
  Function* p = NULL;
  m_functionListMutex.lock(); // Lock before access

  while (! m_functionList.isEmpty() )
    {
      p = m_functionList.take(0);
      p->stop();
    }

  m_functionListMutex.unlock(); // Unlock after access
}

//
// Add a function producer to producer list to run it
//
void FunctionConsumer::cue(Function* f)
{
  ASSERT(f);

  m_functionListMutex.lock(); // Lock before access
  m_functionList.append(f);
  m_functionListMutex.unlock(); // Unlock after append
}

//
// Stop the function consumer
//
void FunctionConsumer::stop()
{
  m_running = false;

  while (running());
}

//
// Set up the consumer's timer etc.
//
void FunctionConsumer::init()
{
  int retval = -1;
  unsigned long tmp = 0;

  m_fd = open("/dev/rtc", O_RDONLY);
  if (m_fd < 0)
    {
      // open failed
      perror("open /dev/rtc");
      
      // but we try an alternate location of the device
      m_fd = open("/dev/misc/rtc", O_RDONLY);
      if (m_fd < 0)
        {
          // failed again, we give up and exit
          perror("open /dev/misc/rtc");
          ::exit(errno);
        }
    }

  retval = ioctl(m_fd, RTC_IRQP_SET, KFrequency);
  if (retval == -1)
    {
      perror("ioctl");
      ::exit(errno);
    }

  retval = ioctl(m_fd, RTC_IRQP_READ, &tmp);
  if (retval == -1)
    {
      perror("ioctl");
    }
  else
    {
      qDebug("\nPeriodic IRQ rate is %ldHz.", tmp);
    }

  retval = ioctl(m_fd, RTC_PIE_ON, 0);
  if (retval == -1)
    {
      perror("ioctl");
      ::exit(errno);
    }
  else
    {
      qDebug("Started RTC interrupt");
    }
}


void FunctionConsumer::run()
{
  int retval = -1;
  unsigned long data = 0;

  m_running = true;

  while (m_running == true)
    {
      retval = read(m_fd, &data, sizeof(unsigned long));
      if (retval == -1)
        {
          perror("read");
          ::exit(errno);
        }
      else
        {
          event(data);
        }
    }

  // Set interrupts off
  retval = ioctl(m_fd, RTC_PIE_OFF, 0);
  if (retval == -1)
    {
      perror("RTC_PIE_OFF");
    }
  else
    {
      qDebug("Stopped RTC interrupt");
    }

  close(m_fd);
}


void FunctionConsumer::event(time_t)
{
  Function* f = NULL;
  t_value* ev = NULL;
  t_channel ch = 0;

  QPtrListIterator<Function> it(m_functionList);

  m_functionListMutex.lock(); // First lock
  while ( (f = it.current()) != 0 )
    {
      ++it;

      m_functionListMutex.unlock(); // Unlock after using iterator it

      ev = f->eventBuffer()->get();

      if ( !ev )
        {
          if (f->removeAfterEmpty())
            {
              m_functionListMutex.lock(); // Lock before remove
              m_functionList.remove(f);
	      f->freeRunTimeData();
              m_functionListMutex.unlock(); // Unlock after remove
            }
        }
      else
        {
	  for (ch = 0; ch < (t_channel) f->eventBuffer()->eventSize(); ch++)
	    {
	      if ((f->type() == Function::Scene) && 
		  (((Scene*) f)->channelValue(ch).type == Scene::NoSet))
		{
		  // Don't write NoSet values
		}
	      else
		{
		  _app->outputPlugin()
		    ->writeChannel(f->device()->address() + ch, ev[ch]);
		}
	    }
        }

      m_functionListMutex.lock(); // Lock for next round
    }

  m_functionListMutex.unlock(); // No more loops, unlock and get out
}
