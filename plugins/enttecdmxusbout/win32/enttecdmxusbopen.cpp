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

#include <QSettings>
#include <QDebug>
#include <math.h>
#include <QTime>

#include "enttecdmxusbopen.h"
#include "qlctypes.h"

#define DMX_CHANNELS 512
#define SETTINGS_FREQUENCY "enttecdmxusbopen/frequency"

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
    , m_frequency(30)
    , m_granularity(Unknown)
{
    m_universe = (char*) malloc(sizeof(char) * DMX_CHANNELS);
    for (int i = 0; i < DMX_CHANNELS; i++)
        m_universe[i] = 0;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_FREQUENCY);
    if (var.isValid() == true)
        m_frequency = var.toDouble();
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
    close();
    free(m_universe);
}

EnttecDMXUSBWidget::Type EnttecDMXUSBOpen::type() const
{
    return EnttecDMXUSBWidget::Open;
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

    /* Set the baud rate to 250Kbit/s */
    status = FT_SetBaudRate(m_handle, 250000);
    if (status != FT_OK)
    {
        qWarning() << "FT_SetBaudRate:" << status;
        return false;
    }

    /* Set data characteristics */
    status = FT_SetDataCharacteristics(m_handle, 8, 2, 0);
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

QString EnttecDMXUSBOpen::additionalInfo() const
{
    QString info;
    QString gran;

    info += QString("<P>");
    info += tr("<B>%1:</B> %2Hz").arg(tr("DMX Frame Frequency"))
								 .arg(m_frequency);
    info += QString("<BR>");
    if (m_granularity == Bad)
        gran = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bad"));
    else if (m_granularity == Good)
        gran = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Good"));
    else
        gran = tr("Patch this widget to a universe to find out.");
    info += tr("<B>System Timer Granularity:</B> %1").arg(gran);
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBOpen::sendDMX(const QByteArray& universe)
{
    memcpy(m_universe, universe.constData(), MIN(DMX_CHANNELS, universe.size()));
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
    char startCode = 0;

    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / m_frequency) + (double)0.5);

    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QTime time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        m_granularity = Bad;
    else
        m_granularity = Good;

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
        if (m_granularity == Good)
            usleep(100);

        status = FT_SetBreakOff(m_handle);
        if (status != FT_OK)
        {
            qWarning() << "FT_SetBreakOff:" << status;
            goto framesleep;
        }

        // Don't sleep if timer granularity is too coarse
        if (m_granularity == Good)
            usleep(8);

        status = FT_Write(m_handle, &startCode, 1, &written);
        if (status != FT_OK)
        {
            qWarning() << "FT_Write startcode:" << status;
            goto framesleep;
        }

        status = FT_Write(m_handle, m_universe, DMX_CHANNELS, &written);
        if (status != FT_OK)
        {
            qWarning() << "FT_Write universe:" << status;
            goto framesleep;
        }

framesleep:
        // Do a busy sleep if timer granularity is too coarse
        if (m_granularity == Good)
            while (time.elapsed() < frameTime) { usleep(1000); }
        else
            while (time.elapsed() < frameTime) { /* NOP */ }
    }
}

