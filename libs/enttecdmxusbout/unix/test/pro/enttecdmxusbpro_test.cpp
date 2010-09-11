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

#define TEST_NAME "Foobar"
#define TEST_SRNO "1234567890"

static int _ftdi_init_called = 0;
static int _ftdi_deinit_called = 0;

static int _ftdi_usb_open_desc_called = 0;
static int _ftdi_usb_close_called = 0;
static int _ftdi_usb_reset_called = 0;
static int _ftdi_set_line_property_called = 0;
static int _ftdi_set_baudrate_called = 0;
static int _ftdi_setrts_called = 0;

static int _ftdi_write_data_called = 0;
static int _ftdi_write_data_expected_size = 0;
static int _ftdi_write_data_expected_return_value = 0;
static int _ftdi_read_data_called = 0;
static int _ftdi_read_data_expected_size = 0;
static int _ftdi_read_data_expected_return_value = 0;

/****************************************************************************
 * FTDI mock functions
 ****************************************************************************/

int ftdi_init(struct ftdi_context* ctx)
{
        Q_ASSERT(ctx != NULL);
        _ftdi_init_called++;
        return 0;
} 

void ftdi_deinit(struct ftdi_context* ctx)
{
        Q_ASSERT(ctx != NULL);
        _ftdi_deinit_called++;
}

int ftdi_usb_open_desc(struct ftdi_context* ctx, int vendor, int product,
                        const char* description, const char* serial)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(vendor == 0x0403);
        Q_ASSERT(product == 0x6001);
        Q_ASSERT(QString(description) == QString(TEST_NAME));
        Q_ASSERT(QString(serial) == QString(TEST_SRNO));

        ctx->usb_dev = reinterpret_cast<usb_dev_handle*> (0xDEADBEEF);

        _ftdi_usb_open_desc_called++;
        return 0;
}

int ftdi_usb_close(struct ftdi_context* ctx)
{
        Q_ASSERT(ctx != NULL);

        ctx->usb_dev = NULL;

        _ftdi_usb_close_called++;
        return 0;
}

int ftdi_usb_reset(struct ftdi_context* ctx)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
        _ftdi_usb_reset_called++;
        return 0;
}

int ftdi_set_line_property(struct ftdi_context* ctx,
                           enum ftdi_bits_type bits,
                           enum ftdi_stopbits_type stop,
                           enum ftdi_parity_type parity)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(bits == BITS_8);
        Q_ASSERT(stop == STOP_BIT_2);
        Q_ASSERT(parity == NONE);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
        _ftdi_set_line_property_called++;
        return 0;
}

int ftdi_set_baudrate(struct ftdi_context* ctx, int baudrate)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(baudrate == 250000);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
        _ftdi_set_baudrate_called++;
        return 0;
}

int ftdi_setrts(struct ftdi_context* ctx, int rts)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(rts == 0);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
        _ftdi_setrts_called++;
        return 0;
}

int ftdi_write_data(struct ftdi_context* ctx, unsigned char* buf, int size)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(buf != NULL);
        Q_ASSERT(size == _ftdi_write_data_expected_size);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));

	if (size <= 5)
	{
		Q_ASSERT(buf[0] == 0x7e);
		Q_ASSERT(buf[1] == 0x0a);
		Q_ASSERT(buf[2] == 0x00);
		Q_ASSERT(buf[3] == 0x00);
		Q_ASSERT(buf[4] == 0xe7);
	}
	else
	{
		Q_ASSERT(size == 518);
		Q_ASSERT(buf[0] == 0x7e);
		Q_ASSERT(buf[1] == 0x06);
		Q_ASSERT(buf[3] == int((513 >> 8) & 0xff));
		Q_ASSERT(buf[2] == int(513 & 0xff));
		Q_ASSERT(buf[4] == 0x00);
		Q_ASSERT(buf[size - 1] == 0xe7);
	}

        _ftdi_write_data_called++;
        return _ftdi_write_data_expected_return_value;
}

int ftdi_read_data(struct ftdi_context* ctx, unsigned char* reply, int size)
{
        Q_ASSERT(ctx != NULL);
        Q_ASSERT(reply != NULL);
        Q_ASSERT(size == _ftdi_read_data_expected_size);
        Q_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));

	reply[0] = 0x7e;
	reply[1] = 0x0a;
	reply[2] = 0x04;
	reply[3] = 0x00;
	reply[4] = 0x11;
	reply[5] = 0x22;
	reply[6] = 0x33;
	reply[7] = 0x44;
	reply[8] = 0xe7;

        _ftdi_read_data_called++;
        return _ftdi_read_data_expected_return_value;
}

/****************************************************************************
 * Unit test code
 ****************************************************************************/

void EnttecDMXUSBPro_Test::construction()
{
	_ftdi_write_data_expected_size = 5;
	_ftdi_read_data_expected_size = 9;
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
