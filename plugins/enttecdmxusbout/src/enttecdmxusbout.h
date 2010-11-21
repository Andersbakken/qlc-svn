/*
  Q Light Controller
  enttecdmxusb.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef ENTTECDMXUSBOUT_H
#define ENTTECDMXUSBOUT_H

#include "qlcoutplugin.h"

class EnttecDMXUSBWidget;

class EnttecDMXUSBOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    virtual ~EnttecDMXUSBOut();

    /** @reimp */
    void init();

#ifdef DBUS_ENABLED
protected slots:
    /** Called when a USB device has been plugged in */
    void slotDeviceAdded(const QString& name);

    /** Called when a USB device has been plugged out */
    void slotDeviceRemoved(const QString& name);
#endif

    /** @reimp */
    QString name();

    /************************************************************************
     * Outputs
     ************************************************************************/
public:
    /** @reimp */
    void open(quint32 output);

    /** @reimp */
    void close(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString infoText(quint32 output = KOutputInvalid);

    /** @reimp */
    void outputDMX(quint32 output, const QByteArray& universe);

    /********************************************************************
     * Configuration
     ********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /********************************************************************
     * Devices (ENTTEC calls them "widgets" and so shall we)
     ********************************************************************/
public:
    /** Attempt to find all connected devices */
    bool rescanWidgets();

protected:
    /** Currently available devices */
    QList <EnttecDMXUSBWidget*> m_widgets;
};

#endif
