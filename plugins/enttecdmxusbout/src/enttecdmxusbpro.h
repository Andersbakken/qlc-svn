/*
  Q Light Controller
  enttecdmxusbpro.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#ifdef WIN32
#   include <windows.h>
#endif

#include <QByteArray>
#include <QObject>

#include "enttecdmxusbwidget.h"

class EnttecDMXUSBPro : public QObject, public EnttecDMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * Construct a new DMXUSBPro object with the given parent and
     * FTDI device information. Neither can be null.
     *
     * @param parent The owner of this object
     * @param info FTDI device information
     * @param id The ID of the device in FT2XX's internal structs
     */
    EnttecDMXUSBPro(const QString& serial, const QString& name,
                    quint32 id = 0, QObject* parent = 0);

    /** Destructor */
    virtual ~EnttecDMXUSBPro();

    /** @reimp */
    EnttecDMXUSBWidget::Type type() const;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString uniqueName() const;

protected:
    /** Extract the widget's unique serial number (printed on the bottom) */
    bool extractSerial();

protected:
    QString m_proSerial;

    /************************************************************************
     * DMX operations
     ************************************************************************/
public:
    /** @reimp */
    bool sendDMX(const QByteArray& universe);
};

#endif
