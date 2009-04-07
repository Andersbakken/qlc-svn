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

#include "common/qlctypes.h"

class EWing : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	EWing(QObject* parent, const QHostAddress& address,
		const QByteArray& data);
	~EWing();

	/** There are currently three types of wings from ENTTEC. */
	enum Type
	{
		Unknown = 0,
		Playback = 1,
		Shortcut = 2,
		Program = 3
	};

	/** The UDP port of ENTTEC wing devices */
	static const int UDPPort;

	/** Check, if data type is output (from the wing's perspective) */
	static bool isOutputData(const QByteArray& datagram);

	/********************************************************************
	 * Wing data
	 ********************************************************************/
public:
	/** Get the device's address */
	QHostAddress address() const { return m_address; }

	/** Get the device's type (see Type enum) */
	Type type() const { return m_type; }

	/** Return the name of this wing */
	QString name() const;
	
	/** Return info regarding the device's state */
	QString infoText() const;

protected:
	/** Resolve the wing type from the data array */
	Type resolveType(const QByteArray& data);

	/** Resolve firmware version */
	unsigned char EWing::resolveFirmware(const QByteArray& data);

protected:
	QHostAddress m_address;
	Type m_type;
	unsigned char m_firmware;

	/********************************************************************
	 * Input data
	 ********************************************************************/
public:
	/** Parse input data and generate signals for each changed value */
	void parseData(const QByteArray& data);

protected:
	/** Set the value of a channel and emit valueChanged() if appropriate */
	void setCacheValue(int channel, char value);

	/** Read a Playback Wing datagram packet */
	void parsePlaybackData(const QByteArray& data);

	/** Read a Shortcut Wing datagram packet */
	void parseShortcutData(const QByteArray& data);

	/** Read a Program Wing datagram packet */
	void parseProgramData(const QByteArray& data);

signals:
	/** Changed values are signalled with this signal */
	void valueChanged(t_input_channel channel, t_input_value value);

protected:
	QByteArray m_values;
	QMap <int,int> m_channelMap;
};

#endif
