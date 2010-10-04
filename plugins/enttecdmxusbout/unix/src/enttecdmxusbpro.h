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
#include <QObject>

#include "enttecdmxusbwidget.h"
#include "ftdi.h"

class EnttecDMXUSBPro : public QObject, public EnttecDMXUSBWidget
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new DMXUSBPro object with the given parent and
     * FTDI device context. Neither can be NULL.
     *
     * @param parent The owner of this object
     * @param name The name of the device
     * @param serial The device's unique serial number
     */
    EnttecDMXUSBPro(QObject* parent, const QString& name,
                    const QString& serial);

    /**
     * Destructor
     */
    virtual ~EnttecDMXUSBPro();

protected:
    struct ftdi_context m_context;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /**
     * Open widget for further operations.
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

    /********************************************************************
     * Serial & name
     ********************************************************************/
public:
    /**
     * Get the device's friendly name, which is not unique, but only
     * tells the product name (e.g. "DMX USB PRO")
     *
     * @return widget's name
     */
    QString name() const;

    /**
     * Get the FTDI chip's serial number as a string. Can be used to
     * uniquely identify widgets.
     *
     * @return widget's serial number in string form
     */
    QString serial() const;

    /**
     * Get the widget's ENTTEC serial number as a string. The same serial
     * should be printed on the actual physical device.
     */
    QString enttecSerial() const;

    /**
     * Get the widget's unique name
     *
     * @return widget's unique name as: "<name> (S/N: <enttecserial>)"
     */
    QString uniqueName() const;

protected:
    /** Extract the widget's ENTTEC serial number */
    bool extractEnttecSerial();

protected:
    QString m_name;
    QString m_serial;
    QString m_enttecSerial;

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
