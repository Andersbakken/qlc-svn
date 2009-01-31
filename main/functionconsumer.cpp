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

#include <QDebug>

#ifdef X11
#include <linux/rtc.h>
#endif

#ifndef WIN32
#include <sys/ioctl.h>
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "outputmap.h"
#include "function.h"
#include "app.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionConsumer::FunctionConsumer(QObject* parent, OutputMap* outputMap)
	: QThread(parent)
{
	Q_ASSERT(outputMap != NULL);
	m_outputMap = outputMap;

#ifndef __APPLE__
#ifdef X11
	m_timerType = RTCTimer;
	m_fdRTC = -1;
#else
	m_timerType = NanoSleepTimer;
#endif
#else
	m_timerType = NanoSleepTimer;
#endif

	m_running = false;

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

	stop();

	switch (type)
	{
#ifndef __APPLE__
#ifdef X11
	case RTCTimer:
		/* Test that we can use RTC */
		result = openRTC();
		if (result == true)
			closeRTC();
		m_timerType = type;
		break;
#endif
#endif
	default:
	case NanoSleepTimer:
		/* Test that we can use NanoSleep */
		result = openNanoSleepTimer();
		if (result == true)
			closeNanoSleepTimer();
		m_timerType = type;
		break;
	}

	start();

	return result;
}

/*****************************************************************************
 * RTC timer
 *****************************************************************************/

#ifdef X11
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
		qWarning("Unable to set %ldHz to RTC: %s",
			 KFrequency, strerror(errno));
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
			event();
		}
	}

	stopRTC();
	closeRTC();
}
#endif /* X11 */

/*****************************************************************************
 * NanoSleep timer
 *****************************************************************************/

bool FunctionConsumer::openNanoSleepTimer()
{
	return true;
}

bool FunctionConsumer::closeNanoSleepTimer()
{
	return true;
}

bool FunctionConsumer::startNanoSleepTimer()
{
	return true;
}

bool FunctionConsumer::stopNanoSleepTimer()
{
	return true;
}

#ifndef WIN32
void FunctionConsumer::runNanoSleepTimer()
{
	/* How long to wait each loop */
	int tickTime = 1000000 / KFrequency;

	/* Allocate this from stack here so that GCC doesn't have
	   to do it everytime implicitly when gettimeofday() is called */
	int tod = 0;

	/* Allocate all the memory at the start so we don't waste any time */
	timeval* finish = static_cast<timeval*> (malloc(sizeof(timeval)));
	timeval* current = static_cast<timeval*> (malloc(sizeof(timeval)));
	timespec* sleepTime = static_cast<timespec*> (malloc(sizeof(timespec)));
	timespec* remainingTime = static_cast<timespec*> (malloc(sizeof(timespec)));

	sleepTime->tv_sec = 0;

	/* This is the start time for the timer */
	tod = gettimeofday(finish, NULL);
	if (tod == -1)
	{
		qWarning("Unable to get the time accurately: %s",
			 strerror(errno));
		m_running = false;
	}

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

		/* Do a rough sleep using the kernel to return control.
		   We know that this will never be seconds as we are dealing
		   with jumps of under a second every time. */
		sleepTime->tv_nsec = 
			((finish->tv_usec - current->tv_usec) * 1000) +
			((finish->tv_sec - current->tv_sec) * 1000000000) - 1000;
		if (sleepTime->tv_nsec > 0)
		{
			tod = nanosleep(sleepTime, remainingTime);
			while (tod == -1 && sleepTime->tv_nsec > 100) {
				sleepTime->tv_nsec = remainingTime->tv_nsec;
				tod = nanosleep(sleepTime, remainingTime);
			}
		}

		/* Now take full CPU for precision (only a few nanoseconds,
		   at maximum 1000 nanoseconds) */
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

		event();
	}

	free(finish);
	free(current);
	free(sleepTime);
	free(remainingTime);
}
#else
void FunctionConsumer::runNanoSleepTimer()
{
	while (m_running == true)
	{
		QThread::usleep(1000000 / KFrequency);
		event();
	}
}
#endif

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
	m_functionListMutex.lock();

	/* Issue a stop command to all running functions */
	QListIterator <Function*> it(m_functionList);
	while (it.hasNext() == true)
		it.next()->stop();

	m_functionListMutex.unlock();

	/* Wait until all functions have been stopped */
	while (runningFunctions() > 0)
		msleep(10);
}

void FunctionConsumer::stop()
{
	m_running = false;

	/* Wait for function threads to finish */
	while (runningFunctions() > 0)
		msleep(10);

	/* Wait for this thread to finish */
	wait();
}

void FunctionConsumer::run()
{
	m_running = true;

	switch (m_timerType)
	{
#ifdef X11
	case RTCTimer:
		runRTC();
		break;
#endif
	default:
	case NanoSleepTimer:
		runNanoSleepTimer();
		break;
	}
}

/**
 * Actual consumer function. Nothing^H^H^H^H^H As little as possible should
 * be allocated here to keep this as fast as possible.
 */
void FunctionConsumer::event()
{
	/* Lock before accessing the running functions list */
	m_functionListMutex.lock();

	QMutableListIterator <Function*> it(m_functionList);
	while (it.hasNext() == true)
	{
		m_function = it.next();

		if (m_function->eventBuffer()->get(m_event) == -1)
		{
			if (m_function->isRunning() == false)
			{
				/* Remove the current function */
				it.remove();

				/* Tell the function that it has been removed */
				m_function->finale();
			}
		}
		else
		{
			qDebug() << m_function->eventBuffer()->eventSize();

			for (m_channel = 0;
			     m_channel < m_function->eventBuffer()->eventSize();
			     m_channel++)
			{
				m_outputMap->setValue(
					t_channel(m_event[m_channel] >> 8),
					t_value(m_event[m_channel]));
			}
		}
	}

	/* No more loops, unlock and get out */
	m_functionListMutex.unlock();

	/* Dump the contents of each universe to their plugins */
	m_outputMap->dumpUniverses();
}
