/*
  Q Light Controller
  usbdmxdevice-win32.cpp

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

#include <QObject>
#include <QDebug>

#include "usbdmxdevice-win32.h"
#include "usbdmx-dynamic.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

USBDMXDevice::USBDMXDevice(QObject* parent, struct usbdmx_functions* usbdmx,
			   int output) : QObject(parent)
{
	Q_ASSERT(usbdmx != NULL);

	m_handle = NULL;
	m_output = output;
	m_usbdmx = usbdmx;

	memset(m_values, 0, 512 * sizeof(t_value));

	if (open() == false)
		m_name = QString("%1: Unable to open device").arg(m_output + 1);
	else
	{
		if (m_usbdmx->is_xswitch(m_handle))
			m_name = QString("%1: X-Switch").arg(m_output + 1);
		else if (m_usbdmx->is_rodin1(m_handle))
			m_name = QString("%1: Rodin 1").arg(m_output + 1);
		else if (m_usbdmx->is_rodin2(m_handle))
			m_name = QString("%1: Rodin 2").arg(m_output + 1);
		else if (m_usbdmx->is_rodint(m_handle))
			m_name = QString("%1: Rodin T").arg(m_output + 1);
		else if (m_usbdmx->is_usbdmx21(m_handle))
			m_name = QString("%1: USBDMX21").arg(m_output + 1);
		else
			m_name = QString("%1: Unknown").arg(m_output + 1);
	}

	close();
}

USBDMXDevice::~USBDMXDevice()
{
	close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString USBDMXDevice::name() const
{
	return m_name;
}

int USBDMXDevice::output() const
{
	return m_output;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool USBDMXDevice::open()
{
	if (m_handle != NULL)
		return true;

	/* Open the device */
	if (m_usbdmx->open(m_output, &m_handle) == TRUE)
	{
		USHORT version;

		/* Check the device version against driver version */
		m_usbdmx->device_version(m_handle, &version);
		if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
			return false;

		/* DMX512 specifies 0 as the official startcode */
		if (m_usbdmx->tx_startcode_set(m_handle, 0) == FALSE)
			return false;

		return true;
	}
	else
	{
		qWarning() << QString("Unable to open USBDMX %1")
					.arg(m_output + 1);
		return false;
	}
}

bool USBDMXDevice::close()
{
	if (m_handle == NULL)
		return true;

	m_usbdmx->close(m_handle);
	m_handle = NULL;

	return true;
}

/****************************************************************************
 * Read & write
 ****************************************************************************/

void USBDMXDevice::write(t_channel channel, t_value value)
{
	Q_ASSERT(channel < 512);
	m_values[channel] = value;
	if (m_handle != NULL)
		m_usbdmx->tx_set(m_handle, m_values, 512);
}

void USBDMXDevice::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);
	memcpy(m_values + address, values, num);
	if (m_handle != NULL)
		m_usbdmx->tx_set(m_handle, m_values, 512);
}

void USBDMXDevice::read(t_channel channel, t_value* value)
{
	Q_ASSERT(value != NULL);
	*value = m_values[channel];
}

void USBDMXDevice::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);
	memcpy(values, m_values + address, num);
}
