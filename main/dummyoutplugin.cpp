/*
  Q Light Controller
  dummyoutplugin.cpp

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

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include "dummyoutplugin.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DummyOutPlugin::DummyOutPlugin() : QLCOutPlugin()
{
}

DummyOutPlugin::~DummyOutPlugin()
{
}

void DummyOutPlugin::init()
{
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString DummyOutPlugin::name()
{
	return QString("Dummy Output");
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void DummyOutPlugin::open(t_output output)
{
	Q_UNUSED(output);
}

void DummyOutPlugin::close(t_output output)
{
	Q_UNUSED(output);
}

QStringList DummyOutPlugin::outputs()
{
	QStringList list;

	for (int i = 0; i < KUniverseCount; i++)
		list << QString("%1: Dummy Out %1").arg(i + 1);

	return list;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void DummyOutPlugin::configure()
{
	QMessageBox::information(NULL,
				 tr("Dummy output configuration"),
				 tr("This plugin has no configurable options"));
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DummyOutPlugin::infoText(t_output output)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (output == KOutputInvalid)
		str += QString("<H3>%1</H3>").arg(name());
	else
		str += QString("<H3>%1 %2</H3>").arg(name()).arg(output + 1);
	str += QString("<P>");
	str += QString("This plugin does absolutely nothing; ");
	str += QString("you can use it if you don't have ");
	str += QString("the necessary hardware for real control.");
	str += QString("</P>");

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

void DummyOutPlugin::writeChannel(t_output output, t_channel channel,
				  t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void DummyOutPlugin::writeRange(t_output output, t_channel address,
				t_value* values, t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}

void DummyOutPlugin::readChannel(t_output output, t_channel channel,
				 t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void DummyOutPlugin::readRange(t_output output, t_channel address,
			       t_value* values, t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}

Q_EXPORT_PLUGIN2(dummyout, DummyOutPlugin)
