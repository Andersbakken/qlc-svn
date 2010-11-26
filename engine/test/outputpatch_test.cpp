/*
  Q Light Controller - Unit test
  outputpatch_test.cpp

  Copyright (c) Heikki Junnila

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
#include <QtXml>

#include "outputpluginstub.h"
#include "outputpatch_test.h"
#include "qlcfile.h"

/* Expose protected members to unit test */
#define protected public
#include "outputpatch.h"
#include "outputmap.h"
#undef protected

#define TESTPLUGINDIR "../outputpluginstub"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void OutputPatch_Test::defaults()
{
    OutputPatch op(this);
    QVERIFY(op.m_plugin == NULL);
    QVERIFY(op.m_output == KOutputInvalid);
    QVERIFY(op.pluginName() == KOutputNone);
    QVERIFY(op.outputName() == KOutputNone);
    QVERIFY(op.isDMXZeroBased() == false);
}

void OutputPatch_Test::patch()
{
    OutputMap om(this);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    OutputPatch* op = new OutputPatch(this);
    op->set(stub, 0);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_output == 0);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[0]);
    QVERIFY(stub->m_openLines.size() == 1);
    QVERIFY(stub->m_openLines.at(0) == 0);

    op->set(stub, 3);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_output == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openLines.size() == 1);
    QVERIFY(stub->m_openLines.at(0) == 3);

    delete op;
    QVERIFY(stub->m_openLines.size() == 0);
}

void OutputPatch_Test::dmxZeroBased()
{
    OutputPatch op(this);
    QVERIFY(op.isDMXZeroBased() == false);

    op.setDMXZeroBased(true);
    QVERIFY(op.isDMXZeroBased() == true);
    QVERIFY(op.m_dmxZeroBased == true);
    op.setDMXZeroBased(true);
    QVERIFY(op.isDMXZeroBased() == true);
    QVERIFY(op.m_dmxZeroBased == true);
    op.setDMXZeroBased(false);
    QVERIFY(op.isDMXZeroBased() == false);
    QVERIFY(op.m_dmxZeroBased == false);
}

void OutputPatch_Test::dump()
{
    QByteArray uni(513, char(0));
    uni[0] = 100;
    uni[169] = 50;
    uni[511] = 25;

    OutputMap om(this);
    OutputPatch* op = new OutputPatch(this);

    om.loadPlugins(testPluginDir());
    QVERIFY(om.m_plugins.size() >= 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (om.m_plugins.at(0));
    QVERIFY(stub != NULL);

    op->set(stub, 0);
    QVERIFY(stub->m_array[0] == (char) 0);
    QVERIFY(stub->m_array[169] == (char) 0);
    QVERIFY(stub->m_array[511] == (char) 0);

    op->dump(uni);
    QVERIFY(stub->m_array[0] == (char) 100);
    QVERIFY(stub->m_array[169] == (char) 50);
    QVERIFY(stub->m_array[511] == (char) 25);

    delete op;
}
