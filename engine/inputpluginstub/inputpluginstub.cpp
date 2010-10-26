/*
  Q Light Controller
  inputplugin_stub.cpp

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

#include "inputpluginstub.h"
#include "qlctypes.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputPluginStub::~InputPluginStub()
{
}

void InputPluginStub::init()
{
    m_configureCalled = 0;
    m_canConfigure = false;
}

QString InputPluginStub::name()
{
    return QString("Input Plugin Stub");
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

void InputPluginStub::open(quint32 input)
{
    if (m_openLines.contains(input) == false && input < KInputUniverseCount)
        m_openLines.append(input);
}

void InputPluginStub::close(quint32 input)
{
    m_openLines.removeAll(input);
}

QStringList InputPluginStub::inputs()
{
    QStringList list;

    for (quint32 i = 0; i < KInputUniverseCount; i++)
        list << QString("%1: Stub %1").arg(i + 1);

    return list;
}

QString InputPluginStub::infoText(quint32 input)
{
    return QString("%1: This is a plugin stub for testing.").arg(input);
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void InputPluginStub::configure()
{
    m_configureCalled++;
    emit configurationChanged();
}

bool InputPluginStub::canConfigure()
{
    return m_canConfigure;
}

/****************************************************************************
 * Feedback
 ****************************************************************************/

void InputPluginStub::feedBack(quint32 input, quint32 channel, uchar value)
{
    m_feedBackInput = input;
    m_feedBackChannel = channel;
    m_feedBackValue = value;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(inputpluginstub, InputPluginStub)
