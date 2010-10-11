/*
  Q Light Controller - Unit tests
  enttecdmxusbout_test.h

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

#ifndef ENTTECDMXUSBOUT_TEST_H
#define ENTTECDMXUSBOUT_TEST_H

#include <QObject>
#include "enttecdmxusbopen.h"

/****************************************************************************
 * EnttecOpenMock
 ****************************************************************************/

class EnttecOpenMock : public EnttecDMXUSBOpen
{
public:
    EnttecOpenMock(QObject* parent, const QString& name, const QString& serial);
    ~EnttecOpenMock();

    bool open();
    bool close();
    bool sendDMX(const QByteArray& data);

public:
    int m_open_called;
    int m_close_called;
    int m_send_dmx_called;
    QByteArray m_send_dmx_expected_data;
};

/****************************************************************************
 * EnttecDMXUSBOut_Test
 ****************************************************************************/

class EnttecDMXUSBOut_Test : public QObject
{
    Q_OBJECT

private slots:
    void name();
    void outputs();
    void rescanEnttecPro();
    void rescanEnttecOpen();
    void outputDMX();
    void open();
    void close();
    void infoText();

#ifdef DBUS_ENABLED
    void slotDeviceAdded();
    void slotDeviceRemoved();
#endif
};

#endif
