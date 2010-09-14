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
#include <QHash>

#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/types.h>
#include <stdint.h>

#include "qlctypes.h"
#include "hiddevice.h"

class HIDEventDevice;
class HIDInput;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDEventDevice : public HIDDevice
{
	Q_OBJECT
		
public:
	HIDEventDevice(HIDInput* parent, quint32 line, const QString& path);
	virtual ~HIDEventDevice();

protected:
	/**
	 * Initialize the device, find out its capabilities etc.
	 */
	void init();

	/**
	 * Find out the HID device's capabilities
	 */
	void getCapabilities(uint8_t* mask);

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
	 * Read one event and emit it
	 */
	bool readEvent();

protected:
	/** Scaling values for absolute/relative axes */
	QHash <int, struct input_absinfo> m_scales;

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
	void feedBack(quint32 channel, uchar value);
};

#endif
