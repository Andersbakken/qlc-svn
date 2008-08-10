/*
  Q Light Controller
  hiddevice.h

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

#ifndef HIDDEVICE_H
#define HIDDEVICE_H

#include <QObject>
#include <QFile>

#include <sys/ioctl.h>
#include <linux/types.h>

#include "common/qlctypes.h"

class HIDInput;

/*****************************************************************************
 * HIDDevice
 *****************************************************************************/

class HIDDevice : public QObject
{
	Q_OBJECT
		
public:
	HIDDevice(HIDInput* parent, t_input line, const QString& path);
	virtual ~HIDDevice();

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
	virtual bool open();

	/**
	 * Close the HID device
	 */
	virtual void close();

	/**
	 * Get the full path of this HID device
	 */
	virtual QString path() const;

	/**
	 * Get the device's file descriptor
	 */
	virtual int handle() const;

	/**
	 * Read one event and emit it
	 */
	virtual void readEvent() = 0;

protected:
	QFile m_file;

	/*********************************************************************
	 * Line
	 *********************************************************************/
public:
	t_input line() const { return m_line; }

protected:
	t_input m_line;

	/*********************************************************************
	 * Enabled status
	 *********************************************************************/
public:
	/**
	 * Get the device's enabled state (whether it sends events to QLC)
	 */
	virtual bool isEnabled();

	/**
	 * Set the device's enabled state (whether it sends events to QLC)
	 */
	virtual void setEnabled(bool state);

	/*********************************************************************
	 * Device info
	 *********************************************************************/
public:
	/**
	 * Get HID device information string to be used in plugin manager
	 */
	virtual QString infoText();

	/**
	 * Get the device's name
	 */
	virtual QString name();

	/**
	 * Get number of channels provided by the given device
	 */
	virtual t_input_channel channels();

protected:
	QString m_name;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
signals:
	/**
	 * Signal that is emitted when an input channel's value is changed
	 *
	 * @param device The eventing HIDDevice
	 * @param channel The channel whose value has changed
	 * @param value The changed value
	 */
	void valueChanged(HIDDevice* device, t_input_channel channel,
			  t_input_value value);

public:
	/**
	 * Send an input value back the HID device to move motorized sliders
	 * and such.
	 */
	virtual void feedBack(t_input_channel channel, t_input_value value);
};

#endif
