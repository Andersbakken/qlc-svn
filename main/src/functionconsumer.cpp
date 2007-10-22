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

#include "app.h"
#include "doc.h"
#include "dmxmap.h"
#include "function.h"
#include "eventbuffer.h"
#include "functionconsumer.h"

FunctionConsumer::FunctionConsumer(DMXMap* dmxMap) : QThread()
{
	Q_ASSERT(dmxMap != NULL);
	m_dmxMap = dmxMap;

	m_running = 0;
	m_fd = 0;
	m_timeCode = 0;

	/* Each fixture should fit inside one universe -> 512 values */
	m_event = new t_buffer_data[512];
	m_function = NULL;
	m_channel = 0;
}


FunctionConsumer::~FunctionConsumer()
{
	stop();
	delete [] m_event;
}


int FunctionConsumer::runningFunctions()
{
	int n = 0;
	m_functionListMutex.lock();
	n = m_functionList.count();
	m_functionListMutex.unlock();
	return n;
}


void FunctionConsumer::cue(Function* function)
{
	Q_ASSERT(function != NULL);
	
	m_functionListMutex.lock();
	m_functionList.append(function);
	m_functionListMutex.unlock();
}


void FunctionConsumer::purge()
{
	QPtrListIterator <Function> it(m_functionList);
	
	m_functionListMutex.lock();
	it.toFirst();
	while (it.current())
	{
		m_functionListMutex.unlock();
		it.current()->stop();
		
		m_functionListMutex.lock();
		++it;
	}
	m_functionListMutex.unlock();
	
	/* Wait until all functions have been stopped */
	while (m_functionList.count())
		pthread_yield();
}


void FunctionConsumer::timeCode(t_bus_value& timeCode)
{
	timeCode = m_timeCode;
}


void FunctionConsumer::stop()
{
	m_running = false;
	
	while (running())
		pthread_yield();
}


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
	m_timeCode = 0;
	
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
			m_timeCode++;
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


/**
 * Actual consumer function. Nothing should be allocated here
 * (not even local variables) to keep this as fast as possible.
 */
void FunctionConsumer::event(time_t)
{
	/* Lock before accessing the running functions list */
	m_functionListMutex.lock();
	
	for (m_function = m_functionList.first();
	     m_function != NULL;
	     m_function = m_functionList.next())
	{
		/* Unlock after accessing the running functions list */
		m_functionListMutex.unlock(); 
		
		if (m_function->eventBuffer()->get(m_event) == -1)
		{
			if (m_function->removeAfterEmpty())
			{
				/* Lock before remove */
				m_functionListMutex.lock(); 

				/* Remove the current function */
				m_functionList.remove();

				/* Cleanup after removal */
				m_function->cleanup();

				/* Unlock after remove */
				m_functionListMutex.unlock();
			}
		}
		else
		{
			/* Each event contains a channel and a value, making
			   eventSize() twice as big (for example 6 channels and
			   6 values) */
			for (m_channel = 0;
			     m_channel < m_function->eventBuffer()->eventSize();
			     m_channel++)
			{
				// Write also invalid values; let setValue()
				// take care of them
				m_dmxMap->setValue(m_event[m_channel] >> 8,
						   m_event[m_channel] & 0xFF);
			}
		}
		
		m_functionListMutex.lock(); // Lock for next round
	}
	
	m_functionListMutex.unlock(); // No more loops, unlock and get out
}
