/*
  Q Light Controller - Unit tests
  ftdimock.cpp

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

#include <QtCore>
#include <ftdi.h>

// Redefined Q_ASSERT because it is not present in release builds
#define UT_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())

int _ftdi_init_called = 0;
int _ftdi_deinit_called = 0;
struct usb_dev_handle* _ftdi_usb_dev_expected_value = (struct usb_dev_handle*) 0xDEADBEEF;

int _ftdi_usb_open_desc_called = 0;
int _ftdi_usb_open_desc_expected_return_value = 0;

int _ftdi_usb_reset_called = 0;
int _ftdi_usb_reset_expected_return_value = 0;

int _ftdi_set_line_property_called = 0;
int _ftdi_set_line_property_expected_return_value = 0;

int _ftdi_set_baudrate_called = 0;
int _ftdi_set_baudrate_expected_return_value = 0;

int _ftdi_setrts_called = 0;
int _ftdi_setrts_expected_return_value = 0;

int _ftdi_set_line_property2_called = 0;
int _ftdi_set_line_property2_expected_return_value = 0;

int _ftdi_get_error_string_called = 0;

int _ftdi_usb_purge_buffers_called = 0;
int _ftdi_usb_purge_buffers_expected_return_value = 0;

int _ftdi_usb_close_called = 0;
int _ftdi_usb_close_expected_return_value = 0;

int _ftdi_write_data_called = 0;
int _ftdi_write_data_expected_size = 0;
int _ftdi_write_data_expected_return_value = 0;

int _ftdi_read_data_called = 0;
int _ftdi_read_data_expected_size = 0;
int _ftdi_read_data_expected_return_value = 0;
const char* _ftdi_read_data_expected_reply = NULL;

int _ftdi_usb_get_strings_called = 0;

QString _ftdi_usb_open_desc_expected_description;
QString _ftdi_usb_open_desc_expected_serial;

/****************************************************************************
 * FTDI mock functions
 ****************************************************************************/

int ftdi_init(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);
    _ftdi_init_called++;
    ctx->error_str = (char*) malloc(sizeof(char) * 15);
    strcpy(ctx->error_str, "Expected error");
    return 0;
}

void ftdi_deinit(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);
    free(ctx->error_str);
    ctx->error_str = NULL;
    _ftdi_deinit_called++;
}

int ftdi_usb_find_all(struct ftdi_context* ctx, struct ftdi_device_list **devlist,
                      int vendor, int product)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(devlist != NULL);
    UT_ASSERT(vendor == 0x0403);
    UT_ASSERT(product == 0x6001);
    return 1;
}

int ftdi_usb_open_desc(struct ftdi_context* ctx, int vendor, int product,
                       const char* description, const char* serial)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(vendor == 0x0403);
    UT_ASSERT(product == 0x6001);
    UT_ASSERT(QString(description) == _ftdi_usb_open_desc_expected_description);
    UT_ASSERT(QString(serial) == _ftdi_usb_open_desc_expected_serial);

    ctx->usb_dev = _ftdi_usb_dev_expected_value;

    _ftdi_usb_open_desc_called++;

    return _ftdi_usb_open_desc_expected_return_value;
}

int ftdi_usb_close(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);

    ctx->usb_dev = NULL;

    _ftdi_usb_close_called++;
    return _ftdi_usb_close_expected_return_value;
}

int ftdi_usb_reset(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_usb_reset_called++;

    return _ftdi_usb_reset_expected_return_value;
}

int ftdi_usb_purge_buffers(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_usb_purge_buffers_called++;

    return _ftdi_usb_purge_buffers_expected_return_value;
}

char* ftdi_get_error_string(struct ftdi_context* ctx)
{
    UT_ASSERT(ctx != NULL);
    _ftdi_get_error_string_called++;
    return ctx->error_str;
}

int ftdi_usb_get_strings(struct ftdi_context* ctx, struct usb_device* dev,
                         char* manufacturer, int mnf_len,
                         char* description, int desc_len,
                         char* serial, int serial_len)
{
    UT_ASSERT(ctx != NULL);
    Q_UNUSED(dev);
    Q_UNUSED(manufacturer);
    Q_UNUSED(mnf_len);
    Q_UNUSED(description);
    Q_UNUSED(desc_len);
    Q_UNUSED(serial);
    Q_UNUSED(serial_len);
    _ftdi_usb_get_strings_called++;
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
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_set_line_property_called++;

    return _ftdi_set_line_property_expected_return_value;
}

int ftdi_set_line_property2(struct ftdi_context* ctx, enum ftdi_bits_type bits,
                            enum ftdi_stopbits_type stop, enum ftdi_parity_type parity,
                            enum ftdi_break_type break_type)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(bits == BITS_8);
    UT_ASSERT(stop == STOP_BIT_2);
    UT_ASSERT(parity == NONE);
    UT_ASSERT(break_type == BREAK_ON || break_type == BREAK_OFF);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_set_line_property2_called++;

    if ((_ftdi_set_line_property2_expected_return_value == -1 && break_type == BREAK_ON)
            ||
            (_ftdi_set_line_property2_expected_return_value == -2 && break_type == BREAK_OFF))
    {
        return _ftdi_set_line_property2_expected_return_value;
    }
    else
    {
        return 0;
    }
}

int ftdi_set_baudrate(struct ftdi_context* ctx, int baudrate)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(baudrate == 250000);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_set_baudrate_called++;

    return _ftdi_set_baudrate_expected_return_value;
}

int ftdi_setrts(struct ftdi_context* ctx, int rts)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(rts == 0);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);
    _ftdi_setrts_called++;

    return _ftdi_setrts_expected_return_value;
}

int ftdi_write_data(struct ftdi_context* ctx, unsigned char* buf, int size)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(buf != NULL);
    UT_ASSERT(size == _ftdi_write_data_expected_size);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);

    if (size <= 5)
    {
        // Enttec pro serial query packet
        UT_ASSERT(buf[0] == 0x7e);
        UT_ASSERT(buf[1] == 0x0a);
        UT_ASSERT(buf[2] == 0x00);
        UT_ASSERT(buf[3] == 0x00);
        UT_ASSERT(buf[4] == 0xe7);
    }
    else if (size > 513)
    {
        // Enttec pro write DMX packet
        UT_ASSERT(size == 518);
        UT_ASSERT(buf[0] == 0x7e);
        UT_ASSERT(buf[1] == 0x06);
        UT_ASSERT(buf[3] == int((513 >> 8) & 0xff));
        UT_ASSERT(buf[2] == int(513 & 0xff));
        UT_ASSERT(buf[4] == 0x00);
        UT_ASSERT(buf[size - 1] == 0xe7);
    }
    else
    {
        // Enttec open DMX writer thread packet
        UT_ASSERT(size == 513);
        UT_ASSERT(buf[0] == 0);
        UT_ASSERT(buf[1] == UCHAR_MAX);
        UT_ASSERT(buf[42] == UCHAR_MAX);
        UT_ASSERT(buf[size - 1] == UCHAR_MAX);
    }

    _ftdi_write_data_called++;
    return _ftdi_write_data_expected_return_value;
}

int ftdi_read_data(struct ftdi_context* ctx, unsigned char* reply, int size)
{
    UT_ASSERT(ctx != NULL);
    UT_ASSERT(reply != NULL);
    UT_ASSERT(size == _ftdi_read_data_expected_size);
    UT_ASSERT(ctx->usb_dev == _ftdi_usb_dev_expected_value);

    memcpy(reply, _ftdi_read_data_expected_reply, _ftdi_read_data_expected_size);

    _ftdi_read_data_called++;
    return _ftdi_read_data_expected_return_value;
}
