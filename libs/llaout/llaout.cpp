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

#ifndef WIN32
#include <lla/LlaClient.h>
#endif

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
	if (output != 0)
		return;

	if (m_lla != NULL)
		m_lla->stop();
}

QStringList LLAOut::outputs()
{
	QStringList list;
	list << QString("1: LLA Output 1");
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
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("the Linux Lighting Architecture (LLA). ");
		str += QString("See <a href=\"http://www.nomis52.net\">");
		str += QString("http://www.nomis52.net</a> for more ");
		str += QString("information.");
		str += QString("</P>");
	}
	else if (output == 0)
	{
		str += QString("<H3>%1</H3>").arg(outputs()[output]);
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
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
		m_lla->send_dmx(output + 1, m_values, 512);
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
		m_lla->send_dmx(output + 1, m_values, 512);
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
