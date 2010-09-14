/*
  Q Light Controller
  enttecdmxusbwidget.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef ENTTECDMXUSBWIDGET_H
#define ENTTECDMXUSBWIDGET_H

/**
 * This is the base interface class for ENTTEC USB DMX [Pro|Open] widgets.
 */
class EnttecDMXUSBWidget
{
public:
	/** Remove this and suffer the oh-so-lovely crashing consequences! */
	virtual ~EnttecDMXUSBWidget() {}

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	/**
	 * Open widget for further operations, such as serial() and sendDMX()
	 *
	 * @return true if widget was opened successfully (or was already open)
	 */
	virtual bool open() = 0;

	/**
	 * Close widget, preventing any further operations
	 *
	 * @param true if widget was closed successfully (or was already closed)
	 */
	virtual bool close() = 0;

	/**
	 * Check, whether widget has been opened
	 *
	 * @return true if widget is open, otherwise false
	 */
	virtual bool isOpen() = 0;

	/********************************************************************
	 * Serial & name
	 ********************************************************************/
public:
	/**
	 * Get the widget serial number as a string. The same serial should be
	 * printed on the actual physical device. Can be used to uniquely
	 * identify widgets.
	 *
	 * @return widget's serial number in string form
	 */
	virtual QString serial() const = 0;

	/**
	 * Get the device's friendly name, which is not unique, but only
	 * tells the product name (e.g. "DMX USB PRO")
	 *
	 * @return widget's name
	 */
	virtual QString name() const = 0;

	/**
	 * Get the widget's unique name
	 *
	 * @return widget's unique name as: "<name> (S/N: <serial>)"
	 */
	virtual QString uniqueName() const = 0;

	/********************************************************************
	 * DMX operations
	 ********************************************************************/
public:
	/**
	 * Send the given universe-ful of DMX data to widget. The universe must
	 * be at least 25 bytes but no more than 513 bytes long.
	 *
	 * @param universe The DMX universe to send
	 * @return true if the values were sent successfully, otherwise false
	 */
	virtual bool sendDMX(const QByteArray& universe) = 0;
};

#endif
