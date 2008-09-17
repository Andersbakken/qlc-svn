/*
  Q Light Controller
  midipoller.h
  
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

#ifndef MIDIPOLLER_H
#define MIDIPOLLER_H

#include <QThread>
#include <QMutex>
#include <QMap>

class MIDIDevice;
class MIDIInput;

class MIDIPoller : public QThread
{
	Q_OBJECT

public:
	/**
	 * Construct a new MIDIReader thread. The parent object will receive
	 * all input events, so it must not be NULL.
	 */
	MIDIPoller(MIDIInput* parent);
	virtual ~MIDIPoller();

	/*********************************************************************
	 * Polled devices
	 *********************************************************************/
public:
	bool addDevice(MIDIDevice* device);
	bool removeDevice(MIDIDevice* device);
	int deviceCount() const { return m_devices.count(); }

protected:
	QMap <int, MIDIDevice*> m_devices;
	bool m_changed;
	QMutex m_mutex;
	
	/*********************************************************************
	 * Poller thread
	 *********************************************************************/
public:
	virtual void stop();

protected:
	virtual void run();
	void readEvent(/*struct pollfd pfd*/);

protected:
	bool m_running;
};

#endif
