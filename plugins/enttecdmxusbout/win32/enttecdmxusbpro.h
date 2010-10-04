/*
  Q Light Controller
  enttecdmxusbpro.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#include <QByteArray>
#include <windows.h>
#include <QObject>

#include "enttecdmxusbwidget.h"
#include "ftd2xx.h"

class EnttecDMXUSBPro : public QObject, public EnttecDMXUSBWidget
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new DMXUSBPro object with the given parent and
     * FTDI device information. Neither can be null.
     *
     * @param parent The owner of this object
     * @param info FTDI device information
     * @param id The ID of the device in FT2XX's internal structs
     */
    EnttecDMXUSBPro(QObject* parent, const FT_DEVICE_LIST_INFO_NODE& info,
                    DWORD id);

    /**
     * Destructor
     */
    virtual ~EnttecDMXUSBPro();

protected:
    DWORD m_id;
    FT_HANDLE m_handle;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /**
     * Open widget for further operations, such as serial() and sendDMX()
     *
     * @return true if widget was opened successfully (or was already open)
     */
    bool open();

    /**
     * Close widget, preventing any further operations
     *
     * @param true if widget was closed successfully (or was already closed)
     */
    bool close();

    /**
     * Check, whether widget has been opened
     *
     * @return true if widget is open, otherwise false
     */
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
    /**
     * Get the widget serial number as a string. The same serial should be
     * printed on the actual physical device. Can be used to uniquely
     * identify widgets.
     *
     * @return widget's serial number in string form
     */
    QString serial() const;

    /**
     * Get the device's friendly name, which is not unique, but only
     * tells the product name (e.g. "DMX USB PRO")
     *
     * @return widget's name
     */
    QString name() const;

    /**
     * Get the widget's unique name
     *
     * @return widget's unique name as: "<name> (S/N: <serial>)"
     */
    QString uniqueName() const;

protected:
    /** Extract the widget's unique serial number */
    bool extractSerial();

protected:
    QString m_serial;
    QString m_name;

    /********************************************************************
     * DMX operations
     ********************************************************************/
public:
    /**
     * Send the given universe-ful of DMX data to widget. The universe must
     * be at least 25 bytes but no more than 513 bytes long.
     *
     * @param universe The DMX universe to send
     * @return true if the values were sent successfully, otherwise false
     */
    bool sendDMX(const QByteArray& universe);
};

#endif
