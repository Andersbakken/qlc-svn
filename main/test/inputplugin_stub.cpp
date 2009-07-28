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

#include "inputplugin_stub.h"
#include "../inputmap.h"

InputPluginStub::InputPluginStub()
{
}

InputPluginStub::~InputPluginStub()
{
}

void InputPluginStub::init()
{
}

void InputPluginStub::open(t_input input)
{
	if (m_openLines.contains(input) == false && input < KInputUniverseCount)
		m_openLines.append(input);
}

void InputPluginStub::close(t_input input)
{
	m_openLines.removeAll(input);
}

QStringList InputPluginStub::inputs()
{
	QStringList list;

	for (int i = 0; i < KInputUniverseCount; i++)
		list << QString("%1: Stub %1").arg(i + 1);

	return list;
}

void InputPluginStub::configure()
{
}

QString InputPluginStub::infoText(t_input input)
{
	Q_UNUSED(input);
	return QString("This is a plugin stub for testing.");
}

QString InputPluginStub::name()
{
	return QString("Input Plugin Stub");
}

void InputPluginStub::connectInputData(QObject* listener)
{
	Q_UNUSED(listener);
}

void InputPluginStub::feedBack(t_input input, t_input_channel channel,
			t_input_value value)
{
	Q_UNUSED(input);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}
