/*
  Q Light Controller
  mastertimer_test.cpp

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

#include <QtTest>
#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#endif

#include "mastertimer_test.h"
#include "outputmap_stub.h"
#include "function_stub.h"

#define protected public
#include "../mastertimer.h"
#undef protected

void MasterTimer_Test::initTestCase()
{
	m_oms = new OutputMapStub(this);
}

void MasterTimer_Test::initial()
{
	MasterTimer mt(this, m_oms);

	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_functionList.size() == 0);
	QVERIFY(mt.m_functionListMutex.tryLock() == true);
	mt.m_functionListMutex.unlock();
	QVERIFY(mt.outputMap() == m_oms);
	QVERIFY(mt.m_outputMap == m_oms);
	QVERIFY(mt.m_running == false);
	QVERIFY(mt.m_stopAllFunctions == false);
}

void MasterTimer_Test::startStop()
{
	MasterTimer mt(this, m_oms);

	mt.start();
#ifdef WIN32
	Sleep(200);
#else
	usleep(200);
#endif
	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_functionList.size() == 0);
	QVERIFY(mt.outputMap() == m_oms);
	QVERIFY(mt.m_outputMap == m_oms);
	QVERIFY(mt.m_running == true);
	QVERIFY(mt.m_stopAllFunctions == false);

	mt.stop();

	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_functionList.size() == 0);
	QVERIFY(mt.outputMap() == m_oms);
	QVERIFY(mt.m_outputMap == m_oms);
	QVERIFY(mt.m_running == false);
	QVERIFY(mt.m_stopAllFunctions == false);
}

void MasterTimer_Test::startStopFunction()
{
	MasterTimer mt(this, m_oms);
	Function_Stub fs(this);

	QVERIFY(mt.runningFunctions() == 0);

	mt.startFunction(&fs);
	QVERIFY(mt.runningFunctions() == 1);

	mt.startFunction(&fs);
	QVERIFY(mt.runningFunctions() == 1);

	mt.stopFunction(&fs);
	QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::interval()
{
	MasterTimer mt(this, m_oms);
	Function_Stub fs(this);

	mt.start();

	mt.startFunction(&fs);
	QVERIFY(mt.runningFunctions() == 1);

#ifdef WIN32
	Sleep(1000);
#else
	usleep(1000000);
#endif
	/* It's not guaranteed that context switch happens exactly after 50
	   cycles, so we just have to estimate here... */
	QVERIFY(fs.m_writeCalls >= 49 && fs.m_writeCalls <= 51);

	mt.stopFunction(&fs);
	QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::functionInitiatedStop()
{
	MasterTimer mt(this, m_oms);
	Function_Stub fs(this);

	mt.start();

	fs.start(&mt);
	QVERIFY(mt.runningFunctions() == 1);

	fs.setReturnState(false);
#ifdef WIN32
	Sleep(500);
#else
	usleep(500000);
#endif
	QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::runMultipleFunctions()
{
	MasterTimer mt(this, m_oms);
	mt.start();

	Function_Stub fs1(this);
	fs1.start(&mt);
	QVERIFY(mt.runningFunctions() == 1);

	Function_Stub fs2(this);
	fs2.start(&mt);
	QVERIFY(mt.runningFunctions() == 2);

	Function_Stub fs3(this);
	fs3.start(&mt);
	QVERIFY(mt.runningFunctions() == 3);

	fs1.setReturnState(false);
	fs2.setReturnState(false);
	fs3.setReturnState(false);
#ifdef WIN32
	Sleep(500);
#else
	usleep(500000);
#endif
	QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::stopAll()
{
	MasterTimer mt(this, m_oms);
	mt.start();

	Function_Stub fs1(this);
	fs1.start(&mt);

	Function_Stub fs2(this);
	fs2.start(&mt);

	Function_Stub fs3(this);
	fs3.start(&mt);
	QVERIFY(mt.runningFunctions() == 3);

	mt.stopAllFunctions();
	QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::stop()
{
	MasterTimer mt(this, m_oms);
	mt.start();

	Function_Stub fs1(this);
	fs1.start(&mt);

	Function_Stub fs2(this);
	fs2.start(&mt);

	Function_Stub fs3(this);
	fs3.start(&mt);
	QVERIFY(mt.runningFunctions() == 3);

	mt.stop();
	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_running == false);
}

void MasterTimer_Test::restart()
{
	MasterTimer mt(this, m_oms);
	mt.start();

	Function_Stub fs1(this);
	fs1.start(&mt);

	Function_Stub fs2(this);
	fs2.start(&mt);

	Function_Stub fs3(this);
	fs3.start(&mt);
	QVERIFY(mt.runningFunctions() == 3);

	mt.stop();
	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_functionList.size() == 0);
	QVERIFY(mt.m_functionListMutex.tryLock() == true);
	mt.m_functionListMutex.unlock();
	QVERIFY(mt.outputMap() == m_oms);
	QVERIFY(mt.m_outputMap == m_oms);
	QVERIFY(mt.m_running == false);
	QVERIFY(mt.m_stopAllFunctions == false);

	mt.start();
	QVERIFY(mt.runningFunctions() == 0);
	QVERIFY(mt.m_functionList.size() == 0);
	QVERIFY(mt.m_functionListMutex.tryLock() == true);
	mt.m_functionListMutex.unlock();
	QVERIFY(mt.outputMap() == m_oms);
	QVERIFY(mt.m_outputMap == m_oms);
	QVERIFY(mt.m_running == true);
	QVERIFY(mt.m_stopAllFunctions == false);

	fs1.start(&mt);
	fs2.start(&mt);
	fs3.start(&mt);
	QVERIFY(mt.runningFunctions() == 3);

	mt.stopAllFunctions();
}

void MasterTimer_Test::cleanupTestCase()
{
	delete m_oms;
	m_oms = NULL;
}

