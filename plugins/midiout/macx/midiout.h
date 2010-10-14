/*
  Q Light Controller
  midiout.h

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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <QStringList>
#include <QList>

#include "qlcoutplugin.h"

class ConfigureMIDIOut;
class MIDIDevice;
class QString;

/*****************************************************************************
 * MIDIOut
 *****************************************************************************/

class MIDIOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    friend class ConfigureMIDIOut;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    void init();

    /** @reimp */
    virtual ~MIDIOut();

    /** @reimp */
    QString name();

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    void open(quint32 output = 0);

    /** @reimp */
    void close(quint32 output = 0);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString infoText(quint32 output = KOutputInvalid);

    /** @reimp */
    void outputDMX(quint32 output, const QByteArray& universe);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /*********************************************************************
     * Devices
     *********************************************************************/
public:
    /** Get the OSX MIDI client */
    const MIDIClientRef client() const;

    /** Find out what kinds of MIDI devices there are available */
    void rescanDevices();

protected:
    /** Get a MIDIDevice with the given uid or NULL if not found */
    MIDIDevice* deviceByUID(SInt32 uid);

    /** Get a MIDIDevice at the given output index or NULL if over limits */
    MIDIDevice* device(quint32 output);

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

    /** The OSX MIDI client */
    MIDIClientRef m_client;
};

#endif
