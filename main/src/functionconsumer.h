/*
  Q Light Controller
  functionconsumer.h

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

#ifndef FUNCTIONCONSUMER_H
#define FUNCTIONCONSUMER_H

#include <qthread.h>
#include <qptrlist.h>

#include "common/types.h"
#include "function.h"

class DMXMap;

class FunctionConsumer : public QThread
{
public:
	/*********************************************************************
	 * Initialization
	 *********************************************************************/

	/** 
	 * Create a new FunctionConsumer instance. A FunctionConsumer takes
	 * care of running functions when the application is in operate mode.
	 *
	 * @param dmxMap A DMXMap instance used to write function values
	 */
	FunctionConsumer(DMXMap* dmxMap);

	/** Destroy a FunctionConsumer instance */
	virtual ~FunctionConsumer();
	
	/**
	 * Initialize the FunctionConsumer prior to starting it.
	 */
	void init();
	
	/*********************************************************************
	 * Timer type
	 *********************************************************************/
public:
	enum TimerType
	{
		RTCTimer,
		USleepTimer
	};

	/**
	 * Set the timer type for function consumer. Best results are achieved
	 * with RTCTimer, which taps directly into your PC's Real Time Clock
	 * chip. If such a chip is not available, try using USleepTimer.
	 *
	 * @param type The timer type to set. See enum @ref TimerType.
	 * @return true if successful, otherwise false
	 */
	bool setTimerType(TimerType type);

	/**
	 * Get the currently used timer type
	 *
	 * @return enum @ref TimerType.
	 */
	TimerType timerType() const { return m_timerType; }

protected:
	/** Currently used timer type */
	TimerType m_timerType;

	/*********************************************************************
	 * RTC timer
	 *********************************************************************/
protected:
	/**
	 * Open the RTC timer device and set its frequency
	 *
	 * @return true if successful, otherwise false
	 */
	bool openRTC();

	/**
	 * Close the RTC timer device
	 *
	 * @return true if successful, otherwise false
	 */
	bool closeRTC();

	/**
	 * Start receiving interrupts from the RTC timer
	 *
	 * @return true if successful, otherwise false
	 */
	bool startRTC();

	/**
	 * Stop RTC timer interrupts
	 *
	 * @return true if successful, otherwise false
	 */
	bool stopRTC();

	/**
	 * The main running thread function for RTC timer method.
	 */
	void runRTC();

protected:
	/** File descriptor for RTC timer device */
	int m_fdRTC;
	
	/*********************************************************************
	 * USleep timer
	 *********************************************************************/
protected:
	bool openUSleepTimer();
	bool closeUSleepTimer();
	bool startUSleepTimer();
	bool stopUSleepTimer();

	/**
	 * The main running thread function for USleep timer method.
	 */
	void runUSleepTimer();

	/*********************************************************************
	 * Functions
	 *********************************************************************/
public:
	/**
	 * Get the number of currently running functions
	 *
	 * @return Number of functions
	 */
	int runningFunctions();
	
	/**
	 * Append the given function to the FunctionConsumer's list of
	 * running functions.
	 *
	 * @param function The function to start running
	 */
	void cue(Function* function);
	
	/**
	 * Clear the list of running functions. This will stop all functions.
	 */
	void purge();
	
	/**
	 * Get the elapsed time since FunctionConsumer was started. This is
	 * used by, for example, chasers to calculate their hold time.
	 *
	 * @return The current timecode
	 */
	void timeCode(t_bus_value& timeCode);
	
	/** 
	 * Stop the FunctionConsumer alltogether. No functions will be run
	 * if FC is stopped.
	 */
	void stop();

	/*********************************************************************
	 * Stuff
	 *********************************************************************/
protected:
	/**
	 * Thhe main thread function that reads periodical interrupts
	 * from /dev/rtc.
	 */
	virtual void run();

	/**
	 * The main thread function calls this function to handle all running
	 * functions on each periodic pass.
	 */
	void event(time_t);
	
protected:
	/** A DMXMap instance that routes all values to correct plugins */
	DMXMap* m_dmxMap;

	/** Running status, telling, whether the FC has been started or not */
	bool m_running;

	/** List of currently running functions */
	QPtrList <Function> m_functionList;

	/** Mutex that guards access to m_functionList */
	QMutex m_functionListMutex;
	
	/** Elapsed time since FC start */
	t_bus_value m_timeCode;
	
	/** Buffer that holds the values of each function */
	t_buffer_data* m_event;

	/** 
	 * The current function, whose values are being handled. Normally,
	 * this would be defined in event() function, but since it is called
	 * 64 times per second, there's no point in allocating space for it
	 * every time from the stack. It is slightly more efficient to have
	 * it as a member variable.
	 */
	Function* m_function;

	/** 
	 * The currently evaluated channel. The same applies to this variable
	 * as to m_function.
	 */
	t_channel m_channel;
};

#endif
