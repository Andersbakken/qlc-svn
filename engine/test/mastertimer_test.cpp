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
#   include <windows.h>
#   include <winbase.h>
#endif

#include "mastertimer_test.h"
#include "outputmap_stub.h"
#include "dmxsource_stub.h"
#include "function_stub.h"

#include "universearray.h"
#include "doc.h"

#define protected public
#include "mastertimer.h"
#undef protected

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void MasterTimer_Test::initTestCase()
{
    m_oms = new OutputMapStub(this);
    m_ua = new UniverseArray(4 * 512);
    m_oms->setUniverses(m_ua);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_cache.load(dir) == true);
}

void MasterTimer_Test::init()
{
    m_doc = new Doc(this, m_cache);
}

void MasterTimer_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void MasterTimer_Test::initial()
{
    MasterTimer mt(this, m_oms);

    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(mt.m_functionList.size() == 0);
    QVERIFY(mt.m_functionListMutex.tryLock() == true);
    mt.m_functionListMutex.unlock();

    QVERIFY(mt.m_dmxSourceList.size() == 0);
    QVERIFY(mt.m_dmxSourceListMutex.tryLock() == true);
    mt.m_dmxSourceListMutex.unlock();

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
    Sleep(100);
#else
    usleep(100000);
#endif
    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(mt.m_functionList.size() == 0);
    QVERIFY(mt.m_dmxSourceList.size() == 0);
    QVERIFY(mt.outputMap() == m_oms);
    QVERIFY(mt.m_outputMap == m_oms);
    QVERIFY(mt.m_running == true);
    QVERIFY(mt.m_stopAllFunctions == false);

    mt.stop();
#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(mt.m_functionList.size() == 0);
    QVERIFY(mt.m_dmxSourceList.size() == 0);
    QVERIFY(mt.outputMap() == m_oms);
    QVERIFY(mt.m_outputMap == m_oms);
    QVERIFY(mt.m_running == false);
    QVERIFY(mt.m_stopAllFunctions == false);
}

void MasterTimer_Test::startStopFunction()
{
    MasterTimer mt(this, m_oms);
    mt.start();

    Function_Stub fs(m_doc);

    QVERIFY(mt.runningFunctions() == 0);

    mt.startFunction(NULL, true);
    QVERIFY(mt.runningFunctions() == 0);

    mt.startFunction(&fs, true);
    QVERIFY(mt.runningFunctions() == 1);
    QVERIFY(fs.initiatedByOtherFunction() == true);

    mt.startFunction(&fs, false);
    QVERIFY(mt.runningFunctions() == 1);
    QVERIFY(fs.initiatedByOtherFunction() == true);

#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    fs.stop();
#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::registerUnregisterDMXSource()
{
    MasterTimer mt(this, m_oms);
    QVERIFY(mt.m_dmxSourceList.size() == 0);

    DMXSource_Stub s1;
    /* Normal registration */
    mt.registerDMXSource(&s1);
    QVERIFY(mt.m_dmxSourceList.size() == 1);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s1);

    /* No double additions */
    mt.registerDMXSource(&s1);
    QVERIFY(mt.m_dmxSourceList.size() == 1);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s1);

    DMXSource_Stub s2;
    /* Normal registration of another source */
    mt.registerDMXSource(&s2);
    QVERIFY(mt.m_dmxSourceList.size() == 2);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt.m_dmxSourceList.at(1) == &s2);

    /* No double additions */
    mt.registerDMXSource(&s2);
    QVERIFY(mt.m_dmxSourceList.size() == 2);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt.m_dmxSourceList.at(1) == &s2);

    /* No double additions */
    mt.registerDMXSource(&s1);
    QVERIFY(mt.m_dmxSourceList.size() == 2);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s1);
    QVERIFY(mt.m_dmxSourceList.at(1) == &s2);

    /* Removal of a source */
    mt.unregisterDMXSource(&s1);
    QVERIFY(mt.m_dmxSourceList.size() == 1);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s2);

    /* No double removals */
    mt.unregisterDMXSource(&s1);
    QVERIFY(mt.m_dmxSourceList.size() == 1);
    QVERIFY(mt.m_dmxSourceList.at(0) == &s2);

    /* Removal of the last source */
    mt.unregisterDMXSource(&s2);
    QVERIFY(mt.m_dmxSourceList.size() == 0);
}

void MasterTimer_Test::interval()
{
    MasterTimer mt(this, m_oms);
    Function_Stub fs(m_doc);
    DMXSource_Stub dss;

    mt.start();
#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    mt.startFunction(&fs, false);
    QVERIFY(mt.runningFunctions() == 1);

    mt.registerDMXSource(&dss);
    QVERIFY(mt.m_dmxSourceList.size() == 1);

    /* Wait for one second */
#ifdef WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif
    /* It's not guaranteed that context switch happens exactly after 50
       cycles, so we just have to estimate here... */
    QVERIFY(fs.m_writeCalls >= 49 && fs.m_writeCalls <= 51);
    QVERIFY(dss.m_writeCalls >= 49 && dss.m_writeCalls <= 51);

    fs.stop();
#ifdef WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif
    QVERIFY(mt.runningFunctions() == 0);

    mt.unregisterDMXSource(&dss);
    QVERIFY(mt.m_dmxSourceList.size() == 0);
}

void MasterTimer_Test::functionInitiatedStop()
{
    MasterTimer mt(this, m_oms);
    Function_Stub fs(m_doc);

    mt.start();

    mt.startFunction(&fs, false);
    QVERIFY(mt.runningFunctions() == 1);

    /* Wait a while so that the function starts running */
#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    /* Stop the function after it has been running for a while */
    fs.stop();

#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    /* Verify that the function is really stopped and the correct
       pre&post handlers have been called. */
    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(fs.m_preRunCalls == 1);
    QVERIFY(fs.m_writeCalls > 0);
    QVERIFY(fs.m_postRunCalls == 1);
}

void MasterTimer_Test::runMultipleFunctions()
{
    MasterTimer mt(this, m_oms);
    mt.start();

    Function_Stub fs1(m_doc);
    mt.startFunction(&fs1, false);
    QVERIFY(mt.runningFunctions() == 1);

    Function_Stub fs2(m_doc);
    mt.startFunction(&fs2, false);
    QVERIFY(mt.runningFunctions() == 2);

    Function_Stub fs3(m_doc);
    mt.startFunction(&fs3, false);
    QVERIFY(mt.runningFunctions() == 3);

    /* Wait a while so that the functions start running */
#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif

    /* Stop the functions after they have been running for a while */
    fs1.stop();
    fs2.stop();
    fs3.stop();

#ifdef WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    QVERIFY(mt.runningFunctions() == 0);
}

void MasterTimer_Test::stopAllFunctions()
{
    MasterTimer mt(this, m_oms);
    mt.start();

    Function_Stub fs1(m_doc);
    mt.startFunction(&fs1, false);

    DMXSource_Stub s1;
    mt.registerDMXSource(&s1);

    Function_Stub fs2(m_doc);
    mt.startFunction(&fs2, false);

    DMXSource_Stub s2;
    mt.registerDMXSource(&s2);

    Function_Stub fs3(m_doc);
    mt.startFunction(&fs3, false);

    QVERIFY(mt.runningFunctions() == 3);
    QVERIFY(mt.m_dmxSourceList.size() == 2);

    mt.stopAllFunctions();
    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(mt.m_dmxSourceList.size() == 2); // Shouldn't stop

    mt.unregisterDMXSource(&s1);
    mt.unregisterDMXSource(&s2);
}

void MasterTimer_Test::stop()
{
    MasterTimer mt(this, m_oms);
    mt.start();

    Function_Stub fs1(m_doc);
    mt.startFunction(&fs1, false);

    Function_Stub fs2(m_doc);
    mt.startFunction(&fs2, false);

    Function_Stub fs3(m_doc);
    mt.startFunction(&fs3, false);
    QVERIFY(mt.runningFunctions() == 3);

    mt.stop();
    QVERIFY(mt.runningFunctions() == 0);
    QVERIFY(mt.m_running == false);
}

void MasterTimer_Test::restart()
{
    MasterTimer mt(this, m_oms);
    mt.start();

    Function_Stub fs1(m_doc);
    mt.startFunction(&fs1, false);

    Function_Stub fs2(m_doc);
    mt.startFunction(&fs2, false);

    Function_Stub fs3(m_doc);
    mt.startFunction(&fs3, false);
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

    mt.startFunction(&fs1, false);
    mt.startFunction(&fs2, false);
    mt.startFunction(&fs3, false);
    QVERIFY(mt.runningFunctions() == 3);

    mt.stopAllFunctions();
}

void MasterTimer_Test::cleanupTestCase()
{
    m_oms->setUniverses(NULL);
    delete m_oms;
    m_oms = NULL;

    delete m_ua;
    m_ua = NULL;
}
