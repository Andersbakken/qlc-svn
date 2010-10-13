/*
  Q Light Controller
  hidinput.h

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

#ifndef HIDINPUT_H
#define HIDINPUT_H

#include <QEvent>
#include <QList>

#include "qlcinplugin.h"

#include "hiddevice.h"
#include "hidpoller.h"

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

class HIDInputEvent : public QEvent
{
public:
    HIDInputEvent(HIDDevice* device, quint32 input, quint32 channel,
                  uchar value, bool alive);
    ~HIDInputEvent();

    HIDDevice* m_device;
    quint32 m_input;
    quint32 m_channel;
    uchar m_value;
    bool m_alive;
};

/*****************************************************************************
 * HIDInput
 *****************************************************************************/

class HIDInput : public QLCInPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCInPlugin)

    friend class ConfigureHIDInput;
    friend class HIDPoller;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    void init();

    /** @reimp */
    virtual ~HIDInput();

    /** @reimp */
    QString name();

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    /** @reimp */
    void open(quint32 input = 0);

    /** @reimp */
    void close(quint32 input = 0);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString infoText(quint32 input = KInputInvalid);

signals:
    /** @reimp */
    void valueChanged(quint32 line, quint32 channel, uchar value);

protected:
    void customEvent(QEvent* event);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

signals:
    /** @reimp */
    void configurationChanged();

    /*********************************************************************
     * Feedback
     *********************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 input, quint32 channel, uchar value);

    /*********************************************************************
     * Devices
     *********************************************************************/
public:
    void rescanDevices();

protected:
    HIDDevice* device(const QString& path);
    HIDDevice* device(quint32 index);

    void addDevice(HIDDevice* device);
    void removeDevice(HIDDevice* device);

signals:
    void deviceAdded(HIDDevice* device);
    void deviceRemoved(HIDDevice* device);

protected:
    QList <HIDDevice*> m_devices;

    /*********************************************************************
     * Device poller
     *********************************************************************/
public:
    void addPollDevice(HIDDevice* device);
    void removePollDevice(HIDDevice* device);

protected:
    HIDPoller* m_poller;
};

#endif
