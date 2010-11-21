/*
  Q Light Controller
  enttecdmxusbopen.h

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

#ifndef ENTTECDMXUSBOPEN_H
#define ENTTECDMXUSBOPEN_H

#ifdef WIN32
#   include <windows.h>
#endif

#include <QByteArray>
#include <QThread>
#include <QMutex>

#include "enttecdmxusbwidget.h"
#include "ftd2xx.h"

class EnttecDMXUSBOpen : public QThread, public EnttecDMXUSBWidget
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new DMXUSBOpen object with the given parent and
     * FTDI device information. Neither can be null.
     *
     * @param parent The owner of this object
     * @param info FTDI device information
     * @param id The device's unique ID
     */
    EnttecDMXUSBOpen(QObject* parent, const FT_DEVICE_LIST_INFO_NODE& info,
                     DWORD id);

    /** Destructor */
    virtual ~EnttecDMXUSBOpen();

    /** @reimp */
    EnttecDMXUSBWidget::Type type() const;

protected:
    /** FTDI device information */
    FT_HANDLE m_handle;
    DWORD m_id;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    bool close();

    /** @reimp */
    bool isOpen();

    /**
     * Initialize the widget port for DMX output
     *
     * @return true if successful, otherwise false
     */
    bool initializePort();

    /********************************************************************
     * Serial & name
     ********************************************************************/
public:
    /** @reimp */
    QString serial() const;

    /** @reimp */
    QString name() const;

    /** @reimp */
    QString uniqueName() const;

    /** @reimp */
    QString additionalInfo() const;

protected:
    QString m_serial;
    QString m_name;

    /********************************************************************
     * DMX operations
     ********************************************************************/
public:
    /** @reimp */
    bool sendDMX(const QByteArray& universe);

    /********************************************************************
     * Thread
     ********************************************************************/
protected:
    enum TimerGranularity { Unknown, Good, Bad };

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

protected:
    bool m_running;
    char* m_universe;
    double m_frequency;
    TimerGranularity m_granularity;
};

#endif
