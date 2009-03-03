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

#include <QThread>
#include <QDebug>

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "functionconsumer.h"
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
	m_running = false;
	m_stopAll = false;
}

FunctionConsumer::~FunctionConsumer()
{
	stop();
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

void FunctionConsumer::startMe(Function* function)
{
	Q_ASSERT(function != NULL);

	m_functionListMutex.lock();
	m_functionList.append(function);
	m_functionListMutex.unlock();
}

void FunctionConsumer::stopMe(Function* function)
{
	Q_ASSERT(function != NULL);

	m_functionListMutex.lock();
	m_functionList.removeAll(function);
	m_functionListMutex.unlock();
}

void FunctionConsumer::stopAll()
{
	m_stopAll = true;

	/* Wait until all functions have been stopped */
	while (runningFunctions() > 0)
		msleep(10);

	m_stopAll = false;
}

/****************************************************************************
 * Thread running / stopping
 ****************************************************************************/

void FunctionConsumer::start(Priority priority)
{
	m_running = true;
	QThread::start(priority);
}
 
#ifndef WIN32
void FunctionConsumer::run()
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
		qWarning() << "Unable to get the time accurately:"
			   << strerror(errno)
			   << "- Stopping FunctionConsumer";
		m_running = false;
	}
	else
	{
		m_running = true;
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
#else /* WIN32 */
void FunctionConsumer::run()
{
	while (m_running == true)
	{
		QThread::msleep(1000 / KFrequency);
		event();
	}
}
#endif

void FunctionConsumer::event()
{
	/* Lock before accessing the running functions list. */
	m_functionListMutex.lock();
	for (int i = 0; i < m_functionList.count(); i++)
	{
		QByteArray* universes;
		Function* function;

		function = m_functionList.at(i);
		Q_ASSERT(function != NULL);

		/* No need to access the list on this round anymore. */
		m_functionListMutex.unlock();
		universes = m_outputMap->claimUniverses();
		if (function->write(universes) == false || m_stopAll == true)
			function->stop();
		m_outputMap->releaseUniverses();

		/* Lock for the next round. */
		m_functionListMutex.lock();
	}

	/* No more functions. Get out and wait for next timer event. */
	m_functionListMutex.unlock();

	/* Write all universes' data to output plugins */
	m_outputMap->dumpUniverses();
}

void FunctionConsumer::stop()
{
	stopAll();

	m_running = false;
	wait();
}
