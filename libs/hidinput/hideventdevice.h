/*
  Q Light Controller
  hideventdevice.h

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

#ifndef HIDEVENTDEVICE_H
#define HIDEVENTDEVICE_H

#include <QObject>
#include <QFile>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/types.h>

#include "common/qlctypes.h"
#include "hiddevice.h"

class HIDInput;

class HIDEventDeviceChannel
{
public:
	HIDEventDeviceChannel(int channel, int type, int min, int max)
	{
		m_channel = channel;
		m_type = type;
		m_min = min;
		m_max = max;
	}

	virtual ~HIDEventDeviceChannel() { }
	
	HIDEventDeviceChannel& operator=(const HIDEventDeviceChannel& channel)
	{
		if (this != &channel)
		{
			m_type = channel.m_type;
			m_channel = channel.m_channel;
			m_min = channel.m_min;
			m_max = channel.m_max;
		}
		return *this;
	}

	int m_channel;
	int m_type;
	int m_min;
	int m_max;
};

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/
class HIDEventDevice;

class HIDEventDevice : public HIDDevice
{
	Q_OBJECT
		
public:
	HIDEventDevice(HIDInput* parent, t_input line, const QString& path);
	virtual ~HIDEventDevice();

protected:
	/**
	 * Initialize the device, find out its capabilities etc.
	 */
	void init();

	/**
	 * Find out the HID device's capabilities
	 */
	void getCapabilities();

	/**
	 * Find out the capabilities of absolute axes
	 */
	void getAbsoluteAxesCapabilities();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/**
	 * Attempt to open the HID device in RW mode and fall back to RO
	 * if that fails.
	 *
	 * @return true if the file was opened RW/RO
	 */
	bool open();

	/**
	 * Close the HID device
	 */
	void close();

	/**
	 * Get the full path of this HID device
	 */
	QString path() const;

	/**
	 * Get number of channels provided by the given device
	 */
	t_input_channel channels();

	/**
	 * Read one event and emit it
	 */
	void readEvent();

protected:
	struct input_id m_deviceInfo;
	uint8_t m_eventTypes[(EV_MAX/8) + 1];
	QList <HIDEventDeviceChannel*> m_channels;

	/*********************************************************************
	 * Enabled status
	 *********************************************************************/
public:
	/**
	 * Get the device's enabled state (whether it sends events to QLC)
	 */
	bool isEnabled();

	/**
	 * Set the device's enabled state (whether it sends events to QLC)
	 */
	void setEnabled(bool state);

	/*********************************************************************
	 * Device info
	 *********************************************************************/
public:
	/**
	 * Get HID device information string to be used in plugin manager
	 */
	QString infoText();

	/*********************************************************************
	 * Input data
	 *********************************************************************/
public:
	/**
	 * Send an input value back the HID device to move motorized sliders
	 * and such.
	 */
	void feedBack(t_input_channel channel, t_input_value value);
};

#endif
