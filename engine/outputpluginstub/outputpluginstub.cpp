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

#include <QtPlugin>
#include "outputpluginstub.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPluginStub::~OutputPluginStub()
{
}

void OutputPluginStub::init()
{
    m_configureCalled = 0;
    m_canConfigure = false;
    m_array = QByteArray(int(KUniverseCount * 512), char(0));
}

QString OutputPluginStub::name()
{
    return QString("Output Plugin Stub");
}

/*****************************************************************************
 * Outputs
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

    for (quint32 i = 0; i < KUniverseCount; i++)
        list << QString("%1: Stub %1").arg(i + 1);

    return list;
}

QString OutputPluginStub::infoText(quint32 output)
{
    Q_UNUSED(output);
    return QString("This is a plugin stub for testing.");
}

void OutputPluginStub::outputDMX(quint32 output, const QByteArray& universe)
{
    m_array = m_array.replace(output * 512, universe.size(), universe);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void OutputPluginStub::configure()
{
    m_configureCalled++;
    emit configurationChanged();
}

bool OutputPluginStub::canConfigure()
{
    return m_canConfigure;
}

/*****************************************************************************
 * Plugin export
 *****************************************************************************/
Q_EXPORT_PLUGIN2(outputpluginstub, OutputPluginStub)
