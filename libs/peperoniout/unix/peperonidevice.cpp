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

/** Control the internal DMX buffer */
#define PEPERONI_TX_MEM_REQUEST  0x04
/** Block until the DMX frame has been completely transmitted */
#define PEPERONI_TX_MEM_BLOCK    0x01
/** Do not block during DMX frame send */
#define PEPERONI_TX_MEM_NONBLOCK 0x00

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(QObject* parent, struct usb_device* device)
	: QObject(parent)
{
	Q_ASSERT(device != NULL);

	m_device = device;
	m_handle = NULL;

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
		info += QString("Device is working correctly.");
		info += QString("</P>");
	}
	else
	{
		info += QString("<B>Unknown device</B>");
		info += QString("<P>");
		info += QString("Cannot connect to USB device.");
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
			qWarning() << "PeperoniDevice:"
				   << "Unable to claim interface EP0!";
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
	if (m_handle == NULL)
		return;

	/* Write all 512 channels at once */
	int r = usb_control_msg(m_handle,
		USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_ENDPOINT_OUT,
		PEPERONI_TX_MEM_REQUEST, // We are writing DMX data
		PEPERONI_TX_MEM_NONBLOCK,// Don't block during frame send
		0,                       // Start at DMX address 0
		(char*) universe.data(),         // Our DMX universe
		universe.size(),         // Our DMX universe size
		500);                    // Timeout
	if (r < 0)
	{
		qWarning() << name() << "is unable to write DMX universe:"
			   << usb_strerror();
	}
}
