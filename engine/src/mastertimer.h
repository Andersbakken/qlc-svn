/*
  Q Light Controller
  mastertimer.h

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

#ifndef MASTERTIMER_H
#define MASTERTIMER_H

#include <QThread>
#include <QMutex>
#include <QList>

class OutputMap;
class DMXSource;
class Function;

class MasterTimer : public QThread
{
	Q_OBJECT
	Q_DISABLE_COPY(MasterTimer)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new MasterTimer instance. FC takes care of running
	 * functions and driving internal DMX universe dumping to plugins.
	 *
	 * @param parent The parent that owns this instance
	 * @param outputMap A OutputMap instance used to write function values
	 */
	MasterTimer(QObject* parent, OutputMap* outputMap);

	/** Destroy a MasterTimer instance */
	virtual ~MasterTimer();

	/*********************************************************************
	 * Timer frequency
	 *********************************************************************/
public:
	/** Get the timer tick frequency in Hertz */
	static quint32 frequency() { return s_frequency; }

private:
	static const quint32 s_frequency;

	/*********************************************************************
	 * OutputMap
	 *********************************************************************/
public:
	/** Get the output map object that MasterTimer uses for DMX output */
	OutputMap* outputMap() const { return m_outputMap; }

protected:
	/** An OutputMap instance that routes all values to correct plugins. */
	OutputMap* m_outputMap;

	/*********************************************************************
	 * Functions
	 *********************************************************************/
public:
	/** Get the number of currently running functions */
	int runningFunctions();

	/** Start running the given function */
	virtual void startFunction(Function* function);

	/** Stop all functions. Doesn't affect registered DMX sources. */
	void stopAllFunctions();

protected:
	/** List of currently running functions */
	QList <Function*> m_functionList;

	/** Mutex that guards access to m_functionList */
	QMutex m_functionListMutex;

	/** Flag for stopping all functions */
	bool m_stopAllFunctions;

	/*********************************************************************
	 * DMX Sources
	 *********************************************************************/
public:
	/**
	 * Register a DMXSource for additional DMX data output (sliders and
	 * other directly user-controlled gadgets). Each DMXSource instance
	 * can be registered exactly once.
	 *
	 * @param source The DMXSource to register
	 */
	virtual void registerDMXSource(DMXSource* source);

	/**
	 * Unregister a previously registered DMXSource. This should be called
	 * in the DMXSource's destructor (at the latest).
	 *
	 * @param source The DMXSource to unregister
	 */
	virtual void unregisterDMXSource(DMXSource* source);

protected:
	/** List of currently running functions */
	QList <DMXSource*> m_dmxSourceList;

	/** Mutex that guards access to m_functionList */
	QMutex m_dmxSourceListMutex;

	/*********************************************************************
	 * Main thread
	 *********************************************************************/
public:
	/** Start the timer */
	void start(Priority priority = InheritPriority);

	/** Stop this altogether. Functions cannot be run after this. */
	void stop();

protected:
	/** The main thread function */
	virtual void run();

	/** Perform steps necessary for each timer tick */
	void timerTick();

	/** Execute one timer tick for each registered Function */
	void runFunctions();

	/** Execute one timer tick for each registered DMXSource */
	void runDMXSources();

protected:
	/** Running status, telling, whether the FC has been started or not */
	bool m_running;
};

#endif
