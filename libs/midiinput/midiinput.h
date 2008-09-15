/*
  Q Light Controller
  midiinput.h
  
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

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

class ConfigureMIDIInput;
class MIDIPoller;
class MIDIDevice;
class MIDIInput;

/*****************************************************************************
 * MIDIInputEvent
 *****************************************************************************/

class MIDIInputEvent : public QEvent
{
public:
	MIDIInputEvent(MIDIDevice* device, t_input input,
		       t_input_channel channel, t_input_value value,
		       bool alive);
	~MIDIInputEvent();

	MIDIDevice* m_device;
	t_input m_input;
	t_input_channel m_channel;
	t_input_value m_value;
	bool m_alive;
};

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
	void init();
	~MIDIInput();

	void open(t_input input = 0);
	void close(t_input input = 0);

	/*********************************************************************
	 * Devices
	 *********************************************************************/
public:
	void rescanDevices();

protected:
	MIDIDevice* device(const QString& path);
	MIDIDevice* device(unsigned int index);

	void addDevice(MIDIDevice* device);
	void removeDevice(MIDIDevice* device);

signals:
	void deviceAdded(MIDIDevice* device);
	void deviceRemoved(MIDIDevice* device);

protected:
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
	QString infoText();

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
