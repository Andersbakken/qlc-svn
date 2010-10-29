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

#include <QObject>
#include <QFile>

#include <alsa/asoundlib.h>

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
    MIDIDevice(MIDIOut* parent, const snd_seq_addr_t* address);
    virtual ~MIDIDevice();

    /** Load global settings */
    void loadSettings();

    /** Save global settings */
    void saveSettings();

    /*********************************************************************
      * ALSA address
     *********************************************************************/
public:
    /** Get the device's ALSA client:port address */
    const snd_seq_addr_t* address() const;

    /** Set the device's ALSA client:port address */
    void setAddress(const snd_seq_addr_t* address);

protected:
    snd_seq_addr_t* m_address;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** Get device information string to be used in plugin manager */
    QString infoText();

    /** Get the device's name */
    QString name() const;

protected:
    /** Extract the name of this device from ALSA */
    void extractName();

protected:
    /** The name of this ALSA MIDI device */
    QString m_name;

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
    /** Write the given values to MIDI output */
    void outputDMX(const QByteArray& universe);

protected:
    /**
     * Cache values since MIDI is so slow that we need to send only those
     * channels that have actually changed.
     */
    uchar m_values[MAX_MIDI_DMX_CHANNELS];
};

#endif
