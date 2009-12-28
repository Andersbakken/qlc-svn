/*
  Q Light Controller
  midiinput.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef MIDIINPUT_H
#define MIDIINPUT_H

#include <QStringList>
#include <QString>
#include <QList>

#include "qlcinplugin.h"
#include "qlctypes.h"

class MIDIDevice;
class MIDIInput;

/*****************************************************************************
 * MIDIInput
 *****************************************************************************/

class MIDIInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** \reimp */
	void init();

	/** \reimp */
	virtual ~MIDIInput();

	/** \reimp */
	void open(t_input input = 0);

	/** \reimp */
	void close(t_input input = 0);

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	/** Find out what devices are present */
	void rescanDevices();

	/** Get a device by its index number */
	MIDIDevice* device(unsigned int index);

	/** Add a new device (as a result of rescanDevices) */
	void addDevice(MIDIDevice* device);

	/** Remove an existing device (as a result of rescanDevices) */
	void removeDevice(MIDIDevice* device);

	/** Get a list of currently available devices */
	const QList <MIDIDevice*>& devices() { return m_devices; }

signals:
	/** Tells that a new device has been made available */
	void deviceAdded(MIDIDevice* device);

	/** Tells that an existing device is no more available */
	void deviceRemoved(MIDIDevice* device);

protected:
	QList <MIDIDevice*> m_devices;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/** \reimp */
	QString name();

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	/** \reimp */
	QStringList inputs();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	/** \reimp */
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/** \reimp */
	QString infoText(t_input input = KInputInvalid);

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected slots:
	/** Catches input events from MidiInProc */
	void slotDeviceValueChanged(MIDIDevice* device,
				    t_input_channel channel,
				    t_input_value value);

signals:
	/** \reimp */
	void valueChanged(QLCInPlugin* plugin, t_input line,
			  t_input_channel channel, t_input_value value);

public:
	/** \reimp */
	void connectInputData(QObject* listener);

	/** \reimp */
	void feedBack(t_input input, t_input_channel channel,
		      t_input_value value);
};

#endif
