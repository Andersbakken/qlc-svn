/*
  Q Light Controller
  outputplugin_stub.cpp

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

#include <QString>
#include <QtTest>

#include "outputplugin_stub.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPluginStub::OutputPluginStub() : QLCOutPlugin()
{
    m_array = QByteArray(int(KUniverseCount * 512), char(0));
}

OutputPluginStub::~OutputPluginStub()
{
}

void OutputPluginStub::init()
{
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString OutputPluginStub::name()
{
    return QString("Output Plugin Stub");
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void OutputPluginStub::open(quint32 output)
{
    if (m_openLines.contains(output) == false && output < KUniverseCount)
        m_openLines.append(output);
}

void OutputPluginStub::close(quint32 output)
{
    m_openLines.removeAll(output);
}

QStringList OutputPluginStub::outputs()
{
    QStringList list;

    for (int i = 0; i < KUniverseCount; i++)
        list << QString("%1: Stub %1").arg(i + 1);

    return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void OutputPluginStub::configure()
{
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString OutputPluginStub::infoText(quint32 output)
{
    Q_UNUSED(output);
    return QString("This is a plugin stub for testing.");
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

void OutputPluginStub::outputDMX(quint32 output, const QByteArray& universe)
{
    m_array = m_array.replace(output * 512, universe.size(), universe);
}
