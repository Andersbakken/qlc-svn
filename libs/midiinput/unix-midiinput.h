/*
  Q Light Controller
  unix-midiinput.h

  Copyright (C) Heikki Junnila
                Stefan Krumm

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
#include <QEvent>

#include <alsa/asoundlib.h>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

class ConfigureMIDIInput;
class MIDIInputEvent;
class MIDIPoller;
class MIDIDevice;
class MIDIInput;

/*****************************************************************************
 * MIDIInput
 *****************************************************************************/

class MIDIInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	friend class ConfigureMIDIInput;
	friend class MIDIPoller;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	~MIDIInput();

	void init();

	/** Open the given input for input data */
	void open(t_input input = 0);

	/** Close the given input */
	void close(t_input input = 0);

	/*********************************************************************
	 * ALSA
	 *********************************************************************/
protected:
	/** Initialize ALSA for proper operation */
	void initALSA();

public:
	/** Get the ALSA sequencer handle */
	snd_seq_t* alsa() { return m_alsa; }

	/** Get the plugin's own ALSA port that collates all events */
	const snd_seq_addr_t* address() { return m_address; }

protected:
	/** The plugin's ALSA sequencer interface handle */
	snd_seq_t* m_alsa;

	/** This sequencer client's port address */
	snd_seq_addr_t* m_address;

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	void rescanDevices();

protected:
	MIDIDevice* device(const snd_seq_addr_t* address);
	MIDIDevice* device(unsigned int index);

	void addDevice(MIDIDevice* device);
	void removeDevice(MIDIDevice* device);

signals:
	void deviceAdded(MIDIDevice* device);
	void deviceRemoved(MIDIDevice* device);

protected:
	/** The list of available MIDI devices */
	QList <MIDIDevice*> m_devices;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	QStringList inputs();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText(t_input input = KInputInvalid);

	/*********************************************************************
	 * Device poller
	 *********************************************************************/
public:
	void addPollDevice(MIDIDevice* device);
	void removePollDevice(MIDIDevice* device);

protected:
	MIDIPoller* m_poller;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected:
	void customEvent(QEvent* event);

signals:
	void valueChanged(QLCInPlugin* plugin, t_input line,
			  t_input_channel channel, t_input_value value);

public:
	void connectInputData(QObject* listener);

	void feedBack(t_input input, t_input_channel channel,
		      t_input_value value);
};

#endif
