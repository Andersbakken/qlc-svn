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

#include <qstring.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>

#include "dummyoutplugin.h"

const QString DummyOutPlugin::PluginName = QString("Dummy Output");

DummyOutPlugin::DummyOutPlugin() : OutputPlugin()
{
	m_open = false;
	m_version = 0x00010000;
	m_name = DummyOutPlugin::PluginName;

	for (t_channel i = 0; i < KChannelMax; i++)
	{
		m_values[i] = 0;
	}
}

DummyOutPlugin::~DummyOutPlugin()
{
}

int DummyOutPlugin::open()
{
	qDebug("DummyOut Plugin opened");

	m_open = true;
	return 0;
}

int DummyOutPlugin::close()
{
	qDebug("DummyOut Plugin closed");

	m_open = false;
	return 0;
}

int DummyOutPlugin::configure(QWidget* parentWidget)
{
	QMessageBox::information(parentWidget,
				 "Dummy Output Configuration",
				 "This plugin has no configurable options");
}

QString DummyOutPlugin::infoText()
{
	QString t;
	QString str = QString::null;
	str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	str += QString("<TR><TD BGCOLOR=\"black\">");
	str += QString("<FONT COLOR=\"white\" SIZE=\"5\">");
	str += name() + QString("</FONT></TD></TR></TABLE>");
	str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");

	str += QString("<TR><TD><B>Version</B></TD>");
	str += QString("<TD>");
	t.setNum((version() >> 16) & 0xff);
	str += t + QString(".");
	t.setNum((version() >> 8) & 0xff);
	str += t + QString(".");
	t.setNum(version() & 0xff);
	str += t + QString("</TD></TR>");

	str += QString("<TR>\n");
	str += QString("<TD><B>Status</B></TD>");
	str += QString("<TD>");
	str += QString("<I>Active</I></TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	str += QString("<H3>NOTE</H3>");
	str += QString("<P>This plugin does absolutely nothing; ");
	str += QString("you can use this if you don't have ");
	str += QString("the necessary hardware for real control.</P>");

	str += QString("</BODY></HTML>");

	return str;
}

int DummyOutPlugin::outputs()
{
	return KUniverseCount;
}

int DummyOutPlugin::writeChannel(t_channel channel, t_value value)
{
	m_mutex.lock();
	m_values[channel] = value;
	m_mutex.unlock();
	return 0;
}

int DummyOutPlugin::writeRange(t_channel address, t_value* values,
			       t_channel num)
{
	Q_ASSERT(values != NULL);

	m_mutex.lock();
	memcpy(m_values + address, values, num * sizeof(t_value));
	m_mutex.unlock();

	return 0;
}

int DummyOutPlugin::readChannel(t_channel channel, t_value &value)
{
	m_mutex.lock();
	value = m_values[channel];
	m_mutex.unlock();

	return 0;
}

int DummyOutPlugin::readRange(t_channel address, t_value* values,
			      t_channel num)
{
	Q_ASSERT(values != NULL);

	m_mutex.lock();
	memcpy(values, m_values + address, num * sizeof(t_value));
	m_mutex.unlock();

	return 0;
}
