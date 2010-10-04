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
#include <QHash>

#include <alsa/asoundlib.h>

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

    /** Destructor */
    virtual ~MIDIPoller();

    /*********************************************************************
     * Polled devices
     *********************************************************************/
public:
    /** Convert an address into a quint64 that can be used as a hash key */
    quint64 addressHash(const snd_seq_addr_t* address) const;

    /** Add a new MIDI device to be polled for events */
    bool addDevice(MIDIDevice* device);

    /** Remove the given device from the poller list */
    bool removeDevice(MIDIDevice* device);

protected:
    /** Subscribe a device's events to come thru to the plugin's port */
    void subscribeDevice(MIDIDevice* device);

    /** Unsubscribe a device's events */
    void unsubscribeDevice(MIDIDevice* device);

protected:
    QHash <quint64, MIDIDevice*> m_devices;

    /*********************************************************************
     * Poller thread
     *********************************************************************/
public:
    void stop();

protected:
    /** Poller thread method */
    void run();

    /** Read events from the sequencer interface */
    void readEvent(snd_seq_t* alsa);

protected:
    bool m_running;
    bool m_changed;
    QMutex m_mutex;
};

#endif
