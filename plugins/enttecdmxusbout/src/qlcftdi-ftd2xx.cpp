/*
  Q Light Controller
  qlcftdi-ftd2xx.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QDebug>

#include "enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "qlcftdi.h"

QLCFTDI::QLCFTDI(const QString& serial, const QString& name, quint32 id)
    : m_serial(serial)
    , m_name(name)
    , m_id(id)
    , m_handle(NULL)
{
}

QLCFTDI::~QLCFTDI()
{
    if (isOpen() == true)
        close();
}

QList <EnttecDMXUSBWidget*> QLCFTDI::widgets()
{
    QList <EnttecDMXUSBWidget*> list;

    /* Find out the number of FTDI devices present */
    DWORD num = 0;
    FT_STATUS status = FT_CreateDeviceInfoList(&num);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << "CreateDeviceInfoList:" << status;
        return list;
    }
    else if (num <= 0)
    {
        return list;
    }

    // Allocate storage for list based on numDevices
    FT_DEVICE_LIST_INFO_NODE* devInfo = new FT_DEVICE_LIST_INFO_NODE[num];

    // Get the device information list
    if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
    {
        for (DWORD i = 0; i < num; i++)
        {
            /* Get the device description field so that it can be
               used to determine the device type (Pro/Open) */
            QString name(devInfo[i].Description);
            QString serial(devInfo[i].SerialNumber);

            if (name.toLower().contains("pro") == true ||
                name.toLower().contains("dmxking") == true)
            {
                /* This is a DMX USB Pro widget */
                list << new EnttecDMXUSBPro(serial, name, i);
            }
            else
            {
                /* This is an Open DMX USB widget */
                list << new EnttecDMXUSBOpen(serial, name, i);
            }
        }
    }

    delete [] devInfo;
    return list;
}

bool QLCFTDI::open()
{
    FT_STATUS status = FT_Open(m_id, &m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::close()
{
    FT_STATUS status = FT_Close(m_handle);
    m_handle = NULL;
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::isOpen() const
{
    return (m_handle != NULL) ? true : false;
}

bool QLCFTDI::reset()
{
    FT_STATUS status = FT_ResetDevice(m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setLineProperties()
{
    FT_STATUS status = FT_SetDataCharacteristics(m_handle, 8, 2, 0);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBaudRate()
{
    FT_STATUS status = FT_SetBaudRate(m_handle, 250000);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setFlowControl()
{
    FT_STATUS status = FT_SetFlowControl(m_handle, 0, 0, 0);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::clearRts()
{
    FT_STATUS status = FT_ClrRts(m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::purgeBuffers()
{
    FT_STATUS status = FT_Purge(m_handle, FT_PURGE_RX | FT_PURGE_TX);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBreak(bool on)
{
    FT_STATUS status;
    if (on == true)
        status = FT_SetBreakOn(m_handle);
    else
        status = FT_SetBreakOff(m_handle);

    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::write(const QByteArray& data)
{
    DWORD written = 0;
    FT_STATUS status = FT_Write(m_handle, (char*) data.data(), data.size(), &written);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

QByteArray QLCFTDI::read(int size)
{
    char* buffer = (char*) malloc(sizeof(char) * size);
    QByteArray array;
    DWORD read = 0;

    FT_STATUS status = FT_Read(m_handle, buffer, size, &read);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
    }
    else
    {
        for (int i = 0; i < int(read); i++)
            array.append(buffer[i]);
    }

    free(buffer);
    return array;
}
