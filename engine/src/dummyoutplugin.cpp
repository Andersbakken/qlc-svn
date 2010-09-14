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

#include <QStringList>
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

void DummyOutPlugin::open(quint32 output)
{
	Q_UNUSED(output);
}

void DummyOutPlugin::close(quint32 output)
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
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DummyOutPlugin::infoText(quint32 output)
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
	str += QString("This plugin does absolutely nothing; you can use ");
	str += QString("it if you don't have DMX output plugins or ");
	str += QString("hardware installed. You can download output plugins from ");
	str += QString("<A HREF=\"http://www.sf.net/projects/qlc/files\">");
	str += QString("http://www.sf.net/projects/qlc/files</a>.");
	str += QString("</P>");
	str += QString("<P>");
	str += QString("This plugin has no configurable options.");
	str += QString("</P>");

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Write
 *****************************************************************************/

void DummyOutPlugin::outputDMX(quint32 output, const QByteArray& universe)
{
	Q_UNUSED(output);
	Q_UNUSED(universe);
}
