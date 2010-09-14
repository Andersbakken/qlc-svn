/*
  Q Light Controller - Unit tests
  enttecdmxusbopen_test.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QTest>
#include "enttecdmxusbopen_test.h"

#define protected public
#include "enttecdmxusbopen.h"
#undef protected

#define TEST_NAME "Foobar"
#define TEST_SRNO "1234567890"

// Redefined Q_ASSERT because it is not present in release builds
#define UT_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())

static int _ftdi_init_called = 0;
static int _ftdi_deinit_called = 0;

static int _ftdi_usb_open_desc_called = 0;
static int _ftdi_usb_close_called = 0;
static int _ftdi_usb_reset_called = 0;
static int _ftdi_set_line_property_called = 0;
static int _ftdi_set_baudrate_called = 0;
static int _ftdi_setrts_called = 0;

static int _ftdi_usb_purge_buffers_called = 0;
static int _ftdi_set_line_property2_called = 0;
static int _ftdi_write_data_called = 0;

/****************************************************************************
 * FTDI mock functions
 ****************************************************************************/

int ftdi_init(struct ftdi_context* ctx)
{
	UT_ASSERT(ctx != NULL);
	_ftdi_init_called++;
	return 0;
}

void ftdi_deinit(struct ftdi_context* ctx)
{
	UT_ASSERT(ctx != NULL);
	_ftdi_deinit_called++;
}

int ftdi_usb_open_desc(struct ftdi_context* ctx, int vendor, int product,
			const char* description, const char* serial)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(vendor == 0x0403);
	UT_ASSERT(product == 0x6001);
	UT_ASSERT(QString(description) == QString(TEST_NAME));
	UT_ASSERT(QString(serial) == QString(TEST_SRNO));

	ctx->usb_dev = reinterpret_cast<usb_dev_handle*> (0xDEADBEEF);

	_ftdi_usb_open_desc_called++;
	return 0;
}

int ftdi_usb_close(struct ftdi_context* ctx)
{
	UT_ASSERT(ctx != NULL);

	ctx->usb_dev = NULL;

	_ftdi_usb_close_called++;
	return 0;
}

int ftdi_usb_reset(struct ftdi_context* ctx)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
	_ftdi_usb_reset_called++;
	return 0;
}

int ftdi_set_line_property(struct ftdi_context* ctx,
			   enum ftdi_bits_type bits,
			   enum ftdi_stopbits_type stop,
			   enum ftdi_parity_type parity)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(bits == BITS_8);
	UT_ASSERT(stop == STOP_BIT_2);
	UT_ASSERT(parity == NONE);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
	_ftdi_set_line_property_called++;
	return 0;
}

int ftdi_set_baudrate(struct ftdi_context* ctx, int baudrate)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(baudrate == 250000);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
	_ftdi_set_baudrate_called++;
	return 0;
}

int ftdi_setrts(struct ftdi_context* ctx, int rts)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(rts == 0);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
	_ftdi_setrts_called++;
	return 0;
}

int ftdi_usb_purge_buffers(struct ftdi_context* ctx)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));
	_ftdi_usb_purge_buffers_called++;
	return 0;
}

int ftdi_set_line_property2(struct ftdi_context* ctx,
			    enum ftdi_bits_type bits,
			    enum ftdi_stopbits_type stop,
			    enum ftdi_parity_type parity,
			    enum ftdi_break_type brk)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(bits == BITS_8);
	UT_ASSERT(stop == STOP_BIT_2);
	UT_ASSERT(parity == NONE);
	UT_ASSERT(brk == BREAK_ON || brk == BREAK_OFF);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));

	_ftdi_set_line_property2_called++;
	return 0;
}

int ftdi_write_data(struct ftdi_context* ctx, unsigned char* buf, int size)
{
	UT_ASSERT(ctx != NULL);
	UT_ASSERT(buf != NULL);
	UT_ASSERT(size == 513);
	UT_ASSERT(ctx->usb_dev == reinterpret_cast<usb_dev_handle*> (0xDEADBEEF));

	_ftdi_write_data_called++;
	return 0;
}

/****************************************************************************
 * Unit test code
 ****************************************************************************/

void EnttecDMXUSBOpen_Test::construction()
{
	EnttecDMXUSBOpen obj(this, TEST_NAME, TEST_SRNO);
	QCOMPARE(obj.m_name, QString(TEST_NAME));
	QCOMPARE(obj.m_serial, QString(TEST_SRNO));
	QCOMPARE(obj.m_running, false);
	QCOMPARE(obj.m_universe, QByteArray(513, 0));

	QCOMPARE(obj.name(), QString(TEST_NAME));
	QCOMPARE(obj.serial(), QString(TEST_SRNO));
	QCOMPARE(obj.uniqueName(), QString("%1 (S/N: %2)").arg(TEST_NAME)
							  .arg(TEST_SRNO));

	QVERIFY(obj.isOpen() == false);
}

void EnttecDMXUSBOpen_Test::sendDMX()
{
	EnttecDMXUSBOpen obj(this, TEST_NAME, TEST_SRNO);
	QCOMPARE((char)obj.m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj.m_universe[1], (char) 0);
	QCOMPARE(obj.m_universe.size(), 513); // 512 + start code

	obj.sendDMX(QByteArray(15, 147));
	QCOMPARE((char)obj.m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj.m_universe[1], (char) 147);
	QCOMPARE((char)obj.m_universe[15], (char) 147);
	QCOMPARE((char)obj.m_universe[16], (char) 0);
	QCOMPARE(obj.m_universe.size(), 513); // 512 + start code

	obj.sendDMX(QByteArray(512, UCHAR_MAX));
	QCOMPARE((char)obj.m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj.m_universe[1], (char) UCHAR_MAX);
	QCOMPARE((char)obj.m_universe[15], (char) UCHAR_MAX);
	QCOMPARE((char)obj.m_universe[16], (char) UCHAR_MAX);
	QCOMPARE((char)obj.m_universe[511], (char) UCHAR_MAX);
	QCOMPARE(obj.m_universe.size(), 513); // 512 + start code
}

void EnttecDMXUSBOpen_Test::run()
{
	EnttecDMXUSBOpen obj(this, TEST_NAME, TEST_SRNO);
	obj.open();
	QVERIFY(obj.isOpen() == true);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
	QTest::qWait(200);
	obj.close();
	QVERIFY(obj.isOpen() == false);

	QCOMPARE(_ftdi_usb_purge_buffers_called, 1);

	QVERIFY(_ftdi_set_line_property2_called >= 15);
	QVERIFY(_ftdi_write_data_called >= 7);

	QVERIFY(obj.isRunning() == false);
	QVERIFY(obj.m_running == false);
}

QTEST_MAIN(EnttecDMXUSBOpen_Test)
