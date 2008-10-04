/*
  Q Light Controller
  dmx4linuxout.cpp
  
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

#include <QApplication>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QFile>

#include "common/qlcfile.h"

#include "configuredmx4linuxout.h"
#include "dmx4linuxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void DMX4LinuxOut::init()
{
	m_file.setFileName("/dev/dmx");

	for (t_channel i = 0; i < 512; i++)
		m_values[i] = 0;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void DMX4LinuxOut::open(t_output output)
{
	if (output != 0)
		return;

	m_file.unsetError();
	if (m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered) == false)
	{
		qWarning() << "DMX4Linux output is not available:"
			   << m_file.errorString();
	}
}

void DMX4LinuxOut::close(t_output output)
{
	if (output != 0)
		return;

	m_file.close();
	m_file.unsetError();
}

QStringList DMX4LinuxOut::outputs()
{
	QStringList list;
	list << QString("DMX4Linux 1");
	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString DMX4LinuxOut::name()
{
	return QString("DMX4Linux Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void DMX4LinuxOut::configure()
{
	ConfigureDMX4LinuxOut conf(NULL, this);
	conf.exec();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DMX4LinuxOut::infoText(t_output output)
{
	QString info = QString::null;
	QString t;

	/* HTML Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
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

void DMX4LinuxOut::writeChannel(t_output output, t_channel channel,
			        t_value value)
{
	if (output != 0)
		return;

	m_mutex.lock();
	m_values[channel] = value;
	m_file.seek(channel);
	if (m_file.write((const char*) &value, 1) == -1)
		qWarning() << "DMX4Linux: Unable to write:"
			   << m_file.errorString();
	m_mutex.unlock();
}

void DMX4LinuxOut::writeRange(t_output output, t_channel address,
			      t_value* values, t_channel num)
{
	if (output != 0)
		return;

	m_mutex.lock();
	memcpy(m_values + address, values, num);
	m_file.seek(address);
	if (m_file.write((const char*) values, num) == -1)
		qWarning() << "DMX4Linux: Unable to write:"
			   << m_file.errorString();
	m_mutex.unlock();
}

void DMX4LinuxOut::readChannel(t_output output, t_channel channel,
			       t_value* value)
{
	if (output != 0)
		return;

	m_mutex.lock();
	*value = m_values[channel];
	m_mutex.unlock();
}

void DMX4LinuxOut::readRange(t_output output, t_channel address,
			     t_value* values, t_channel num)
{
	if (output != 0)
		return;

	m_mutex.lock();
	memcpy(values, m_values + address, num);
	m_mutex.unlock();
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(dmx4linuxout, DMX4LinuxOut)
