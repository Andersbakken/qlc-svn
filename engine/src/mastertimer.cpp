/*
  Q Light Controller
  mastertimer.cpp

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
#include <QTime>

#ifndef WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#else
#include <windows.h>
#endif

#include "universearray.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "dmxsource.h"
#include "function.h"

/** The timer tick frequency in Hertz */
const quint32 MasterTimer::s_frequency = 50;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

MasterTimer::MasterTimer(QObject* parent, OutputMap* outputMap)
        : QThread(parent),
        m_outputMap(outputMap),
        m_stopAllFunctions(false),
        m_running(false)
{
}

MasterTimer::~MasterTimer()
{
    stop();
}

quint32 MasterTimer::frequency()
{
    return s_frequency;
}

OutputMap* MasterTimer::outputMap() const
{
    return m_outputMap;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

int MasterTimer::runningFunctions()
{
    m_functionListMutex.lock();
    int n = m_functionList.size();
    m_functionListMutex.unlock();
    return n;
}

void MasterTimer::startFunction(Function* function, bool initiatedByOtherFunction)
{
    if (function == NULL)
        return;

    m_functionListMutex.lock();
    if (m_functionList.contains(function) == false)
    {
        m_functionList.append(function);
        function->setInitiatedByOtherFunction(initiatedByOtherFunction);
    }
    m_functionListMutex.unlock();

    emit functionListChanged();
}

void MasterTimer::stopAllFunctions()
{
    m_stopAllFunctions = true;

    /* Wait until all functions have been stopped */
    while (runningFunctions() > 0)
        msleep(10);

    m_stopAllFunctions = false;
}

/****************************************************************************
 * DMX Sources
 ****************************************************************************/

void MasterTimer::registerDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    m_dmxSourceListMutex.lock();
    if (m_dmxSourceList.contains(source) == false)
        m_dmxSourceList.append(source);
    m_dmxSourceListMutex.unlock();
}

void MasterTimer::unregisterDMXSource(DMXSource* source)
{
    Q_ASSERT(source != NULL);

    m_dmxSourceListMutex.lock();
    m_dmxSourceList.removeAll(source);
    m_dmxSourceListMutex.unlock();
}

/****************************************************************************
 * Thread running / stopping
 ****************************************************************************/

void MasterTimer::start(Priority priority)
{
    /* Start with a clean slate */
    m_functionList.clear();
    m_dmxSourceList.clear();

    m_running = true;
    QThread::start(priority);
}

#ifndef WIN32
void MasterTimer::run()
{
    /* How long to wait each loop */
    int tickTime = 1000000 / frequency();

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
        qWarning() << Q_FUNC_INFO << "Unable to get the time accurately:"
                   << strerror(errno) << "- Stopping MasterTimer";
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
            qWarning() << Q_FUNC_INFO << "Unable to get the current time:"
                       << strerror(errno);
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
                qWarning() << Q_FUNC_INFO << "Unable to get the current time:"
                           << strerror(errno);
                m_running = false;
                break;
            }
        }

        /* Execute the next timer event */
        timerTick();
    }

    free(finish);
    free(current);
    free(sleepTime);
    free(remainingTime);
}
#else /* WIN32 */
void MasterTimer::run()
{
    /* This timer implementation requires 64bit support from compiler.
       (Not 64bit processor architecture, though.) */
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
    LARGE_INTEGER lap;
    LONGLONG target;

    /* Calculate the target time that should be waited before each event */
    QueryPerformanceFrequency(&freq);
    target = freq.QuadPart / frequency();

    while (m_running == true)
    {
        /* Reset the timer and make the first check */
        QueryPerformanceCounter(&start);
        QueryPerformanceCounter(&lap);

        /* Loop here until $target ticks have passed */
        while ((lap.QuadPart - start.QuadPart) < target)
        {
            /* Relinquish this thread's time slot, but don't sleep
               because that would skew the timer at least 30ms. */
            Sleep(0);

            /* Check how many ticks have passed */
            QueryPerformanceCounter(&lap);
        }

        /* Execute the next timer event */
        timerTick();
    }
}
#endif

void MasterTimer::timerTick()
{
    UniverseArray* universes = m_outputMap->claimUniverses();
    universes->zeroIntensityChannels();

    runFunctions(universes);
    runDMXSources(universes);

    m_outputMap->releaseUniverses();
    m_outputMap->dumpUniverses();
}

void MasterTimer::runFunctions(UniverseArray* universes)
{
    /* Lock before accessing the running functions list. */
    m_functionListMutex.lock();
    for (int i = 0; i < m_functionList.size(); i++)
    {
        Function* function = m_functionList.at(i);

        /* No need to access function list on this round anymore */
        m_functionListMutex.unlock();

        if (function != NULL)
        {
            if (function->elapsed() == 0)
                function->preRun(this);

            /* Check for pre-conditions before getting data */
            if (function->stopped() == true ||
                    m_stopAllFunctions == true)
            {
                /* Function should be stopped instead */
                m_functionListMutex.lock();
                m_functionList.removeAt(i);
                function->postRun(this, universes);
                m_functionListMutex.unlock();
                emit functionListChanged();
            }
            else
            {
                /* Run normally: get function data */
                function->write(this, universes);
            }
        }

        /* Lock function list for the next round. */
        m_functionListMutex.lock();
    }

    /* No more functions. Get out and wait for next timer event. */
    m_functionListMutex.unlock();
}

void MasterTimer::runDMXSources(UniverseArray* universes)
{
    /* Lock before accessing the running functions list. */
    m_dmxSourceListMutex.lock();
    for (int i = 0; i < m_dmxSourceList.size(); i++)
    {
        DMXSource* source = m_dmxSourceList.at(i);
        Q_ASSERT(source != NULL);

        /* No need to access the list on this round anymore. */
        m_dmxSourceListMutex.unlock();

        /* Get DMX data from the source */
        source->writeDMX(this, universes);

        /* Lock for the next round. */
        m_dmxSourceListMutex.lock();
    }

    /* No more sources. Get out and wait for next timer event. */
    m_dmxSourceListMutex.unlock();
}

void MasterTimer::stop()
{
    stopAllFunctions();

    m_running = false;
    wait();
}

