/*
  Q Light Controller
  unix-mididevice.h

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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <QObject>
#include <QFile>

#include "common/qlctypes.h"

class MIDIInput;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
	Q_OBJECT
		
public:
	MIDIDevice(MIDIInput* parent, t_input line, const QString& path);
	virtual ~MIDIDevice();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/**
	 * Attempt to open the device in RW mode and fall back to RO
	 * if that fails.
	 *
	 * @return true if the file was opened RW/RO
	 */
	virtual bool open();

	/**
	 * Close the device
	 */
	virtual void close();

	/**
	 * Get the full path of this device
	 */
	virtual QString path() const;

	/**
	 * Get the device's file descriptor
	 */
	virtual int handle() const;

	/**
	 * Read one event and emit it.
	 */
	virtual bool readEvent();

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
	 * Device info
	 *********************************************************************/
public:
	/**
	 * Get device information string to be used in plugin manager
	 */
	virtual QString infoText();

	/**
	 * Get the device's name
	 */
	virtual QString name();

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
	void valueChanged(MIDIDevice* device, t_input_channel channel,
			  t_input_value value);

public:
	/**
	 * Send an input value back the device to move motorized sliders
	 * and such.
	 */
	virtual void feedBack(t_input_channel channel, t_input_value value);
};

#endif
