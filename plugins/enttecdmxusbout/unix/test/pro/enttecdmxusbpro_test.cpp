/*
  Q Light Controller - Unit tests
  enttecdmxusbpro_test.cpp

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
#include <QDebug>
#include "enttecdmxusbpro_test.h"

#define protected public
#include "enttecdmxusbpro.h"
#undef protected

// Redefined Q_ASSERT because it is not present in release builds
#define UT_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())

#define TEST_NAME "Foobar"
#define TEST_SRNO "1234567890"

extern int _ftdi_init_called;
extern int _ftdi_deinit_called;

extern int _ftdi_usb_open_desc_called;
extern int _ftdi_usb_close_called;
extern int _ftdi_usb_reset_called;
extern int _ftdi_set_line_property_called;
extern int _ftdi_set_baudrate_called;
extern int _ftdi_setrts_called;

extern int _ftdi_write_data_called;
extern int _ftdi_write_data_expected_size;
extern int _ftdi_write_data_expected_return_value;
extern int _ftdi_read_data_called;
extern int _ftdi_read_data_expected_size;
extern int _ftdi_read_data_expected_return_value;

extern QString _ftdi_usb_open_desc_expected_description;
extern QString _ftdi_usb_open_desc_expected_serial;

/****************************************************************************
 * Unit test code
 ****************************************************************************/

void EnttecDMXUSBPro_Test::construction()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_usb_open_desc_expected_description = TEST_NAME;
	_ftdi_usb_open_desc_expected_serial = TEST_SRNO;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
	QCOMPARE(_ftdi_write_data_called, 1);
	QCOMPARE(_ftdi_read_data_called, 1);
	QCOMPARE(_ftdi_usb_close_called, 1);

        QCOMPARE(obj->m_name, QString(TEST_NAME));
        QCOMPARE(obj->m_serial, QString(TEST_SRNO));  
        QCOMPARE(obj->m_enttecSerial, QString("44332211"));  

	QCOMPARE(obj->name(), QString(TEST_NAME));
	QCOMPARE(obj->serial(), QString(TEST_SRNO));
	QCOMPARE(obj->uniqueName(), QString("%1 (S/N: %2)").arg(TEST_NAME)
							   .arg("44332211"));
	QVERIFY(obj->isOpen() == false);

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBPro_Test::extractSerialFailedWrite()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_write_data_expected_return_value = -1;
	_ftdi_read_data_expected_return_value = 0;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 2);
	QCOMPARE(_ftdi_usb_reset_called, 2);
	QCOMPARE(_ftdi_set_baudrate_called, 2);
	QCOMPARE(_ftdi_set_baudrate_called, 2);
	QCOMPARE(_ftdi_set_line_property_called, 2);
	QCOMPARE(_ftdi_setrts_called, 2);
	QCOMPARE(_ftdi_write_data_called, 2);
	QCOMPARE(_ftdi_read_data_called, 1);
	QCOMPARE(_ftdi_usb_close_called, 2);

        QCOMPARE(obj->m_name, QString(TEST_NAME));
        QCOMPARE(obj->m_serial, QString(TEST_SRNO));
        QCOMPARE(obj->m_enttecSerial, QString(TEST_SRNO));

	delete obj;
	QCOMPARE(_ftdi_init_called, 2);
	QCOMPARE(_ftdi_deinit_called, 2);
}

void EnttecDMXUSBPro_Test::extractSerialFailedRead()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_write_data_expected_return_value = 0;
	_ftdi_read_data_expected_return_value = -1;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 3);
	QCOMPARE(_ftdi_usb_reset_called, 3);
	QCOMPARE(_ftdi_set_baudrate_called, 3);
	QCOMPARE(_ftdi_set_baudrate_called, 3);
	QCOMPARE(_ftdi_set_line_property_called, 3);
	QCOMPARE(_ftdi_setrts_called, 3);
	QCOMPARE(_ftdi_write_data_called, 3);
	QCOMPARE(_ftdi_read_data_called, 2);
	QCOMPARE(_ftdi_usb_close_called, 3);

        QCOMPARE(obj->m_name, QString(TEST_NAME));
        QCOMPARE(obj->m_serial, QString(TEST_SRNO));
        QCOMPARE(obj->m_enttecSerial, QString(TEST_SRNO));

	delete obj;
	QCOMPARE(_ftdi_init_called, 3);
	QCOMPARE(_ftdi_deinit_called, 3);
}

void EnttecDMXUSBPro_Test::closeAlreadyClosed()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_write_data_expected_return_value = 0;
	_ftdi_read_data_expected_return_value = 0;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 4);
	QCOMPARE(_ftdi_usb_reset_called, 4);
	QCOMPARE(_ftdi_set_baudrate_called, 4);
	QCOMPARE(_ftdi_set_baudrate_called, 4);
	QCOMPARE(_ftdi_set_line_property_called, 4);
	QCOMPARE(_ftdi_setrts_called, 4);
	QCOMPARE(_ftdi_write_data_called, 4);
	QCOMPARE(_ftdi_read_data_called, 3);
	QCOMPARE(_ftdi_usb_close_called, 4);

	obj->close();
	QCOMPARE(_ftdi_usb_close_called, 4); // Already closed

        QCOMPARE(obj->m_name, QString(TEST_NAME));
        QCOMPARE(obj->m_serial, QString(TEST_SRNO));
        QCOMPARE(obj->m_enttecSerial, QString("44332211"));

	delete obj;
	QCOMPARE(_ftdi_init_called, 4);
	QCOMPARE(_ftdi_deinit_called, 4);
}

void EnttecDMXUSBPro_Test::open()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_write_data_expected_return_value = 0;
	_ftdi_read_data_expected_return_value = 0;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 5);
	QCOMPARE(_ftdi_usb_reset_called, 5);
	QCOMPARE(_ftdi_set_baudrate_called, 5);
	QCOMPARE(_ftdi_set_baudrate_called, 5);
	QCOMPARE(_ftdi_set_line_property_called, 5);
	QCOMPARE(_ftdi_setrts_called, 5);
	QCOMPARE(_ftdi_write_data_called, 5);
	QCOMPARE(_ftdi_read_data_called, 4);
	QCOMPARE(_ftdi_usb_close_called, 5);

        QCOMPARE(obj->m_name, QString(TEST_NAME));
        QCOMPARE(obj->m_serial, QString(TEST_SRNO));
        QCOMPARE(obj->m_enttecSerial, QString("44332211"));

	QCOMPARE(obj->open(), true);
	QCOMPARE(_ftdi_usb_open_desc_called, 6);
	QCOMPARE(_ftdi_usb_reset_called, 6);
	QCOMPARE(_ftdi_set_baudrate_called, 6);
	QCOMPARE(_ftdi_set_baudrate_called, 6);
	QCOMPARE(_ftdi_set_line_property_called, 6);
	QCOMPARE(_ftdi_setrts_called, 6);
	QCOMPARE(_ftdi_write_data_called, 5);
	QCOMPARE(_ftdi_read_data_called, 4);
	QCOMPARE(obj->isOpen(), true);

	// 2nd open "succeeds" but nothing actually happens
	QCOMPARE(obj->open(), true);
	QCOMPARE(_ftdi_usb_open_desc_called, 6);
	QCOMPARE(_ftdi_usb_reset_called, 6);
	QCOMPARE(_ftdi_set_baudrate_called, 6);
	QCOMPARE(_ftdi_set_baudrate_called, 6);
	QCOMPARE(_ftdi_set_line_property_called, 6);
	QCOMPARE(_ftdi_setrts_called, 6);
	QCOMPARE(_ftdi_write_data_called, 5);
	QCOMPARE(_ftdi_read_data_called, 4);
	QCOMPARE(obj->isOpen(), true);

	delete obj;
	QCOMPARE(_ftdi_init_called, 5);
	QCOMPARE(_ftdi_deinit_called, 5);
}

void EnttecDMXUSBPro_Test::sendDMX()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
	_ftdi_write_data_expected_return_value = 0;
	_ftdi_read_data_expected_return_value = 0;
	EnttecDMXUSBPro* obj = new EnttecDMXUSBPro(this, TEST_NAME, TEST_SRNO);

	_ftdi_write_data_expected_size = 512 + 6;
	_ftdi_write_data_expected_return_value = 0;

	QByteArray arr(512, uchar(128));
	QCOMPARE(obj->sendDMX(arr), false);
	QCOMPARE(obj->isOpen(), false);

	QCOMPARE(obj->open(), true);
	QCOMPARE(obj->sendDMX(arr), true);
	QCOMPARE(obj->sendDMX(arr), true);

	_ftdi_write_data_expected_return_value = -1;
	QCOMPARE(obj->sendDMX(arr), false);

	delete obj;
	QCOMPARE(_ftdi_init_called, 6);
	QCOMPARE(_ftdi_deinit_called, 6);
}

QTEST_MAIN(EnttecDMXUSBPro_Test)
