/*
  Q Light Controller
  ewing.h

  Copyright (c) Heikki Junnila

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

#ifndef EWING_H
#define EWING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>

#include "qlctypes.h"

#define EWING_INVALID_CHANNEL -1

/****************************************************************************
 * Header data common to all wings
 ****************************************************************************/

#define EWING_BYTE_HEADER   0 /* 4 bytes */
#define EWING_HEADER_SIZE   4
#define EWING_HEADER_OUTPUT "WODD"
#define EWING_HEADER_INPUT "WIDD"

/****************************************************************************
 * Status data common to all wings
 ****************************************************************************/

#define EWING_BYTE_FIRMWARE   4 /* Firmware version, 8bit value (0-255) */

#define EWING_BYTE_FLAGS      5 /* Wing flags */
#define EWING_FLAGS_MASK_PGUP 1 << 7
#define EWING_FLAGS_MASK_PGDN 1 << 6
#define EWING_FLAGS_MASK_BACK 1 << 5 /* Playback Wing only */
#define EWING_FLAGS_MASK_GO   1 << 4 /* Playback Wing only */
#define EWING_FLAGS_MASK_TYPE 0x3

/****************************************************************************
 * EWing
 ****************************************************************************/

class EWing : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/** The UDP port of ENTTEC wing devices */
	static const int UDPPort;

	/** There are currently three types of wings from ENTTEC. */
	enum Type
	{
		Unknown  = 0,
		Playback = 1,
		Shortcut = 2,
		Program  = 3
	};

	/**
	 * Construct a new EWing object. Since EWing contains pure virtual
	 * functions, the EWing class cannot be instantiated without
	 * inheritance.
	 *
	 * @param parent The parent object that owns the new wing object.
	 * @param address The address of the physical wing board.
	 * @param data A UDP datagram packet originating from a wing.
	 */
	EWing(QObject* parent, const QHostAddress& address,
	      const QByteArray& data);

	/**
	 * Destructor.
	 */
	~EWing();

public:
	/**
	 * Check, if the type of data is output (from the wing's perspective).
	 *
	 * @param datagram The data, whose type to check.
	 */
	static bool isOutputData(const QByteArray& datagram);

	/********************************************************************
	 * Wing data
	 ********************************************************************/
public:
	/**
	 * Get the address of the device.
	 *
	 * @return The IP address of the wing board.
	 */
	QHostAddress address() const { return m_address; }

	/**
	 * Get the type of the device (see Type enum).
	 *
	 * @return The type of the device
	 */
	Type type() const { return m_type; }

	/**
	 * Return the name of the wing.
	 *
	 * This function is pure virtual and must be implemented in each of the
	 * inheriting classes.
	 *
	 * @return The name of the device in the form "<type> at <address>"
	 */
	virtual QString name() const = 0;
	
	/**
	 * Return an info string describing the device's state.
	 *
	 * This function is pure virtual and must be implemented in each of the
	 * inheriting classes.
	 *
	 * @return Information string.
	 */
	virtual QString infoText() const = 0;

	/**
	 * Get the wing's firmware version.
	 *
	 * @return Firmware version
	 */
	unsigned char firmware() const { return m_firmware; }

public:
	/**
	 * Resolve the exact type of the wing from the given data packet.
	 *
	 * @param data The data packet to resolve
	 * @return The wing type
	 */
	static Type resolveType(const QByteArray& data);

protected:
	/**
	 * Resolve the wing's firmware version.
	 *
	 * @param data The data packet to resolve
	 * @return Firmware version (0-255)
	 */
	unsigned char resolveFirmware(const QByteArray& data);

protected:
	QHostAddress m_address;
	Type m_type;
	unsigned char m_firmware;

	/********************************************************************
	 * Input data
	 ********************************************************************/
public:
	/**
	 * Parse input data and generate signals for each changed value.
	 *
	 * This function is pure virtual and must be implemented in each of the
	 * inheriting classes.
	 *
	 * @param data The data packet to parse
	 */
	virtual void parseData(const QByteArray& data) = 0;

	/**
	 * Send feedback data to the wing.
	 *
	 * @param channel The channel to send feedback data to
	 * @param value The feedback value to set to the given channel
	 */
	virtual void feedBack(t_input_channel channel, t_input_value value);

	/**
	 * Get the cached value of the given channel.
	 *
	 * @param channel A channel whose value to get
	 * @return The channel's value (0 if not found)
	 */
	unsigned char cacheValue(int channel);

protected:
	/**
	 * Set the value of a channel and emit valueChanged() if the value has
	 * changed from its previous state.
	 *
	 * @param channel A channel, whose value to change
	 * @param value The value to set
	 */
	void setCacheValue(int channel, char value);

signals:
	/**
	 * Changed values are signalled with this signal.
	 *
	 * @param channel The number of the changed channel
	 * @param value The new value for the channel
	 */
	void valueChanged(t_input_channel channel, t_input_value value);

protected:
	QByteArray m_values;
};

#endif
