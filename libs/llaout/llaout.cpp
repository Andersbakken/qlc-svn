/*
  Q Light Controller
  llaout.cpp
  
  Copyright (c) Simon Newton
                Heikki Junnila
  
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

#include <QApplication>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QFile>

#include <lla/LlaClient.h>

#include "common/qlcfile.h"
#include "configurellaout.h"
#include "llaout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void LLAOut::init()
{
	m_lla = new LlaClient();

	for (t_channel i = 0; i < 512; i++)
		m_values[i] = 0;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void LLAOut::open(t_output output)
{
	if (m_lla == NULL)
		return;

	if (output != 0)
		return;

	if (m_lla->start() < 0)
		qWarning() << "Unable to open LLA";
}

void LLAOut::close(t_output output)
{
	if (m_lla != NULL)
		m_lla->stop();
}

QStringList LLAOut::outputs()
{
	QStringList list;
	list << QString("LLA Out 1");
	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString LLAOut::name()
{
	return QString("LLA Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void LLAOut::configure()
{
	ConfigureLLAOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString LLAOut::infoText(t_output output)
{
	QString t;
	QString info;

	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * Outputs
	 *********************************************************************/

	/* Output */
	info += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Output");
	info += QString("</FONT>");
	info += QString("</TD>");

	/* Device name */
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Device name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	int i = 0;
	QStringListIterator it(outputs());
	while (it.hasNext() == true)
	{
		info += QString("<TR>");
		info += QString("<TD>%1</TD>").arg(i++);
		info += QString("<TD>%1</TD>").arg(it.next());
		info += QString("</TR>");
	}
	
	info += QString("</TABLE>");

	info += QString("</BODY>");
	info += QString("</HTML>");
	
	return info;
}

/*****************************************************************************
 * Value read/write
 *****************************************************************************/

void LLAOut::writeChannel(t_output output, t_channel channel, t_value value)
{
	if (output != 0)
		return;

	m_mutex.lock();
	m_values[channel] = value;
	if (m_lla != NULL)
	{
		m_lla->send_dmx(output + 1, (int*) m_values, 512);
		m_lla->fd_action(0);
	}
	m_mutex.unlock();
}

void LLAOut::writeRange(t_output output, t_channel address, t_value* values,
			t_channel num)
{
	if (output != 0)
		return;

	Q_ASSERT(values != NULL);

	m_mutex.lock();	
	memcpy(m_values + address, values, num * sizeof(t_value));
	if (m_lla != NULL)
	{
		m_lla->send_dmx(output + 1, (int*) m_values, 512);
		m_lla->fd_action(0);
	}
	m_mutex.unlock();
}

void LLAOut::readChannel(t_output output, t_channel channel, t_value* value)
{
	if (output != 0)
		return;

	m_mutex.lock();
	*value = m_values[channel];
	m_mutex.unlock();
}

void LLAOut::readRange(t_output output, t_channel address, t_value* values,
		       t_channel num)
{
	if (output != 0)
		return;

	Q_ASSERT(values != NULL);
	
	m_mutex.lock();
	memcpy(values, m_values + address, num);
	m_mutex.unlock();
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(llaout, LLAOut)
