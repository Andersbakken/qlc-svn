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
class MIDIInput;
class QString;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
    Q_OBJECT

public:
    MIDIDevice(MIDIInput* parent, const snd_seq_addr_t* address);
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
    /** Extract the name of this device from ALSA. */
    void extractName();

protected:
    /** The name of this ALSA MIDI device */
    QString m_name;

    /*********************************************************************
     * MIDI channel
     *********************************************************************/
public:
    /** Get this device's MIDI channel */
    uchar midiChannel() const { return m_midiChannel; }

    /** Set this device's MIDI channel */
    void setMidiChannel(uchar channel) { m_midiChannel = channel; }

protected:
    uchar m_midiChannel;

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
    void valueChanged(MIDIDevice* device, quint32 channel, uchar value);

public:
    /**
     * Send an input value back the device to move motorized sliders
     * and such.
     */
    void feedBack(quint32 channel, uchar value);
};

#endif
