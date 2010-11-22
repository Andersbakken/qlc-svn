/*
  Q Light Controller
  vellemanout.cpp

  Copyright (c) Matthew Jaggard

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

#include <windows.h>
#include <stdint.h>

#include "k8062d_dll.h"
#include "vellemanout.h"


/*****************************************************************************
 * Initialization
 *****************************************************************************/

VellemanOut::~VellemanOut()
{
}

void VellemanOut::init()
{
    m_currentlyOpen = false;
}

QString VellemanOut::name()
{
    return QString("Velleman Output");
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void VellemanOut::open(quint32 output)
{
    if (output != 0)
        return;
}

void VellemanOut::close(quint32 output)
{
    if (output != 0)
        return;

    m_currentlyOpen = false;
    StopDevice();
}

QStringList VellemanOut::outputs()
{
    QStringList list;
    list << QString("1: Velleman Device");
    return list;
}

QString VellemanOut::infoText(quint32 output)
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
        str += tr("This plugin provides DMX output support for the Velleman "
                  "K8062D using the DLL supplied with the product.");
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

void VellemanOut::outputDMX(quint32 output, const QByteArray& universe)
{
    if (output != 0)
        return;

    if (m_currentlyOpen == false)
    {
        StartDevice();
        m_currentlyOpen = true;
    }

    int32_t channelCount = (int32_t) universe.size();
    SetChannelCount(channelCount);

    int32_t values [512]; //Set up array to pass to SetAllData.
    for (int32_t channelLoop = 0; channelLoop < channelCount; channelLoop++)
    {
        //Write the value to our temporary array.
        values[channelLoop] = (quint32) universe[channelLoop];
    }

    SetAllData(values);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void VellemanOut::configure()
{
}

bool VellemanOut::canConfigure()
{
    return false;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(VellemanOut, VellemanOut)
