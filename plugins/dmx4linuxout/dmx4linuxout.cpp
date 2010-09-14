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
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QFile>

#include "dmx4linuxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void DMX4LinuxOut::init()
{
	m_file.setFileName("/dev/dmx");
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void DMX4LinuxOut::open(quint32 output)
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

void DMX4LinuxOut::close(quint32 output)
{
	if (output != 0)
		return;

	m_file.close();
	m_file.unsetError();
}

QStringList DMX4LinuxOut::outputs()
{
	QStringList list;
	if (m_file.exists() == true)
		list << QString("1: DMX4Linux");
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
	int r = QMessageBox::question(NULL, name(),
				tr("Do you wish to re-scan your hardware?"),
				QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
		init();
}

/*****************************************************************************
 * Status
 *****************************************************************************/

QString DMX4LinuxOut::infoText(quint32 output)
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
		str += QString("devices supported by the DMX4Linux driver ");
		str += QString("suite. See <a href=\"http://llg.cubic.org\">");
		str += QString("http://llg.cubic.org</a> for more information");
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

void DMX4LinuxOut::outputDMX(quint32 output, const QByteArray& universe)
{
	if (output != 0 || m_file.isOpen() == false)
		return;

	m_file.seek(0);
	if (m_file.write(universe) == -1)
	{
		qWarning() << "DMX4Linux: Unable to write:"
			   << m_file.errorString();
	}
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(dmx4linuxout, DMX4LinuxOut)
