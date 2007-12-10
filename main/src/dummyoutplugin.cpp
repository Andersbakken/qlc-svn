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
#include <qapplication.h>

#include "dummyoutplugin.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DummyOutPlugin::DummyOutPlugin() : OutputPlugin()
{
	m_version = 0x00010100;
	m_name = QString("Dummy Output");

	for (t_channel i = 0; i < KChannelMax; i++)
		m_values[i] = 0;
}

DummyOutPlugin::~DummyOutPlugin()
{
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

int DummyOutPlugin::open()
{
	return 0;
}

int DummyOutPlugin::close()
{
	return 0;
}

int DummyOutPlugin::outputs()
{
	return KUniverseCount;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int DummyOutPlugin::configure(QWidget* parentWidget)
{
	QMessageBox::information(parentWidget,
				 "Dummy Output Configuration",
				 "This plugin has no configurable options");
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DummyOutPlugin::infoText()
{
	QString str;
	QString t;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>Plugin Info</TITLE>");
	str += QString("</HEAD>");
	str += QString("<BODY>");

	/* Title */
	str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD BGCOLOR=\"");
	str += QApplication::palette().active().highlight().name();
	str += QString("\">");
	str += QString("<FONT COLOR=\"");
	str += QApplication::palette().active().highlightedText().name();
	str += QString("\" SIZE=\"5\">");
	str += name();
	str += QString("</FONT>");
	str += QString("</TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	/* Version */
	str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	str += QString("<TR>");
	str += QString("<TD><B>Version</B></TD>");
	t.sprintf("%d.%d.%d", (version() >> 16) & 0xff,
		  (version() >> 8) & 0xff, version() & 0xff);
	str += QString("<TD>" + t +"</TD>");
	str += QString("</TR>");

	str += QString("<TR>");
	str += QString("<TD><B>Outputs</B></TD>");
	t.sprintf("%d", outputs());
	str += QString("<TD><I>" + t + "</I></TD>");
	str += QString("</TR>");
	str += QString("</TABLE>");

	/* Note */
	str += QString("<H3>NOTE</H3>");
	str += QString("<P>This plugin does absolutely nothing; ");
	str += QString("you can use this if you don't have ");
	str += QString("the necessary hardware for real control.</P>");

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

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
