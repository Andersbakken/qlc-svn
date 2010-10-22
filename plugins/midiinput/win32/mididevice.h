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
class MIDIInput;
class QEvent;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
    Q_OBJECT

public:
    MIDIDevice(MIDIInput* parent, UINT id);
    virtual ~MIDIDevice();

    /** Load global settings */
    void loadSettings();

    /** Save global settings */
    void saveSettings();

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** Open the device for input data */
    bool open();

    /** Close the input device */
    void close();

protected:
    /** Handle to the opened MIDI input device */
    HMIDIIN m_handle;

    /** The ID of the windows MIDI input that this device represents */
    UINT m_id;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /**
     * Get device information string to be used in plugin manager
     */
    QString infoText();

    /**
     * Get the device's name
     */
    QString name();

protected:
    void extractName();

protected:
    QString m_name;
    bool m_isOK;

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
protected:
    void customEvent(QEvent* event);

signals:
    /** Signal that is emitted when an input channel's value is changed */
    void valueChanged(quint32 channel, uchar value);

    /*********************************************************************
     * Feedback
     *********************************************************************/
public:
    /** Get this input device's feedback output line number */
    UINT feedBackId() const;

    /** Set this input device's feedback output line number */
    void setFeedBackId(UINT id);

    /** Open the device for feedback output */
    void openOutput();

    /** Close the feedback device */
    void closeOutput();

    /** Get a list of available outputs for feedback */
    static QStringList feedBackNames();

public:
    /** Send input values back to the device to move motorized faders */
    void feedBack(quint32 channel, uchar value);

protected:
    UINT m_feedBackId;
    HMIDIOUT m_feedBackHandle;
};

#endif
