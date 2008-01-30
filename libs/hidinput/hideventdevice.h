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

#include <qobject.h>
#include <qfile.h>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/types.h>

#include "common/types.h"
#include "hiddevice.h"

class HIDInput;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDEventDevice : public HIDDevice
{
	Q_OBJECT
		
public:
	HIDEventDevice(HIDInput* parent, const QString& path);
	virtual ~HIDEventDevice();

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
	 * Find out the HID device's capabilities
	 */
	void getCapabilities();

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

protected:
	struct input_id m_deviceInfo;
	uint8_t m_eventTypes[(EV_MAX/8) + 1];

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
signals:
	/**
	 * Signal that is emitted when an input channel's value is changed
	 */
	void valueChanged(t_input_channel channel, t_input_value value);

public:
	/**
	 * Send an input value back the HID device to move motorized sliders
	 * and such.
	 */
	void feedBack(t_input_channel channel, t_input_value value);
};

#endif
