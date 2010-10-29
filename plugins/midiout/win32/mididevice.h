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

#include <Windows.h>
#include <QObject>

#include "qlctypes.h"

class MIDIDevice;
class MIDIOut;
class QString;

/** MIDI devices can have only 128 real channels */
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
    MIDIDevice(MIDIOut* parent, UINT id);
    virtual ~MIDIDevice();

    /** Load global settings */
    void loadSettings();

    /** Save global settings */
    void saveSettings();

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** Attempt to open the device in write-only mode */
    bool open();

    /** Close the device */
    void close();

protected:
    HMIDIOUT m_handle;

    /*********************************************************************
     * Output
     *********************************************************************/
public:
    quint32 output() const {
        return static_cast<quint32> (m_id);
    }

protected:
    UINT m_id;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** Get device information string to be used in plugin manager */
    QString infoText();

    /** Get the device's name */
    QString name() const;

protected:
    /** Extract the name of this device */
    void extractName();

protected:
    /** The name of this MIDI device */
    QString m_name;
    bool m_isOK;

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
    /** Send one channel */
    void sendData(BYTE command, BYTE channel, BYTE value);

protected:
    /**
     * Cache values here because MIDI is so slow that we need to send only
     * those values that have actually changed.
     */
    BYTE m_values[MAX_MIDI_DMX_CHANNELS];
};

#endif
