/*
  Q Light Controller
  udmxdevice.cpp

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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "libusb_dyn.h"
#else
#include <usb.h>
#endif

#include <QDebug>
#include "udmxdevice.h"

#define UDMX_SHARED_VENDOR     0x16C0 /* VOTI */
#define UDMX_SHARED_PRODUCT    0x05DC /* Obdev's free shared PID */
#define UDMX_SET_CHANNEL_RANGE 0x0002 /* Command to set n channel values */

/****************************************************************************
 * Initialization
 ****************************************************************************/

UDMXDevice::UDMXDevice(QObject* parent, struct usb_device* device)
        : QObject(parent)
{
    Q_ASSERT(device != NULL);

    m_device = device;
    m_handle = NULL;

    extractName();
}

UDMXDevice::~UDMXDevice()
{
    close();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool UDMXDevice::isUDMXDevice(const struct usb_device* device)
{
    if (device == NULL)
        return false;

    if ((device->descriptor.idVendor == UDMX_SHARED_VENDOR) &&
            (device->descriptor.idProduct == UDMX_SHARED_PRODUCT))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UDMXDevice::extractName()
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

    /* Close the device if it was opened for this function only. */
    if (needToClose == true)
        close();
}

QString UDMXDevice::name() const
{
    return m_name;
}

QString UDMXDevice::infoText() const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>%1</B>").arg(tr("Unknown device"));
        info += QString("<P>");
        info += tr("Cannot connect to USB device.");
        info += QString("</P>");
    }

    return info;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void UDMXDevice::open()
{
    if (m_device != NULL && m_handle == NULL)
        m_handle = usb_open(m_device);
}

void UDMXDevice::close()
{
    if (m_device != NULL && m_handle != NULL)
        usb_close(m_handle);
    m_handle = NULL;
}

const struct usb_device* UDMXDevice::device() const
{
    return m_device;
}

const usb_dev_handle* UDMXDevice::handle() const
{
    return m_handle;
}

/****************************************************************************
 * Write
 ****************************************************************************/

void UDMXDevice::outputDMX(const QByteArray& universe)
{
    if (m_handle == NULL)
        return;

    /* Write all 512 channels */
    int r = usb_control_msg(m_handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                            UDMX_SET_CHANNEL_RANGE, /* Command */
                            universe.size(),	/* Number of channels to set */
                            0,                      /* Starting index */
                            (char*)universe.data(), /* Values to set */
                            universe.size(),        /* Size of values */
                            500);                   /* Timeout 0.5s */
    if (r < 0)
        qWarning() << "uDMX: unable to write universe:" << usb_strerror();
}
