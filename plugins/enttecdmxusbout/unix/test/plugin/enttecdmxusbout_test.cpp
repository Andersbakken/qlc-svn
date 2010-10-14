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

#include <QMessageBox>
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

EnttecOpenMock::EnttecOpenMock(QObject* parent, const QString& name,
                               const QString& serial)
    : EnttecDMXUSBOpen(parent, name, serial)
{
    m_open_called = 0;
    m_close_called = 0;
    m_send_dmx_called = 0;
}

EnttecOpenMock::~EnttecOpenMock()
{
}

bool EnttecOpenMock::open()
{
    m_open_called++;
    return true;
}

bool EnttecOpenMock::close()
{
    m_close_called++;
    return true;
}

bool EnttecOpenMock::sendDMX(const QByteArray& data)
{
    UT_ASSERT(data == m_send_dmx_expected_data);
    m_send_dmx_called++;
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

    plugin.m_widgets.clear();
}

void EnttecDMXUSBOut_Test::outputDMX()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.outputDMX(0, QByteArray());
    QCOMPARE(open.m_send_dmx_called, 1);
    QCOMPARE(open2.m_send_dmx_called, 0);

    open.m_send_dmx_called = 0;
    open2.m_send_dmx_called = 0;

    plugin.outputDMX(1, QByteArray());
    QCOMPARE(open.m_send_dmx_called, 0);
    QCOMPARE(open2.m_send_dmx_called, 1);

    open.m_send_dmx_called = 0;
    open2.m_send_dmx_called = 0;

    plugin.outputDMX(2, QByteArray());
    QCOMPARE(open.m_send_dmx_called, 0);
    QCOMPARE(open2.m_send_dmx_called, 0);

    plugin.m_widgets.clear();
}

void EnttecDMXUSBOut_Test::open()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.open(0);
    QCOMPARE(open.m_open_called, 1);
    QCOMPARE(open2.m_open_called, 0);

    open.m_open_called = 0;
    open2.m_open_called = 0;

    plugin.open(1);
    QCOMPARE(open.m_open_called, 0);
    QCOMPARE(open2.m_open_called, 1);

    open.m_open_called = 0;
    open2.m_open_called = 0;

    plugin.open(2);
    QCOMPARE(open.m_open_called, 0);
    QCOMPARE(open2.m_open_called, 0);

    plugin.m_widgets.clear();
}

void EnttecDMXUSBOut_Test::close()
{
    EnttecDMXUSBOut plugin;

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    plugin.close(0);
    QCOMPARE(open.m_close_called, 1);
    QCOMPARE(open2.m_close_called, 0);

    open.m_close_called = 0;
    open2.m_close_called = 0;

    plugin.close(1);
    QCOMPARE(open.m_close_called, 0);
    QCOMPARE(open2.m_close_called, 1);

    open.m_close_called = 0;
    open2.m_close_called = 0;

    plugin.close(2);
    QCOMPARE(open.m_close_called, 0);
    QCOMPARE(open2.m_close_called, 0);

    plugin.m_widgets.clear();
}

void EnttecDMXUSBOut_Test::infoText()
{
    EnttecDMXUSBOut plugin;

    QVERIFY(plugin.infoText(KOutputInvalid).contains("No devices available."));
    QVERIFY(!plugin.infoText(0).contains("No devices available."));

    EnttecOpenMock open(&plugin, "Enttec DMX USB Open", "1");
    plugin.m_widgets.append(&open);

    EnttecOpenMock open2(&plugin, "Enttec DMX USB Open", "2");
    plugin.m_widgets.append(&open2);

    QVERIFY(plugin.infoText(0).contains("Device is operating correctly."));
    QVERIFY(plugin.infoText(1).contains("Device is operating correctly."));
    QVERIFY(!plugin.infoText(2).contains("Device is operating correctly."));
    QVERIFY(plugin.infoText(KOutputInvalid).contains("This plugin provides"));
    QVERIFY(!plugin.infoText(0).contains("This plugin provides"));
    QVERIFY(!plugin.infoText(1).contains("This plugin provides"));
    QVERIFY(!plugin.infoText(2).contains("This plugin provides"));

    plugin.m_widgets.clear();
}

#ifdef DBUS_ENABLED
void EnttecDMXUSBOut_Test::slotDeviceAdded()
{
    ftdimock_reset_variables();

    EnttecDMXUSBOut plugin;

    plugin.slotDeviceAdded("foobar");
    QCOMPARE(_ftdi_usb_find_all_called, 0);

    // @todo Change to a real string!
    plugin.slotDeviceAdded("/org/freedesktop/Hal/devices/usb_device_403_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    plugin.slotDeviceAdded("foobar");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    plugin.slotDeviceAdded("/org/freedesktop/Hal/devices/usb_device_413_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    // @todo Change to a real string!
    plugin.slotDeviceAdded("/org/freedesktop/Hal/devices/usb_device_403_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 2);
}

void EnttecDMXUSBOut_Test::slotDeviceRemoved()
{
    ftdimock_reset_variables();

    EnttecDMXUSBOut plugin;

    plugin.slotDeviceRemoved("foobar");
    QCOMPARE(_ftdi_usb_find_all_called, 0);

    // @todo Change to a real string!
    plugin.slotDeviceRemoved("/org/freedesktop/Hal/devices/usb_device_403_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    plugin.slotDeviceRemoved("foobar");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    plugin.slotDeviceRemoved("/org/freedesktop/Hal/devices/usb_device_413_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 1);

    // @todo Change to a real string!
    plugin.slotDeviceRemoved("/org/freedesktop/Hal/devices/usb_device_403_x_y_if0_serial_usb_z");
    QCOMPARE(_ftdi_usb_find_all_called, 2);
}

#endif

QTEST_MAIN(EnttecDMXUSBOut_Test)
