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
	
	if (open() == false)
		m_name = QString("%1: Nothing").arg(m_output);

	usbdmx->device_version(m_handle, &version);
	if (m_usbdmx->is_xswitch(m_handle))
		m_name = QString("%1: X-Switch V%2").arg(m_output).arg(version);
	else if (m_usbdmx->is_rodin1(m_handle))
		m_name = QString("%1: Rodin1 V%2").arg(m_output).arg(version);
	else if (m_usbdmx->is_rodin2(m_handle))
		m_name = QString("%1: Rodin2 V%2").arg(m_output).arg(version);
	else if (m_usbdmx->is_rodint(m_handle))
		m_name = QString("%1: RodinT V%2").arg(m_output).arg(version);
	else if (m_usbdmx->is_usbdmx21(m_handle))
		m_name = QString("%1: USBDMX21 V%2").arg(m_output).arg(version);
	else
		m_name = QString("%1: Unknown V%2").arg(m_output).arg(version);

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
	if (m_handle == NULL)
		return false;

	/* Open the device */
	if (m_usbdmx->open(m_output, &m_handle) == TRUE)
	{
		return true;
	}
	else
	{
		qWarning() << QString("Unable to open USBDMX %1").arg(m_output);
		return false;
	}
}

bool USBDMXDevice::close()
{
	if (m_handle != NULL)
		return false;

	m_usbdmx->close(m_handle);
	m_handle = NULL;

	return true;
}

HANDLE USBDMXDevice::handle() const
{
	return m_handle;
}
