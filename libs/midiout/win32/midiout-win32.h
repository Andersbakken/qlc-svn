/*
  Q Light Controller
  midiout-win32.h

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

#ifndef MIDIOUT_H
#define MIDIOUT_H

#include <QStringList>
#include <QtPlugin>
#include <QList>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class ConfigureMIDIOut;
class MIDIDevice;
class QString;

/*****************************************************************************
 * MIDIOut
 *****************************************************************************/

class MIDIOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	friend class ConfigureMIDIOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Destructor */
	~MIDIOut();

	/** Second-stage constructor */
	void init();

	/** Open the given output */
	void open(t_output output = 0);

	/** Close the given output */
	void close(t_output output = 0);

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	/** Find out what kinds of MIDI devices there are available */
	void rescanDevices();

protected:
	/** Get a MIDIDevice by its index in the QList */
	MIDIDevice* device(unsigned int index);

	/** Add a new MIDIDevice and associate it with the given output */
	void addDevice(MIDIDevice* device);

	/** Remove the MIDIDevice associated with the output from the QMap */
	void removeDevice(MIDIDevice* device);

signals:
	/** Configuration dialog listens to this signal to refill its list */
	void deviceAdded(MIDIDevice* device);

	/** Configuration dialog listens to this signal to refill its list */
	void deviceRemoved(MIDIDevice* device);

protected:
	/** A map of available MIDI devices */
	QList <MIDIDevice*> m_devices;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/** Get the name of this plugin */
	QString name();

	/*********************************************************************
	 * Oututs
	 *********************************************************************/
public:
	/** Get the number of outputs provided by this plugin */
	QStringList outputs();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	/** Configure this plugin */
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	/** Get a short information snippet on the given output's state */
	QString infoText(t_output output = KOutputInvalid);

	/*********************************************************************
	 * Value read/write methods
	 *********************************************************************/
public:
	void writeChannel(t_output output, t_channel channel, t_value value);
	void writeRange(t_output output, t_channel address, t_value* values,
			t_channel num);

	void readChannel(t_output output, t_channel channel, t_value* value);
	void readRange(t_output output, t_channel address, t_value* values,
		       t_channel num);
};

#endif
