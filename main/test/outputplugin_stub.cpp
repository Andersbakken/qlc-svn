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

OutputPluginStub::OutputPluginStub() : QLCOutPlugin(),
					m_array(KUniverseCount * 512, 0)
{
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

void OutputPluginStub::open(t_output output)
{
	if (m_openLines.contains(output) == false && output < KUniverseCount)
		m_openLines.append(output);
}

void OutputPluginStub::close(t_output output)
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

QString OutputPluginStub::infoText(t_output output)
{
	Q_UNUSED(output);
	return QString("This is a plugin stub for testing.");
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

void OutputPluginStub::writeChannel(t_output output, t_channel channel,
				    t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void OutputPluginStub::writeRange(t_output output, t_channel address,
				  t_value* values, t_channel num)
{
	QVERIFY(output <= 3);
	QVERIFY(address == 0);
	QVERIFY(num == 512);

	memcpy(m_array.data(), values, num);
}

void OutputPluginStub::readChannel(t_output output, t_channel channel,
				   t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void OutputPluginStub::readRange(t_output output, t_channel address,
				 t_value* values, t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}
