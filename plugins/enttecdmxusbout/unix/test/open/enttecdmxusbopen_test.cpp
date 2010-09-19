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
#include <QDebug>
#include "enttecdmxusbopen_test.h"

#define protected public
#include "enttecdmxusbopen.h"
#undef protected

// Redefined Q_ASSERT because it is not present in release builds
#define UT_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())

#define TEST_NAME "Foobar"
#define TEST_SRNO "1234567890"

extern int _ftdi_init_called;
extern int _ftdi_deinit_called;
extern struct usb_dev_handle* _ftdi_usb_dev_expected_value;

extern int _ftdi_usb_open_desc_called;
extern int _ftdi_usb_open_desc_expected_return_value;

extern int _ftdi_usb_close_called;
extern int _ftdi_usb_close_expected_return_value;

extern int _ftdi_usb_reset_called;
extern int _ftdi_usb_reset_expected_return_value;

extern int _ftdi_set_line_property_called;
extern int _ftdi_set_line_property_expected_return_value;

extern int _ftdi_set_baudrate_called;
extern int _ftdi_set_baudrate_expected_return_value;

extern int _ftdi_setrts_called;
extern int _ftdi_setrts_expected_return_value;

extern int _ftdi_usb_purge_buffers_called;
extern int _ftdi_usb_purge_buffers_expected_return_value;

extern int _ftdi_write_data_called;
extern int _ftdi_write_data_expected_size;
extern int _ftdi_write_data_expected_return_value;

extern int _ftdi_usb_purge_buffers_called;
extern int _ftdi_usb_purge_buffers_expected_return_value;

extern int _ftdi_set_line_property2_called;
extern int _ftdi_set_line_property2_expected_return_value;

extern QString _ftdi_usb_open_desc_expected_description;
extern QString _ftdi_usb_open_desc_expected_serial;

/****************************************************************************
 * Unit test code
 ****************************************************************************/

void EnttecDMXUSBOpen_Test::resetCounters()
{
	_ftdi_usb_dev_expected_value = (struct usb_dev_handle*) 0xDEADBEEF;

	_ftdi_init_called = 0;
	_ftdi_deinit_called = 0;

	_ftdi_usb_open_desc_called = 0;
	_ftdi_usb_open_desc_expected_return_value = 0;

	_ftdi_usb_close_called = 0;
	_ftdi_usb_close_expected_return_value = 0;

	_ftdi_usb_reset_called = 0;
	_ftdi_usb_reset_expected_return_value = 0;

	_ftdi_set_line_property_called = 0;
	_ftdi_set_line_property_expected_return_value = 0;

	_ftdi_set_baudrate_called = 0;
	_ftdi_set_baudrate_expected_return_value = 0;

	_ftdi_setrts_called = 0;
	_ftdi_setrts_expected_return_value = 0;

	_ftdi_write_data_called = 0;
	_ftdi_write_data_expected_size = 5;
	_ftdi_write_data_expected_return_value = 0;

	_ftdi_usb_purge_buffers_called = 0;
	_ftdi_usb_purge_buffers_expected_return_value = 0;

	_ftdi_set_line_property2_called = 0;
	_ftdi_set_line_property2_expected_return_value = 0;

	_ftdi_usb_open_desc_expected_description = TEST_NAME;
	_ftdi_usb_open_desc_expected_serial = TEST_SRNO;
}

void EnttecDMXUSBOpen_Test::construction()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(obj->m_name, QString(TEST_NAME));
	QCOMPARE(obj->m_serial, QString(TEST_SRNO));
	QCOMPARE(obj->m_running, false);
	QCOMPARE(obj->m_universe, QByteArray(513, 0));

	QCOMPARE(obj->name(), QString(TEST_NAME));
	QCOMPARE(obj->serial(), QString(TEST_SRNO));
	QCOMPARE(obj->uniqueName(), QString("%1 (S/N: %2)").arg(TEST_NAME)
							  .arg(TEST_SRNO));

	QVERIFY(obj->isOpen() == false);

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::closeFail()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);
        
	QVERIFY(obj->open() == true);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
        QCOMPARE(_ftdi_usb_close_called, 0);

	_ftdi_usb_close_expected_return_value = -1;
	QVERIFY(obj->close() == false);
	QCOMPARE(_ftdi_usb_close_called, 1);

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::openFail()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	_ftdi_usb_open_desc_expected_return_value = -1;
	QVERIFY(obj->open() == false);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 0);
	QCOMPARE(_ftdi_set_line_property_called, 0);
	QCOMPARE(_ftdi_set_baudrate_called, 0);
	QCOMPARE(_ftdi_setrts_called, 0);
	QVERIFY(obj->close() == true);
	resetCounters();

	_ftdi_usb_reset_expected_return_value = -1;
	QVERIFY(obj->open() == false);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 0);
	QCOMPARE(_ftdi_set_baudrate_called, 0);
	QCOMPARE(_ftdi_setrts_called, 0);
	resetCounters();

	_ftdi_set_line_property_expected_return_value = -1;
	QVERIFY(obj->open() == false);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 0);
	QCOMPARE(_ftdi_setrts_called, 0);
	resetCounters();

	_ftdi_set_baudrate_expected_return_value = -1;
	QVERIFY(obj->open() == false);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_setrts_called, 0);
	resetCounters();

	_ftdi_setrts_expected_return_value = -1;
	QVERIFY(obj->open() == false);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
	resetCounters();

}

void EnttecDMXUSBOpen_Test::openClose()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	QCOMPARE(_ftdi_usb_open_desc_called, 0);
	QCOMPARE(_ftdi_usb_reset_called, 0);
	QCOMPARE(_ftdi_set_line_property_called, 0);
	QCOMPARE(_ftdi_set_baudrate_called, 0);
	QCOMPARE(_ftdi_setrts_called, 0);
	QCOMPARE(_ftdi_usb_close_called, 0);

	QVERIFY(obj->isOpen() == false);
	QVERIFY(obj->open() == true);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
	QCOMPARE(_ftdi_usb_close_called, 0);

	obj->close();
	QCOMPARE(_ftdi_usb_close_called, 1);
	obj->close();
	QCOMPARE(_ftdi_usb_close_called, 1); // Counter value doesn't grow
	obj->close();
	QCOMPARE(_ftdi_usb_close_called, 1); // Counter value doesn't grow

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::sendDMX()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	QCOMPARE((char)obj->m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj->m_universe[1], (char) 0);
	QCOMPARE(obj->m_universe.size(), 513); // 512 + start code

	obj->sendDMX(QByteArray(15, 147));
	QCOMPARE((char)obj->m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj->m_universe[1], (char) 147);
	QCOMPARE((char)obj->m_universe[15], (char) 147);
	QCOMPARE((char)obj->m_universe[16], (char) 0);
	QCOMPARE(obj->m_universe.size(), 513); // 512 + start code

	obj->sendDMX(QByteArray(512, UCHAR_MAX));
	QCOMPARE((char)obj->m_universe[0], (char) 0); // Start code always 0
	QCOMPARE((char)obj->m_universe[1], (char) UCHAR_MAX);
	QCOMPARE((char)obj->m_universe[15], (char) UCHAR_MAX);
	QCOMPARE((char)obj->m_universe[16], (char) UCHAR_MAX);
	QCOMPARE((char)obj->m_universe[511], (char) UCHAR_MAX);
	QCOMPARE(obj->m_universe.size(), 513); // 512 + start code

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::runFail()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	// Simulate sudden closing of the device
	QVERIFY(obj->open() == true);
	_ftdi_usb_dev_expected_value = NULL;
	obj->m_context.usb_dev = NULL;
	QVERIFY(obj->isOpen() == false);
	QTest::qWait(100);
	obj->close();
	QVERIFY(_ftdi_write_data_called < 2);

	resetCounters();

	// Ftdimock knows to return false on BREAK_ON with -2
	_ftdi_set_line_property2_expected_return_value = -1;
	obj->sendDMX(QByteArray(512, UCHAR_MAX));
	QVERIFY(obj->open() == true);
	QVERIFY(obj->isOpen() == true);
	QTest::qWait(100);
	obj->close();
	QVERIFY(obj->isOpen() == false);
	QCOMPARE(_ftdi_write_data_called, 0); // Write is never reached
	QVERIFY(_ftdi_set_line_property2_called >= 4);

	resetCounters();

	// Ftdimock knows to return false on BREAK_OFF with -2
	_ftdi_set_line_property2_expected_return_value = -2;
	QVERIFY(obj->open() == true);
	QVERIFY(obj->isOpen() == true);
	QTest::qWait(100);
	obj->close();
	QVERIFY(obj->isOpen() == false);
	QCOMPARE(_ftdi_write_data_called, 0); // Write is never reached
	QVERIFY(_ftdi_set_line_property2_called >= 4);

	resetCounters();

	// Simulate failing write
	_ftdi_write_data_expected_return_value = -1;
	_ftdi_write_data_expected_size = 513;
	obj->sendDMX(QByteArray(512, UCHAR_MAX));
	QVERIFY(obj->open() == true);
	QVERIFY(obj->isOpen() == true);
	QTest::qWait(100);
	obj->close();
	QVERIFY(obj->isOpen() == false);
	QVERIFY(_ftdi_write_data_called >= 4);
	QVERIFY(_ftdi_set_line_property2_called >= 4);

	resetCounters();

	delete obj;
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::run()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	_ftdi_write_data_expected_size = 513;
	obj->sendDMX(QByteArray(512, UCHAR_MAX));
	QVERIFY(obj->open() == true);
	QVERIFY(obj->isOpen() == true);
	QCOMPARE(_ftdi_usb_open_desc_called, 1);
	QCOMPARE(_ftdi_usb_reset_called, 1);
	QCOMPARE(_ftdi_set_line_property_called, 1);
	QCOMPARE(_ftdi_set_baudrate_called, 1);
	QCOMPARE(_ftdi_setrts_called, 1);
	QTest::qWait(200);
	obj->close();
	QVERIFY(obj->isOpen() == false);

	QCOMPARE(_ftdi_usb_purge_buffers_called, 1);

	QVERIFY(_ftdi_set_line_property2_called >= 15);
	QVERIFY(_ftdi_write_data_called >= 7);

	QVERIFY(obj->isRunning() == false);
	QVERIFY(obj->m_running == false);

	delete obj;
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

void EnttecDMXUSBOpen_Test::deleteWhileRunning()
{
	resetCounters();

	EnttecDMXUSBOpen* obj = new EnttecDMXUSBOpen(this, TEST_NAME, TEST_SRNO);

	_ftdi_write_data_expected_size = 513;
	obj->sendDMX(QByteArray(512, UCHAR_MAX));
	QVERIFY(obj->open() == true);
	QVERIFY(obj->isOpen() == true);
	QTest::qWait(100);

	delete obj;
	QCOMPARE(_ftdi_usb_close_called, 1);
	QCOMPARE(_ftdi_init_called, 1);
	QCOMPARE(_ftdi_deinit_called, 1);
}

QTEST_MAIN(EnttecDMXUSBOpen_Test)
