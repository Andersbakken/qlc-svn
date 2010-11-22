/*
  Q Light Controller
  udmxdevice.h

  Copyright (c) Heikki Junnila
		Lutz Hillebrand

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

#ifndef UDMXDEVICE_H
#define UDMXDEVICE_H

#include <QObject>

#include "qlctypes.h"

struct usb_dev_handle;
struct usb_device;
class QString;

class UDMXDevice : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    UDMXDevice(QObject* parent, struct usb_device* device);
    virtual ~UDMXDevice();

    /** Find out, whether the given USB device is a uDMX device */
    static bool isUDMXDevice(const struct usb_device* device);

    /********************************************************************
     * Device information
     ********************************************************************/
public:
    QString name() const;
    QString infoText() const;

protected:
    void extractName();

protected:
    QString m_name;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    void open();
    void close();

    const struct usb_device* device() const;
    const usb_dev_handle* handle() const;

protected:
    struct usb_device* m_device;
    usb_dev_handle* m_handle;

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);
};

#endif
