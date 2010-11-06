/*
  Q Light Controller
  enttecdmxusbopen.cpp

  Copyright (C) Heikki Junnila
        		Christopher Staite

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#include <QDebug>
#include <math.h>
#include <QTime>

#include "enttecdmxusbopen.h"
#include "qlctypes.h"

#define DMX_CHANNELS 512
#define DMX_FREQ 44

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(QObject* parent,
                                   const FT_DEVICE_LIST_INFO_NODE& info,
                                   DWORD id)
    : QThread(parent)
    , m_handle(0)
    , m_id(id)
    , m_serial(QString(info.SerialNumber))
    , m_name(QString(info.Description))
    , m_running(false)
    , m_universe(NULL)
{
    m_universe = (char*) malloc(sizeof(char) * DMX_CHANNELS);
    for (int i = 0; i < DMX_CHANNELS; i++)
        m_universe[i] = 0;
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
    close();
    free(m_universe);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open()
{
    if (isOpen() == false)
    {
        /* Attempt to open the device */
        FT_STATUS status = FT_Open(m_id, &m_handle);
        if (status == FT_OK)
        {
            if (initializePort() == false)
            {
                qWarning() << "Unable to initialize" << name() << "- Closing.";
                close();
                return false;
            }

            if (isRunning() == false)
                start(QThread::TimeCriticalPriority);

            return true;
        }
        else
        {
            qWarning() << "Unable to open" << name() << "- Error:" << status;
            return false;
        }
    }
    else
    {
        /* Already open */
        return true;
    }
}

bool EnttecDMXUSBOpen::close()
{
    if (isOpen() == true)
    {
        /* Stop the writer thread */
        if (isRunning() == true)
            stop();

        FT_STATUS status = FT_Close(m_handle);
        if (status == FT_OK)
        {
            m_handle = 0;
            return true;
        }
        else
        {
            qWarning() << "Unable to close" << name() << ". Error:" << status;
            return false;
        }
    }
    else
    {
        return true;
    }
}

bool EnttecDMXUSBOpen::isOpen()
{
    if (m_handle != 0)
        return true;
    else
        return false;
}

bool EnttecDMXUSBOpen::initializePort()
{
    FT_STATUS status = FT_OK;

    /* Reset the widget */
    status = FT_ResetDevice(m_handle);
    if (status != FT_OK)
    {
        qWarning() << "FT_ResetDevice:" << status;
        return false;
    }

    /* Set the baud rate. 12 will give us 250Kbits */
    status = FT_SetDivisor(m_handle, 12);
    if (status != FT_OK)
    {
        qWarning() << "FT_SetDivisor:" << status;
        return false;
    }

    /* Set data characteristics */
    status = FT_SetDataCharacteristics(m_handle, FT_BITS_8,
                                       FT_STOP_BITS_2, FT_PARITY_NONE);
    if (status != FT_OK)
    {
        qWarning() << "FT_SetDataCharacteristics:" << status;
        return false;
    }

    /* Set flow control */
    status = FT_SetFlowControl(m_handle, FT_FLOW_NONE, 0, 0);
    if (status != FT_OK)
    {
        qWarning() << "FT_SetFlowControl:" << status;
        return false;
    }

    /* Set RS485 for sending */
    FT_ClrRts(m_handle);

    /* Clear TX RX buffers */
    FT_Purge(m_handle, FT_PURGE_TX);
    FT_Purge(m_handle, FT_PURGE_RX);

    return true;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBOpen::name() const
{
    return m_name;
}

QString EnttecDMXUSBOpen::serial() const
{
    return m_serial;
}

QString EnttecDMXUSBOpen::uniqueName() const
{
    return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBOpen::sendDMX(const QByteArray& universe)
{
    memcpy(m_universe, universe.constData(), DMX_CHANNELS);
    return true;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

void EnttecDMXUSBOpen::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void EnttecDMXUSBOpen::run()
{
    ULONG written = 0;
    FT_STATUS status = FT_OK;

    // Skip some sleep calls if timer is found to behave badly
    bool badTimer = false;

    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / (double)44) + (double)0.5);

    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QTime time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        badTimer = true;
    else
        badTimer = false;

    m_running = true;
    while (m_running == true)
    {
        time.restart();

        status = FT_SetBreakOn(m_handle);
        if (status != FT_OK)
        {
            qWarning() << "FT_SetBreakOn:" << status;
            goto framesleep;
        }

        // Don't sleep if timer granularity is too coarse
        if (badTimer == false)
            usleep(88);

        status = FT_SetBreakOff(m_handle);
        if (status != FT_OK)
        {
            qWarning() << "FT_SetBreakOff:" << status;
            goto framesleep;
        }

        // Don't sleep if timer granularity is too coarse
        if (badTimer == false)
            usleep(8);

        status = FT_Write(m_handle, m_universe, DMX_CHANNELS, &written);
        if (status != FT_OK)
        {
            qWarning() << "FT_Write universe:" << status;
            goto framesleep;
        }

framesleep:
        // Do a busy sleep if timer granularity is too coarse
        if (badTimer == true)
            while (time.elapsed() < frameTime) { /* NOP */ }
        else
            while (time.elapsed() < frameTime) { msleep(1); }
    }
}

