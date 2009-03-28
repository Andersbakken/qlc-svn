/*
  Q Light Controller
  ewing.cpp

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

#include <QHostAddress>
#include <QByteArray>
#include <QString>

#include "ewing.h"

#define EWING_HEADER_OUTPUT "WODD"
#define EWING_HEADER_INPUT "WIDD"

#define EWING_BYTE_HEADER   0 /* 4 bytes, either "WODD" or "WIDD" */
#define EWING_HEADER_SIZE   4

#define EWING_BYTE_FIRMWARE 4 /* Firmware version (0-255) */

#define EWING_BYTE_FLAGS    5 /* Wing flags */

#define EWING_FLAGS_MASK_PGUP 1 << 7
#define EWING_FLAGS_MASK_PGDN 1 << 6
#define EWING_FLAGS_MASK_BACK 1 << 5
#define EWING_FLAGS_MASK_GO   1 << 4
#define EWING_FLAGS_MASK_TYPE 0x3

/****************************************************************************
 * Playback wing specifics
 ****************************************************************************/

#define EWING_PLAYBACK_BYTE_BUTTON 7 /* Bytes 7-11 are for buttons */
#define EWING_PLAYBACK_BUTTON_SIZE 5 /* 5 bytes of button states */

#define EWING_PLAYBACK_BYTE_SLIDER 15 /* Bytes 15-25 are for sliders */
#define EWING_PLAYBACK_SLIDER_SIZE 10 /* 10 slider values in all */

/** Should constitute up to 50 channels */
#define EWING_PLAYBACK_CHANNEL_COUNT 8 * EWING_PLAYBACK_BUTTON_SIZE \
					+ EWING_PLAYBACK_SLIDER_SIZE

/****************************************************************************
 * Shortcut wing specifics
 ****************************************************************************/
/*
The ENTTEC Shortcut wing produces packets that contain
EWING_SHORTCUT_BUTTON_SIZE bytes of button data. Each bit in these bytes
signifies the state of one button. Thus, each byte contains 8 buttons (except
the first one only 4 buttons), and they are in reversed order:

EWING_SHORTCUT_BYTE_BUTTON +
                07  06  05  04  03  02  01  00
-----------------------------------------------
bit 0 : buttons 07, 15, 23, 31, 39, 47, 55, N/A
bit 1 : buttons 06, 14, 22, 30, 38, 46, 54, N/A
bit 2 : buttons 05, 13, 21, 29, 37, 45, 53, N/A
bit 3 : buttons 04, 12, 20, 28, 36, 44, 52, N/A
bit 4 : buttons 03, 11, 19, 27, 35, 43, 51, 59
bit 5 : buttons 02, 10, 18, 26, 34, 42, 50, 58
bit 6 : buttons 01, 09, 17, 25, 33, 41, 49, 57
bit 7 : buttons 00, 08, 16, 24, 32, 40, 48, 56

As it can be seen from the table above, the byte order is also reversed:

EWING_SHORTCUT_BYTE_BUTTON + 0: Buttons 56 - 59 (4 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 1: Buttons 48 - 55 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 2: Buttons 40 - 47 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 3: Buttons 32 - 39 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 4: Buttons 24 - 31 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 5: Buttons 16 - 23 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 6: Buttons 08 - 15 (8 buttons)
EWING_SHORTCUT_BYTE_BUTTON + 7: Buttons 00 - 07 (8 buttons)
*/

#define EWING_SHORTCUT_BYTE_BUTTON 6
#define EWING_SHORTCUT_BUTTON_SIZE 8

/** Should constitute up to 64 values (with the last 4 unused) */
#define EWING_SHORTCUT_CHANNEL_COUNT 8 * EWING_SHORTCUT_BUTTON_SIZE
 
/****************************************************************************
 * Program wing specifics
 ****************************************************************************/

/****************************************************************************
 * Initialization
 ****************************************************************************/

/** ENTTEC wings send UDP data thru port 3330 */
const int EWing::UDPPort = 3330;

EWing::EWing(QObject* parent, const QHostAddress& address,
	const QByteArray& data) : QObject(parent)
{
	m_address = address;
	m_type = resolveType(data);

	if (m_type == Playback)
		m_values.resize(EWING_PLAYBACK_CHANNEL_COUNT);
	else if (m_type == Shortcut)
		m_values.resize(EWING_SHORTCUT_CHANNEL_COUNT);
}

EWing::~EWing()
{
}

bool EWing::isOutputData(const QByteArray& data)
{
	/* Check, if there's enough bytes for the header */
	if (data.size() < EWING_HEADER_SIZE)
		return false;

	QByteArray header(data.mid(EWING_BYTE_HEADER, EWING_HEADER_SIZE));
	return (header == EWING_HEADER_OUTPUT);
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

EWing::Type EWing::resolveType(const QByteArray& data)
{
	/* Check, if there's enough bytes for wing flags */
	if (data.size() < 6)
		return Unknown;

	char flags = data[EWING_BYTE_FLAGS];
	return EWing::Type(flags & EWING_FLAGS_MASK_TYPE);
}

QString EWing::name() const
{
	QString name;
	
	switch (m_type)
	{
	default:
		name = "Unknown device";
		break;

	case Playback:
		name = "ENTTEC Playback Wing";
		break;

	case Shortcut:
		name = "ENTTEC Shortcut Wing";
		break;

	case Program:
		name = "ENTTEC Program Wing";
		break;
	}

	name += tr(" at ");
	name += m_address.toString();

	return name;
}

QString EWing::infoText() const
{
	QString str;

	str  = QString("<B>%1</B>").arg(name());
	str += tr("<P>Device is operating correctly</P>");

	return str;
}

/****************************************************************************
 * Input data
 ****************************************************************************/

void EWing::parseData(const QByteArray& data)
{
	switch(m_type)
	{
	case Playback:
		parsePlaybackData(data);
		break;

	case Shortcut:
		parseShortcutData(data);
		break;

	case Program:
		parseProgramData(data);
		break;

	default:
		break;
	}
}

void EWing::setCacheValue(int channel, char value)
{
	Q_ASSERT(m_values.size() < channel);

	if (m_values[channel] != value)
	{
		m_values[channel] = value;
		emit valueChanged(channel, value);
	}
}

void EWing::parsePlaybackData(const QByteArray& data)
{
	int channel;
	char value;
	int size;
	int byte;

	/* Check that we can get all buttons from the packet */
	size = EWING_PLAYBACK_BYTE_BUTTON + EWING_PLAYBACK_BUTTON_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each button */
	for (byte = size - 1; byte >= EWING_PLAYBACK_BYTE_BUTTON; byte--)
	{
		/* Each byte has 8 button values as binary bits */
		for (int bit = 7; bit >= 0; bit--)
		{
			/* Calculate the channel number (0-39) */
			channel = (byte - EWING_PLAYBACK_BYTE_BUTTON) * 8;
			channel += bit;

			/* Start button channels after sliders */
			channel += EWING_PLAYBACK_SLIDER_SIZE;

			/* 0 = button down, 1 = button up */
			if ((data[byte] & (1 << bit)) == 0)
				value = 255;
			else
				value = 0;

			setCacheValue(channel, value);
		}
	}

	/* Check that we can get all sliders from the packet */
	size = EWING_PLAYBACK_BYTE_SLIDER + EWING_PLAYBACK_SLIDER_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each slider. Each value takes all 8 bits. */
	for (int slider = 0; slider < EWING_PLAYBACK_SLIDER_SIZE; slider++)
	{
		value = data[EWING_PLAYBACK_BYTE_SLIDER + slider];
		/* Start slider channels from zero */
		setCacheValue(slider, value);
	}
}

void EWing::parseShortcutData(const QByteArray& data)
{
	int channel;
	char value;
	int size;

	/* Check that we can get all channels from the packet */
	size = EWING_SHORTCUT_BYTE_BUTTON + EWING_SHORTCUT_BUTTON_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each button */
	for (int byte = size - 1; byte >= EWING_SHORTCUT_BYTE_BUTTON; byte--)
	{
		/* Each byte has 8 button values as binary bits */
		for (int bit = 7; bit >= 0; bit--)
		{
			/* Calculate the channel number (0-59) */
			channel = (byte - EWING_SHORTCUT_BYTE_BUTTON) * 8;
			channel += bit;

			/* There's only 60 channels in a Shortcut Wing, but
			   the data packet contains 64 values. So don't read
			   the 4 excess bytes. */
			if (channel > 59)
				break;

			/* 0 = button down, 1 = button up */
			if ((data[byte] & (1 << bit)) == 0)
				value = 255;
			else
				value = 0;

			setCacheValue(channel, value);
		}
	}
}

void EWing::parseProgramData(const QByteArray& data)
{
}
