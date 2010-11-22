/*
  Q Light Controller
  peperonidevice.cpp

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

#include <QDebug>
#include <usb.h>

#include "peperonidevice.h"

/** Lighting Solutions/Peperoni Light Vendor ID */
#define PEPERONI_VID            0x0CE1

/* Recognized Product IDs */
#define PEPERONI_PID_XSWITCH    0x0001
#define PEPERONI_PID_RODIN1     0x0002
#define PEPERONI_PID_RODIN2     0x0003
#define PEPERONI_PID_RODINT     0x0008
#define PEPERONI_PID_USBDMX21   0x0004

/** Common interface */
#define PEPERONI_IFACE_EP0      0x00

/** CONTROL MSG: Control the internal DMX buffer */
#define PEPERONI_TX_MEM_REQUEST  0x04
/** CONTROL MSG: Block until the DMX frame has been completely transmitted */
#define PEPERONI_TX_MEM_BLOCK    0x01
/** CONTROL MSG: Do not block during DMX frame send */
#define PEPERONI_TX_MEM_NONBLOCK 0x00

/** BULK WRITE: Bulk out endpoint */
#define PEPERONI_BULK_OUT_ENDPOINT 0x02
/** BULK WRITE: Oldest firmware version with bulk write support */
#define PEPERONI_FW_BULK_SUPPORT 0x400
/** BULK WRITE: Size of the "old" bulk header */
#define PEPERONI_OLD_BULK_HEADER_SIZE 4
/** BULK WRITE: "Old" bulk protocol ID */
#define PEPERONI_OLD_BULK_HEADER_ID 0x01
/** BULK WRITE: "Old" bulk transmit request */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX 0x00

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(QObject* parent, struct usb_device* device)
        : QObject(parent)
{
    Q_ASSERT(device != NULL);

    m_device = device;
    m_handle = NULL;
    m_firmwareVersion = 0;
    m_bulkBuffer = QByteArray(512 + PEPERONI_OLD_BULK_HEADER_SIZE, 0);

    extractName();
}

PeperoniDevice::~PeperoniDevice()
{
    close();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool PeperoniDevice::isPeperoniDevice(const struct usb_device* device)
{
    /* If there's nothing to inspect, it can't be what we're looking for */
    if (device == NULL)
        return false;

    /* If it's not manufactured by them, we're not interested in it */
    if (device->descriptor.idVendor != PEPERONI_VID)
        return false;

    if (device->descriptor.idProduct == PEPERONI_PID_RODIN1 ||
            device->descriptor.idProduct == PEPERONI_PID_RODIN2 ||
            device->descriptor.idProduct == PEPERONI_PID_RODINT ||
            device->descriptor.idProduct == PEPERONI_PID_XSWITCH ||
            device->descriptor.idProduct == PEPERONI_PID_USBDMX21)
    {
        /* We need one interface */
        if (device->config->bNumInterfaces < 1)
            return false;

        return true;
    }
    else
    {
        return false;
    }
}

void PeperoniDevice::extractName()
{
    bool needToClose = false;
    char name[256];
    int len;

    Q_ASSERT(m_device != NULL);

    if (m_handle == NULL)
    {
        needToClose = true;
        open();
    }

    /* Check, whether open() was successful */
    if (m_handle == NULL)
        return;

    /* Extract the name */
    len = usb_get_string_simple(m_handle, m_device->descriptor.iProduct,
                                name, sizeof(name));
    if (len > 0)
        m_name = QString(name);
    else
        m_name = tr("Unknown");

    /* Store fw version so we don't need to rely on libusb's volatile data */
    m_firmwareVersion = m_device->descriptor.bcdDevice;

    /* Close the device if it was opened for this function only. */
    if (needToClose == true)
        close();
}

QString PeperoniDevice::name() const
{
    return m_name;
}

QString PeperoniDevice::infoText() const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("<BR/>");
        info += tr("Firmware version: %1").arg(m_firmwareVersion, 4, 16, QChar('0'));
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>");
        info += tr("Unknown device");
        info += QString("</B>");
        info += QString("<P>");
        info += tr("Cannot connect to USB device.");
        info += QString("</P>");
    }

    return info;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void PeperoniDevice::open()
{
    if (m_device != NULL && m_handle == NULL)
    {
        m_handle = usb_open(m_device);
        if (m_handle == NULL)
        {
            qWarning() << "Unable to open Peperoni device!";
            return;
        }

        /* We must claim the interface before doing anything */
        int r = usb_claim_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
        {
            qWarning() << "PeperoniDevice:" << "Unable to claim interface EP0!";
        }

        r = usb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
        if (r < 0)
        {
            qWarning() << "PeperoniDevice:" << "Unable to reset endpoint";
        }
    }
}

void PeperoniDevice::close()
{
    if (m_device != NULL && m_handle != NULL)
    {
        /* Release the interface in case we claimed it */
        int r = usb_release_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
        {
            qWarning() << "PeperoniDevice:"
                       << "Unable to release interface EP0!";
        }

        usb_close(m_handle);
    }
    m_handle = NULL;
}

const struct usb_device* PeperoniDevice::device() const
{
    return m_device;
}

const usb_dev_handle* PeperoniDevice::handle() const
{
    return m_handle;
}

/****************************************************************************
 * Write
 ****************************************************************************/

void PeperoniDevice::outputDMX(const QByteArray& universe)
{
    int r = -1;

    if (m_handle == NULL)
        return;

    /* Choose write method based on firmware version. One has to unplug
       and then re-plug the dongle in apple for bulk write to work,
       so disable it for apple, since control msg should work for all. */
#ifndef __APPLE__
    if (m_firmwareVersion < PEPERONI_FW_BULK_SUPPORT)
    {
#endif
        r = usb_control_msg(m_handle, USB_TYPE_VENDOR |
                            USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
                            PEPERONI_TX_MEM_REQUEST, // We are WRITING data
                            PEPERONI_TX_MEM_BLOCK,   // Block during frame send?
                            0,                       // Start at DMX address 0
                            (char*) universe.data(), // The DMX universe data
                            universe.size(),         // Size of DMX universe
                            50);                     // Timeout (ms)
#ifndef __APPLE__
    }
    else
    {
        /* Construct a bulk header first */
        m_bulkBuffer[0] = char(PEPERONI_OLD_BULK_HEADER_ID);
        m_bulkBuffer[1] = char(PEPERONI_OLD_BULK_HEADER_REQUEST_TX);
        m_bulkBuffer[2] = char(universe.size() & 0xFF);
        m_bulkBuffer[3] = char((universe.size() >> 8) & 0xFF);

        /* Append universe data to the buffer */
        m_bulkBuffer.replace(PEPERONI_OLD_BULK_HEADER_SIZE,
                             universe.size(), universe);

        /* Perform a bulk write */
        r = usb_bulk_write(m_handle,
                           PEPERONI_BULK_OUT_ENDPOINT,
                           m_bulkBuffer.data(),
                           m_bulkBuffer.size(),
                           50);
    }
#endif

    if (r < 0)
    {
        qWarning() << name() << "is unable to write DMX universe:"
                   << usb_strerror();
    }
}
