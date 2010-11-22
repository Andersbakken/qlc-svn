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

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QFile>

#include "dmx4linuxout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DMX4LinuxOut::~DMX4LinuxOut()
{
    if (m_file.isOpen() == true)
        m_file.close();
}

void DMX4LinuxOut::init()
{
    m_file.setFileName("/dev/dmx");
}

QString DMX4LinuxOut::name()
{
    return QString("DMX4Linux Output");
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
        str += tr("This plugin provides DMX output for devices supported by "
                  "the DMX4Linux driver suite.");
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

void DMX4LinuxOut::outputDMX(quint32 output, const QByteArray& universe)
{
    if (output != 0 || m_file.isOpen() == false)
        return;

    m_file.seek(0);
    if (m_file.write(universe) == -1)
        qWarning() << "DMX4Linux: Unable to write:" << m_file.errorString();
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void DMX4LinuxOut::configure()
{
    /* NOP */
}

bool DMX4LinuxOut::canConfigure()
{
    return false;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(dmx4linuxout, DMX4LinuxOut)
