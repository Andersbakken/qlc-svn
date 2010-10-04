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

#include <QObject>

#include "qlcoutplugin.h"
#include "qlctypes.h"

class EnttecDMXUSBWidget;

class EnttecDMXUSBOut : public QObject, public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    void init();
    void open(quint32 output);
    void close(quint32 output);

    /********************************************************************
     * Devices (ENTTEC calls them "widgets" and so shall we)
     ********************************************************************/
public:
    bool rescanWidgets();
    QStringList outputs();

protected:
    QList <EnttecDMXUSBWidget*> m_widgets;

    /********************************************************************
     * Name
     ********************************************************************/
public:
    QString name();

    /********************************************************************
     * Configuration
     ********************************************************************/
public:
    void configure();

    /********************************************************************
     * Plugin status
     ********************************************************************/
public:
    QString infoText(quint32 output = KOutputInvalid);

    /********************************************************************
     * Value read/write methods
     ********************************************************************/
public:
    void outputDMX(quint32 output, const QByteArray& universe);
};

#endif
