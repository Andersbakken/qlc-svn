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
	USHORT version;

	Q_ASSERT(usbdmx != NULL);
	
	m_handle = NULL;
	m_output = output;
	m_usbdmx = usbdmx;

	for (t_channel i = 0; i < 512; i++)
		m_values[i] = 0;
	
	if (open() == false)
		m_name = QString("%1: Nothing").arg(m_output + 1);

	usbdmx->device_version(m_handle, &version);
	if (m_usbdmx->is_xswitch(m_handle))
	{
		m_name = QString("%1: X-Switch Version %2")
				.arg(m_output + 1).arg(version);
	}
	else if (m_usbdmx->is_rodin1(m_handle))
	{
		m_name = QString("%1: Rodin1 Version %2")
				.arg(m_output + 1).arg(version);
	}
	else if (m_usbdmx->is_rodin2(m_handle))
	{
		m_name = QString("%1: Rodin2 Version %2")
				.arg(m_output + 1).arg(version);
	}
	else if (m_usbdmx->is_rodint(m_handle))
	{
		m_name = QString("%1: RodinT Version %2")
				.arg(m_output + 1).arg(version);
	}
	else if (m_usbdmx->is_usbdmx21(m_handle))
	{
		m_name = QString("%1: USBDMX21 Version %2")
				.arg(m_output + 1).arg(version);
	}
	else
	{
		m_name = QString("%1: Unknown Version %2")
				.arg(m_output + 1).arg(version);
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
	m_mutex.lock();

	m_values[channel] = value;
	if (m_handle != NULL)
		m_usbdmx->tx_set(m_handle, &value, 1);

	m_mutex.unlock();
}

t_value USBDMXDevice::read(t_channel channel)
{
	t_value value = 0;

	m_mutex.lock();
	value = m_values[channel];
	m_mutex.unlock();

	return value;
}
