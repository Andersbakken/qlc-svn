/*
  Q Light Controller
  serialdmxdevice.cpp
  
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

#include "serialdmxdevice.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

SerialDMXDevice::SerialDMXDevice(QObject* parent, const QString &path,
			   t_output output) : QObject(parent)
{
	Q_ASSERT(path.isEmpty() == false);
	Q_ASSERT(output != KOutputInvalid);
	
	m_output = output;
	m_forceTimeoutWait = false;
	m_updateTimeout = -1;
	m_updateFunction = 0;
	m_updateTimer = 0;
	
	if (detectDeviceType(path)) {
		// Only set this if valid device
		m_path = path;
	} else {
		m_path = "";
	}

	for (t_channel i = 0; i < sizeof(m_values); i++)
		m_values[i] = 0;

	m_name = QString("USB Serial Device");
}

SerialDMXDevice::~SerialDMXDevice()
{
	close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString SerialDMXDevice::name() const
{
	return m_name;
}

QString SerialDMXDevice::path() const
{
	return m_path;
}

t_output SerialDMXDevice::output() const
{
	return m_output;
}


/****************************************************************************
 * Device code
 ****************************************************************************/

bool SerialDMXDevice::detectDeviceType(const QString &path) {
	// Return false because we do not know what the device is
	return false;
}

void SerialDMXDevice::applyDeviceOptions() {
	if (m_updateTimeout >= 0) {
		if (m_updateTimer != 0) {
			m_updateTimer = new QTimer(this);
			connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(performDeviceWrite()));
		}
		m_updateTimer->start(m_updateTimeout);
	} else if (m_updateTimer != 0) {
		m_updateTimer->stop();
		delete m_updateTimer;
		m_updateTimer = 0;
	}
}

void SerialDMXDevice::dataChanged() {
	if (m_forceTimeoutWait)
		return;
	if (m_updateTimer != 0 && m_updateTimer->isActive()) {
		// Restart the timer
		m_updateTimer->stop();
		m_updateTimer->start(m_updateTimeout);
	}
	// We use the QTimer to run this in another thread
	m_updateTimer->singleShot(0, this, SLOT(performDeviceWrite()));
}

void SerialDMXDevice::performDeviceWrite() {
	m_mutex.lock();
	// Perform write function
	if (m_updateFunction != 0)
		m_updateFunction(m_values);
	m_mutex.unlock();
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool SerialDMXDevice::open()
{
	// Nothing to open
	if (m_path == "")
		return false;
	
	if (true)
	{
		// Start up the device writing thread
		applyDeviceOptions();
		return true;
	}
	else
	{
		qWarning() << QString("Unable to open SerialDMX %1: %2")
			.arg(m_output).arg("Because the world is stupid");
		return false;
	}
}

bool SerialDMXDevice::close()
{
	// Make sure we stop trying to write to the device first
	if (m_updateTimer != 0) {
		m_updateTimer->stop();
		delete m_updateTimer;
	}
	m_updateTimer = 0;
	// Make sure we have finished writing before we close
	m_mutex.lock();
	m_mutex.unlock();
	
	if (false)
	{
		qWarning() << QString("Unable to close SerialDMX %1: %2")
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

void SerialDMXDevice::write(t_channel channel, t_value value)
{
	m_mutex.lock();
	m_values[channel + 1] = value;
	dataChanged();
	m_mutex.unlock();
}

void SerialDMXDevice::writeRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(m_values + address + 1, values, num);
	dataChanged();
	m_mutex.unlock();
}

void SerialDMXDevice::read(t_channel channel, t_value* value)
{
	m_mutex.lock();
	*value = m_values[channel + 1];
	m_mutex.unlock();
}

void SerialDMXDevice::readRange(t_channel address, t_value* values, t_channel num)
{
	Q_ASSERT(address + num <= 512);

	m_mutex.lock();
	memcpy(values, m_values + address + 1, num);
	m_mutex.unlock();
}
