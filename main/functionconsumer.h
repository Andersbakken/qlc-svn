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

#include <QThread>
#include <QMutex>
#include <QList>

#include "common/qlctypes.h"
#include "function.h"

class OutputMap;

class FunctionConsumer : public QThread
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new FunctionConsumer instance. FC takes care of running
	 * functions and driving internal DMX universe dumping to plugins.
	 *
	 * @param parent The parent that owns this instance
	 * @param outputMap A OutputMap instance used to write function values
	 */
	FunctionConsumer(QObject* parent, OutputMap* outputMap);

	/** Destroy a FunctionConsumer instance */
	virtual ~FunctionConsumer();

	/** Initialize the FunctionConsumer prior to starting it. */
	void init();

private:
	Q_DISABLE_COPY(FunctionConsumer)

	/*********************************************************************
	 * NanoSleep timer
	 *********************************************************************/
protected:
	void runNanoSleepTimer();

	/*********************************************************************
	 * Functions
	 *********************************************************************/
public:
	/** Get the number of currently running functions */
	int runningFunctions();

	/** Start running the given function */
	void startMe(Function* function);

	/** Stop running the given function */
	void stopMe(Function* function);

	/** Stop all functions */
	void stopAll();

protected:
	/** List of currently running functions */
	QList <Function*> m_functionList;

	/** Mutex that guards access to m_functionList */
	QMutex m_functionListMutex;

	/** An OutputMap instance that routes all values to correct plugins */
	OutputMap* m_outputMap;

	/*********************************************************************
	 * Main thread
	 *********************************************************************/
public:
	/** Stop this altogether. Functions cannot be run after this. */
	void stop();

protected:
	/** The main thread function */
	virtual void run();

	/**
	 * The main thread function calls this function to handle all running
	 * functions on each periodic pass.
	 */
	void event();

protected:
	/** Running status, telling, whether the FC has been started or not */
	bool m_running;

	/** Flag for stopping all functions */
	bool m_stopAll;
};

#endif
