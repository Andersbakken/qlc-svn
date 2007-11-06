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

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionConsumer::FunctionConsumer(DMXMap* dmxMap) : QThread()
{
	Q_ASSERT(dmxMap != NULL);
	m_dmxMap = dmxMap;

	m_timerType = RTCTimer;
	m_running = 0;
	m_fdRTC = -1;
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

/*****************************************************************************
 * Timer type
 *****************************************************************************/

bool FunctionConsumer::setTimerType(TimerType type)
{
	bool result = false;

	switch (type)
	{
	case RTCTimer:
		/* Test that we can use RTC */
		result = openRTC();
		if (result == true)
			closeRTC();
		m_timerType = type;
		break;

	default:
	case USleepTimer:
		/* Test that we can use USleep */
		result = openUSleepTimer();
		if (result == true)
			closeUSleepTimer();
		m_timerType = type;
		break;
	}

	return result;
}

/*****************************************************************************
 * RTC timer
 *****************************************************************************/

bool FunctionConsumer::openRTC()
{
	int retval = -1;
	unsigned long tmp = 0;
	
	m_fdRTC = open("/dev/rtc", O_RDONLY);
	if (m_fdRTC == -1)
	{
		/* open failed */
		qWarning("Unable to open /dev/rtc: %s", strerror(errno));
		
		/* but we try an alternate location of the device */
		m_fdRTC = open("/dev/misc/rtc", O_RDONLY);
		if (m_fdRTC == -1)
		{
			/* failed again, we give up */
			qWarning("Unable to open /dev/misc/rtc: %s",
				 strerror(errno));
			closeRTC();
			return false;
		}
	}
	
	/* Attempt to set RTC frequency to KFrequency (64Hz) */
	retval = ioctl(m_fdRTC, RTC_IRQP_SET, KFrequency);
	if (retval == -1)
	{
		qWarning("Unable to set %ldHz to RTC: %s", strerror(errno));
		closeRTC();
		return false;
	}
	
	/* Attempt to read RTC frequency (should now be 64Hz) */
	retval = ioctl(m_fdRTC, RTC_IRQP_READ, &tmp);
	if (retval == -1)
	{
		qWarning("Unable to read RTC timer frequency: %s",
			 strerror(errno));
		closeRTC();
		return false;
	}
	else
	{
		qDebug("RTC timer frequency is %ldHz.", tmp);
	}

	/* Check that the frequency is correct */
	if (tmp == KFrequency)
	{
		qDebug("RTC opened");
		return true;
	}
	else
	{
		qWarning("Unable to set RTC frequency to %ldHz", KFrequency);
		closeRTC();
		return false;
	}
}

bool FunctionConsumer::closeRTC()
{
	int retval = -1;

	if (m_fdRTC == -1)
		return true;

	/* Attempt to close RTC file descriptor */
	retval = close(m_fdRTC);
	if (retval == -1)
	{
		qWarning("Unable to close RTC file descriptor: %s",
			 strerror(errno));
		return false;
	}
	else
	{
		qDebug("RTC closed");
		m_fdRTC = -1;
		return true;
	}
}

bool FunctionConsumer::startRTC()
{
	int retval = -1;

	if (m_fdRTC == -1)
	{
		qWarning("Unable to start RTC: Device is not open.");
		return false;
	}

	/* Attempt to start periodic timer interrupts */
	retval = ioctl(m_fdRTC, RTC_PIE_ON, 0);
	if (retval == -1)
	{
		qWarning("Unable to switch RTC interrupts ON: %s",
			 strerror(errno));
		return false;
	}
	else
	{
		qDebug("RTC started");
		return true;
	}
}

bool FunctionConsumer::stopRTC()
{
	int retval = -1;

	/* Attempt to stop periodic interrupts */
	retval = ioctl(m_fdRTC, RTC_PIE_OFF, 0);
	if (retval == -1)
	{
		qWarning("Unable to switch RTC interrupts OFF: %s",
			 strerror(errno));
		return false;
	}
	else
	{
		qDebug("RTC stopped");
		return true;
	}
}

void FunctionConsumer::runRTC()
{
	int retval = -1;
	unsigned long data = 0;
	
	if (openRTC() == false)
		return;

	if (startRTC() == false)
		return;

	while (m_running == true)
	{
		retval = read(m_fdRTC, &data, sizeof(unsigned long));
		if (retval == -1)
		{
			qWarning("Unable to read from RTC: %s",
				 strerror(errno));
			m_running = false;
		}
		else
		{
			m_timeCode++;
			event(data);
		}
	}
	
	stopRTC();
	closeRTC();
}

/*****************************************************************************
 * USleep timer
 *****************************************************************************/

bool FunctionConsumer::openUSleepTimer()
{
	return true;
}

bool FunctionConsumer::closeUSleepTimer()
{
	return true;
}

bool FunctionConsumer::startUSleepTimer()
{
	return true;
}

bool FunctionConsumer::stopUSleepTimer()
{
	return true;
}

void FunctionConsumer::runUSleepTimer()
{
	/* How long to wait each loop */
	int tickTime = 1000000 / KFrequency;

	/* Allocate this from stack here so that GCC doesn't have
	   to do it everytime implicitly when gettimeofday() is called */
	int tod = 0;

	/* Allocate all the memory at the start so we don't waste any time */
	timeval* finish = static_cast<timeval*> (malloc(sizeof(timeval)));
	timeval* current = static_cast<timeval*> (malloc(sizeof(timeval)));
	timeval* prev = static_cast<timeval*> (malloc(sizeof(timeval)));
	long sleepTime = 0;

	/* This is the start time for the timer */
	tod = gettimeofday(finish, NULL);
	if (tod == -1)
	{
		qWarning("Unable to get the time accurately: %s",
			 strerror(errno));
		m_running = false;
	}

	memcpy(prev, finish, sizeof(timeval));

	while (m_running == true)
	{
		/* Increment the finish time for this loop */
		finish->tv_sec += (finish->tv_usec + tickTime) / 1000000;
		finish->tv_usec = (finish->tv_usec + tickTime) % 1000000;
		
		tod = gettimeofday(current, NULL);
		if (tod == -1)
		{
			qWarning("Unable to get the current time: %s",
				 strerror(errno));
			m_running = false;
			break;
		}
		
		/* Do a rough sleep using the kernel to return control */
		sleepTime = finish->tv_usec - current->tv_usec +
			(finish->tv_sec - current->tv_sec) * 1000000 - 1000;
		if (sleepTime > 0)
			usleep(sleepTime);

		/* Now take full CPU for precision (only a few milliseconds,
		   at maximum 1000 milliseconds) */
		while (finish->tv_usec - current->tv_usec + 
		       (finish->tv_sec - current->tv_sec) * 1000000 > 5)
		{
			tod = gettimeofday(current, NULL);
			if (tod == -1)
			{
				qWarning("Unable to get the current time: %s",
					 strerror(errno));
				m_running = false;
				break;
			}
		}

		m_timeCode++;

		event(current->tv_sec);
	}

	free(finish);
	free(current);
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

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


void FunctionConsumer::run()
{
	m_timeCode = 0;
	m_running = true;

	switch (m_timerType)
	{
	case RTCTimer:
		runRTC();
		break;
		
	default:
	case USleepTimer:
		runUSleepTimer();
		break;
	}
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
			for (m_channel = 0;
			     m_channel < m_function->eventBuffer()->eventSize();
			     m_channel++)
			{
				/* Write also invalid values; let setValue()
				   take care of them */
				m_dmxMap->setValue(m_event[m_channel] >> 8,
						   m_event[m_channel] & 0xFF);
			}
		}
		
		/* Lock for next round */
		m_functionListMutex.lock(); 
	}
	
	/* No more loops, unlock and get out */
	m_functionListMutex.unlock();
}
