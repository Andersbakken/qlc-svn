/*
  Q Light Controller
  peperoniout.h

  Copyright (c)	Heikki Junnila

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

#ifndef PEPERONIOUT_H
#define PEPERONIOUT_H

#include <QStringList>
#include <QtPlugin>
#include <QList>

#include "qlcoutplugin.h"

class PeperoniDevice;
class QString;

/*****************************************************************************
 * USBDMXOut
 *****************************************************************************/

class PeperoniOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~PeperoniOut();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /*********************************************************************
     * Outputs
     *********************************************************************/
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
    void rescanDevices();

protected:
    /** Get a PeperoniDevice entry by its usbdev struct */
    PeperoniDevice* device(struct usb_device* usbdev);

protected:
    /** List of available devices */
    QList <PeperoniDevice*> m_devices;
};

#endif
