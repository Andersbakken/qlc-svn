/*
  Q Light Controller - Unit test
  dummyoutplugin_test.cpp

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

#include "dummyoutplugin_test.h"
#define protected public
#include "dummyoutplugin.h"
#undef protected

void DummyOutPlugin_Test::dummyMethods()
{
    // Just a crash test for each useless function
    DummyOutPlugin* plugin = new DummyOutPlugin();
    plugin->open(12345);
    plugin->close(54321);
    QVERIFY(plugin->canConfigure() == false);
    plugin->configure();
    plugin->outputDMX(0, QByteArray(512, UCHAR_MAX));
    plugin->outputDMX(UINT_MAX, QByteArray(512, UCHAR_MAX));
    delete plugin;
}

void DummyOutPlugin_Test::outputs()
{
    DummyOutPlugin* plugin = new DummyOutPlugin();
    QStringList outputs(plugin->outputs());
    QCOMPARE(outputs.size(), int(KUniverseCount));
    for (int i = 0; i < outputs.size(); i++)
        QCOMPARE(outputs[i], QString("%1: Dummy Out %1").arg(i + 1));
    delete plugin;
}

void DummyOutPlugin_Test::infoText()
{
    DummyOutPlugin* plugin = new DummyOutPlugin();
    QString info(plugin->infoText(KInputInvalid));
    QVERIFY(info.contains(QString("<H3>%1</H3>").arg(plugin->name())));
    info = plugin->infoText(0);
    QVERIFY(info.contains(QString("<H3>%1 1</H3>").arg(plugin->name())));
    delete plugin;
}
