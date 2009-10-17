/*
  Q Light Controller
  enttecdmxusbopen.h

  Copyright (C) Heikki Junnila
		Christopher Staite

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

#ifndef ENTTECDMXUSBOPEN_H
#define ENTTECDMXUSBOPEN_H

#include <QByteArray>
#include <QThread>

#include "enttecdmxusbwidget.h"

#ifdef WIN32
#	include "windows.h"
#	include "ftd2xx-win32.h"
#else
#	include "ftd2xx.h"
#endif

class EnttecDMXUSBOpen : public QThread, public EnttecDMXUSBWidget
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/**
	 * Construct a new DMXUSBOpen object with the given parent and
	 * FTDI device information. Neither can be null.
	 *
	 * @param parent The owner of this object
	 * @param info FTDI device information
	 */
	EnttecDMXUSBOpen(QObject* parent,
			 const FT_DEVICE_LIST_INFO_NODE& info);

	/**
	 * Destructor
	 */
	virtual ~EnttecDMXUSBOpen();

protected:
	/** FTDI device information */
	FT_DEVICE_LIST_INFO_NODE m_info;

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	/**
	 * Open widget for further operations, such as serial() and sendDMX()
	 *
	 * @return true if widget was opened successfully (or was already open)
	 */
        bool open();

	/**
	 * Close widget, preventing any further operations
	 *
	 * @param true if widget was closed successfully (or was already closed)
	 */
        bool close();

	/**
	 * Check, whether widget has been opened
	 *
	 * @return true if widget is open, otherwise false
	 */
        bool isOpen() const;

	/**
	 * Initialize the widget port for DMX output
	 *
	 * @return true if successful, otherwise false
	 */
	bool initializePort();

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
        QString serial() const;

	/**
	 * Get the device's friendly name, which is not unique, but only
	 * tells the product name (e.g. "Open DMX USB")
	 *
	 * @return widget's name
	 */
	QString name() const;

	/**
	 * Get the widget's unique name
	 *
	 * @return widget's unique name as: "<name> (S/N: <serial>)"
	 */
	QString uniqueName() const;

	/**
	 * Set the serial number for the widget. Open DMX USB doesn't have
	 * API for getting the serial, so it has to be given to the device.
	 *
	 * Serial is used only by users to discern which widget is which.
	 *
	 * @param serial The serial to set
	 */
	void setSerial(const QString& serial) { m_serial = serial; }

protected:
	QString m_serial;

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
	bool sendDMX(const QByteArray& universe);

	/********************************************************************
	 * Thread
	 ********************************************************************/
protected:
	/**
	 * Stop the writer thread
	 */
	void stop();

	/**
	 * DMX writer thread
	 */
	void run();

protected:
	bool m_running;
	QByteArray m_universe;
};

#endif
