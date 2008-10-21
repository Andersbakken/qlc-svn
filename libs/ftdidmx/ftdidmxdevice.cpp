/*
  Q Light Controller
  ftdidmxdevice.cpp
  
  Copyright (c) Christopher Staite
  
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

#include <sys/ioctl.h>
#include <QDebug>

#include "ftdidmxdevice.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

FTDIDMXDevice::FTDIDMXDevice(QObject* parent, char *description, int input_id,
			   t_output output) : QObject(parent)
{
	Q_ASSERT(path.isEmpty() == false);
	Q_ASSERT(output != KOutputInvalid);
	
	m_output = output;
	m_input_id = input_id;
	m_path.setNum(m_input_id);

	// Ensure we set everything to 0
	for (t_channel i = 0; i < sizeof(m_values); i++)
		m_values[i] = 0;
	m_dataChanged = true;

	m_name = QString("FTDI DMX Device: ") + QString(description);
}

FTDIDMXDevice::~FTDIDMXDevice()
{
	close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString FTDIDMXDevice::name() const
{
	return m_name;
}

QString FTDIDMXDevice::path() const
{
	return m_path;
}

t_output FTDIDMXDevice::output() const
{
	return m_output;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool FTDIDMXDevice::open()
{
	if (FT_Open(m_input_id, &m_handle) == FT_OK)
	{
		return true;
	}
	else
	{
		qWarning() << QString("Unable to open FTDIDMX %1: %2")
			.arg(m_output).arg("Because the world is stupid");
		return false;
	}
}

bool FTDIDMXDevice::close()
{
	if (FT_Close(m_handle) != FT_OK)
	{
		qWarning() << QString("Unable to close FTDIDMX %1: %2")
			.arg(m_output).arg("Because it was never open");
		return false;
	}
	else
	{
		return true;
	}
}

/****************************************************************************
 * Read & write
 ****************************************************************************/

void FTDIDMXDevice::write(t_channel channel, t_value value)
{
	m_mutex.lock();
	m_values[channel + 1] = value;
	m_mutex.unlock();
}

void FTDIDMXDevice::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(m_values + address + 1, values, num);
	m_mutex.unlock();
}

void FTDIDMXDevice::read(t_channel channel, t_value* value)
{
	m_mutex.lock();
	*value = m_values[channel + 1];
	m_mutex.unlock();
}

void FTDIDMXDevice::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(values, m_values + address + 1, num);
	m_mutex.unlock();
}
