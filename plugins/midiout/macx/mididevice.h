/*
  Q Light Controller
  mididevice.h

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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <QObject>
#include <QFile>

#include "qlctypes.h"

class MIDIDevice;
class MIDIOut;
class QString;

#define MAX_MIDI_DMX_CHANNELS 128

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    MIDIDevice(MIDIOut* parent, MIDIEntityRef entity);
    virtual ~MIDIDevice();

    /** Get the device's unique ID and store it to m_uid */
    bool extractUID();

    /** Get the device's name and store it to m_name */
    bool extractName();

    /** Load global settings */
    void loadSettings();

    /** Save global settings */
    void saveSettings();

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** Open the device for output data */
    bool open();

    /** Close the output device */
    void close();

protected:
    /** Reference to the MIDI entity that owns the destination */
    MIDIEntityRef m_entity;

    /** Reference to the opened MIDI destination */
    MIDIEndpointRef m_destination;

    /** Reference to a virtual port that sends data to the destination */
    MIDIPortRef m_outPort;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    QString infoText() const;
    QString name() const {
        return m_name;
    }
    SInt32 uid() const {
        return m_uid;
    }

protected:
    QString m_name;
    SInt32 m_uid;

    /*********************************************************************
     * Operational mode
     *********************************************************************/
public:
    /**
     * This device's operational mode.
     *
     * @ControlChange: Use MIDI ControlChange ID's as DMX channels
     * @Note: Use MIDI Note ON/OFF commands as DMX channels
     */
    enum Mode
    {
        ControlChange,
        Note
    };

    /** Get this device's operational mode */
    Mode mode() const {
        return m_mode;
    }

    /** Set this device's operational mode */
    void setMode(Mode m) {
        m_mode = m;
    }

    static QString modeToString(Mode mode);
    static Mode stringToMode(const QString& mode);

protected:
    Mode m_mode;

    /*********************************************************************
     * MIDI channel
     *********************************************************************/
public:
    /** Get this device's MIDI channel */
    quint32 midiChannel() const {
        return m_midiChannel;
    }

    /** Set this device's MIDI channel */
    void setMidiChannel(quint32 channel) {
        m_midiChannel = channel;
    }

protected:
    quint32 m_midiChannel;

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);

protected:
    uchar m_values[MAX_MIDI_DMX_CHANNELS];
};

#endif
