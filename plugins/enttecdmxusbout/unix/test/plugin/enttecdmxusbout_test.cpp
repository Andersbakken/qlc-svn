/*
  Q Light Controller - Unit tests
  enttecdmxusbout_test.cpp

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

#include <QtTest>
#include <ftdi.h>

#include "enttecdmxusbout_test.h"
#define protected public
#include "enttecdmxusbout.h"
#undef protected
#include "ftdimock.h"
#include "mockutil.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"

/****************************************************************************
 * EnttecOpenMock
 ****************************************************************************/

QByteArray _open_send_dmx_expected_data;
int _open1_send_dmx_called = 0;
int _open2_send_dmx_called = 0;

int _open1_called = 0;
int _open2_called = 0;

int _close1_called = 0;
int _close2_called = 0;

EnttecOpenMock::EnttecOpenMock(QObject* parent, const QString& name, const QString& serial)
    : EnttecDMXUSBOpen(parent, name, serial)
{
}

EnttecOpenMock::~EnttecOpenMock()
{
}

bool EnttecOpenMock::open()
{
    if (serial() == "1")
        _open1_called++;
    else
        _open2_called++;
    return true;
}

bool EnttecOpenMock::close()
{
    if (serial() == "1")
        _close1_called++;
    else
        _close2_called++;
    return true;
}

bool EnttecOpenMock::sendDMX(const QByteArray& data)
{
    UT_ASSERT(data == _open_send_dmx_expected_data);

    if (serial() == "1")
        _open1_send_dmx_called++;
    else
        _open2_send_dmx_called++;

    return true;
}

/****************************************************************************
 * EnttecDMXUSBOut_Test
 ****************************************************************************/

void EnttecDMXUSBOut_Test::name()
{
    EnttecDMXUSBOut plugin;
    QCOMPARE(plugin.name(), QString("Enttec DMX USB Output"));
}

void EnttecDMXUSBOut_Test::rescanEnttecPro()
{
    ftdimock_reset_variables();

    // Construct a list with 1 mock device
    struct ftdi_device_list list;
    list.dev = (struct usb_device*) 0xDEADBEEF;
    list.next = NULL;
    _ftdi_usb_find_all_expected_devlist = &list;
    _ftdi_usb_get_strings_expected_device = (struct usb_device*) 0xDEADBEEF;

    _ftdi_usb_get_strings_expected_manufacturer = "ENTTEC";
    _ftdi_usb_get_strings_expected_description = "Enttec DMX USB Pro";
    _ftdi_usb_get_strings_expected_serial = "1234567890";

    // Load expected variables to ftdimock for enttec pro serial query
    _ftdi_usb_open_desc_expected_description = _ftdi_usb_get_strings_expected_description;
    _ftdi_usb_open_desc_expected_serial = _ftdi_usb_get_strings_expected_serial;
    _ftdi_write_data_expected_size = 5;
    const char proSerialOK[9] = { 0x7e, 0x0a, 0x04, 0x00, 0x11, 0x22, 0x33, 0x44, 0xe7 };
    _ftdi_read_data_expected_reply = proSerialOK;
    _ftdi_read_data_expected_size = sizeof(proSerialOK);

    EnttecDMXUSBOut* plugin = new EnttecDMXUSBOut;
    plugin->init();

    QCOMPARE(_ftdi_init_called, 2); // Another time with the mock dongle
    QCOMPARE(_ftdi_usb_find_all_called, 1);
    QCOMPARE(_ftdi_usb_get_strings_called, 1);
    QCOMPARE(_ftdi_deinit_called, 1);
    QVERIFY(plugin->m_widgets.size() == 1);
    QVERIFY(plugin->m_widgets.at(0)->type() == EnttecDMXUSBWidget::Pro);

    QVERIFY(plugin->widget("1234567890") == plugin->m_widgets.at(0));

    // Do a rescan and make sure the device is still there
    plugin->rescanWidgets();
    QCOMPARE(_ftdi_init_called, 4); // Another time with the mock dongle
    QCOMPARE(_ftdi_usb_find_all_called, 2);
    QCOMPARE(_ftdi_usb_get_strings_called, 2);
    QCOMPARE(_ftdi_deinit_called, 3);
    QVERIFY(plugin->m_widgets.size() == 1);
    QVERIFY(plugin->m_widgets.at(0)->type() == EnttecDMXUSBWidget::Pro);
    QVERIFY(plugin->widget("1234567890") == plugin->m_widgets.at(0));

    delete plugin;
}

void EnttecDMXUSBOut_Test::rescanEnttecOpen()
{
    ftdimock_reset_variables();

    // Construct a list with 1 mock device
    struct ftdi_device_list list;
    list.dev = (struct usb_device*) 0xDEADBEEF;
    list.next = NULL;
    _ftdi_usb_find_all_expected_devlist = &list;
    _ftdi_usb_get_strings_expected_device = (struct usb_device*) 0xDEADBEEF;

    _ftdi_usb_get_strings_expected_manufacturer = "foobar";
    _ftdi_usb_get_strings_expected_description = "Enttec DMX USB Open";
    _ftdi_usb_get_strings_expected_serial = "0987654321";

    EnttecDMXUSBOut* plugin = new EnttecDMXUSBOut;
    plugin->init();

    // Do a rescan and make sure the device is still there
    QCOMPARE(_ftdi_init_called, 2); // Another time with the mock dongle
    QCOMPARE(_ftdi_usb_find_all_called, 1);
    QCOMPARE(_ftdi_usb_get_strings_called, 1);
    QCOMPARE(_ftdi_deinit_called, 1);
    QVERIFY(plugin->m_widgets.size() == 1);
    QVERIFY(plugin->m_widgets.at(0)->type() == EnttecDMXUSBWidget::Open);

    QVERIFY(plugin->widget("0987654321") == plugin->m_widgets.at(0));

    delete plugin;
}

void EnttecDMXUSBOut_Test::outputs()
{
    ftdimock_reset_variables();

    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1234567890");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "0987654321");
    plugin.m_widgets.append(&open2);

    QStringList outputs(plugin.outputs());
    QCOMPARE(outputs.size(), 2);
    QCOMPARE(outputs.at(0), QString("1: Enttec DMX USB Open (S/N: 1234567890)"));
    QCOMPARE(outputs.at(1), QString("2: Enttec DMX USB Open (S/N: 0987654321)"));
}

void EnttecDMXUSBOut_Test::outputDMX()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.outputDMX(0, QByteArray());
    QCOMPARE(_open1_send_dmx_called, 1);
    QCOMPARE(_open2_send_dmx_called, 0);

    _open1_send_dmx_called = 0;
    _open2_send_dmx_called = 0;

    plugin.outputDMX(1, QByteArray());
    QCOMPARE(_open1_send_dmx_called, 0);
    QCOMPARE(_open2_send_dmx_called, 1);

    _open1_send_dmx_called = 0;
    _open2_send_dmx_called = 0;

    plugin.outputDMX(2, QByteArray());
    QCOMPARE(_open1_send_dmx_called, 0);
    QCOMPARE(_open2_send_dmx_called, 0);
}

void EnttecDMXUSBOut_Test::open()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.open(0);
    QCOMPARE(_open1_called, 1);
    QCOMPARE(_open2_called, 0);

    _open1_called = 0;
    _open2_called = 0;

    plugin.open(1);
    QCOMPARE(_open1_called, 0);
    QCOMPARE(_open2_called, 1);

    _open1_called = 0;
    _open2_called = 0;

    plugin.open(2);
    QCOMPARE(_open1_called, 0);
    QCOMPARE(_open2_called, 0);
}

void EnttecDMXUSBOut_Test::close()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.close(0);
    QCOMPARE(_close1_called, 1);
    QCOMPARE(_close2_called, 0);

    _close1_called = 0;
    _close2_called = 0;

    plugin.close(1);
    QCOMPARE(_close1_called, 0);
    QCOMPARE(_close2_called, 1);

    _close1_called = 0;
    _close2_called = 0;

    plugin.close(2);
    QCOMPARE(_close1_called, 0);
    QCOMPARE(_close2_called, 0);
}

QTEST_MAIN(EnttecDMXUSBOut_Test)
