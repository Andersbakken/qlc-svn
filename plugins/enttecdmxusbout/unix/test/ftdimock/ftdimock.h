/*
  Q Light Controller - Unit tests
  ftdimock.h

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

#ifndef FTDIMOCK_H
#define FTDIMOCK_H

void ftdimock_reset_variables();

extern int _ftdi_init_called;
extern int _ftdi_deinit_called;
extern struct usb_dev_handle* _ftdi_usb_dev_expected_value;

extern int _ftdi_usb_find_all_called;
extern struct ftdi_device_list* _ftdi_usb_find_all_expected_devlist;
extern int _ftdi_usb_find_all_expected_return_value;

extern int _ftdi_usb_get_strings_called;
extern struct usb_device* _ftdi_usb_get_strings_expected_device;
extern int _ftdi_usb_get_strings_expected_return_value;
extern const char* _ftdi_usb_get_strings_expected_manufacturer;
extern const char* _ftdi_usb_get_strings_expected_description;
extern const char* _ftdi_usb_get_strings_expected_serial;

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

extern int _ftdi_read_data_called;
extern int _ftdi_read_data_expected_return_value;
extern const char* _ftdi_read_data_expected_reply;
extern int _ftdi_read_data_expected_size;

extern int _ftdi_usb_purge_buffers_called;
extern int _ftdi_usb_purge_buffers_expected_return_value;

extern int _ftdi_set_line_property2_called;
extern int _ftdi_set_line_property2_expected_return_value;

extern QString _ftdi_usb_open_desc_expected_description;
extern QString _ftdi_usb_open_desc_expected_serial;

#endif
