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
#include <QMessageBox>
#include <QByteArray>
#include <QString>

#include "ewing.h"

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
 * Playback wing specifics
 ****************************************************************************/
/*
The ENTTEC Playback wing produces packets that contain
EWING_PLAYBACK_BUTTON_SIZE bytes of button data. Each bit in the button bytes
signifies the state of one button. Thus, each byte contains 8 buttons, and
they are in reversed order, with some of them (27, 28, 29, 22, 23, 24, 25, 26)
mixed up in a very weird way:

EWING_PLAYBACK_BYTE_BUTTON +
                04  03  02  01  00
----------------------------------
bit 0 : buttons 07, 15, 28, 31, 39
bit 1 : buttons 06, 14, 27, 30, 38
bit 2 : buttons 05, 13, 21, 26, 37
bit 3 : buttons 04, 12, 20, 25, 36
bit 4 : buttons 03, 11, 19, 24, 35
bit 5 : buttons 02, 10, 18, 23, 34
bit 6 : buttons 01, 09, 17, 22, 33
bit 7 : buttons 00, 08, 16, 29, 32

As it can be seen from the table above, the byte order is also reversed:

EWING_PLAYBACK_BYTE_BUTTON + 0: Buttons 32 - 39 (8 buttons)
EWING_PLAYBACK_BYTE_BUTTON + 1: Buttons 24 - 31 (8 buttons)
EWING_PLAYBACK_BYTE_BUTTON + 2: Buttons 16 - 23 (8 buttons)
EWING_PLAYBACK_BYTE_BUTTON + 3: Buttons 08 - 15 (8 buttons)
EWING_PLAYBACK_BYTE_BUTTON + 4: Buttons 00 - 07 (8 buttons)

The Playback Wing contains also EWING_PLAYBACK_SLIDER_SIZE bytes of slider data,
where each byte contains an 8bit char value signifying the slider value. Slider
bytes are not reversed:

EWING_PLAYBACK_BYTE_SLIDER + 0: Slider 01 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 1: Slider 02 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 2: Slider 03 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 3: Slider 04 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 4: Slider 05 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 5: Slider 06 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 6: Slider 07 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 7: Slider 08 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 8: Slider 09 (0-255)
EWING_PLAYBACK_BYTE_SLIDER + 9: Slider 10 (0-255)
*/

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

#define EWING_PROGRAM_BYTE_BUTTON 6
#define EWING_PROGRAM_BUTTON_SIZE 9

#define EWING_PROGRAM_BYTE_ENCODER 20
#define EWING_PROGRAM_ENCODER_SIZE 3

/** Should constitute up to  values (with the last 4 unused) */
#define EWING_PROGRAM_CHANNEL_COUNT 8 * EWING_PROGRAM_BUTTON_SIZE \
					+ EWING_PROGRAM_ENCODER_SIZE

/****************************************************************************
 * Initialization
 ****************************************************************************/

/** ENTTEC wings send data thru UDP port 3330 */
const int EWing::UDPPort = 3330;

EWing::EWing(QObject* parent, const QHostAddress& address,
	const QByteArray& data) : QObject(parent)
{
	m_address = address;
	m_type = resolveType(data);
	m_firmware = resolveFirmware(data);

	if (m_type == Playback)
	{
		m_values.resize(EWING_PLAYBACK_CHANNEL_COUNT);

		/* Playback wing keys seem to be in a somewhat weird order */
		m_channelMap[0] = 7 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[1] = 6 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[2] = 5 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[3] = 4 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[4] = 3 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[5] = 2 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[6] = 1 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[7] = 0 + EWING_PLAYBACK_SLIDER_SIZE;
		
		m_channelMap[8] = 15 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[9] = 14 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[10] = 13 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[11] = 12 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[12] = 11 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[13] = 10 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[14] = 9 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[15] = 8 + EWING_PLAYBACK_SLIDER_SIZE;
		
		m_channelMap[16] = 28 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[17] = 27 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[18] = 21 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[19] = 20 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[20] = 19 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[21] = 18 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[22] = 17 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[23] = 16 + EWING_PLAYBACK_SLIDER_SIZE;
		
		m_channelMap[24] = 31 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[25] = 30 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[26] = 26 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[27] = 25 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[28] = 24 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[29] = 23 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[30] = 22 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[31] = 29 + EWING_PLAYBACK_SLIDER_SIZE;

		m_channelMap[32] = 39 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[33] = 38 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[34] = 37 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[35] = 36 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[36] = 35 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[37] = 34 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[38] = 33 + EWING_PLAYBACK_SLIDER_SIZE;
		m_channelMap[39] = 32 + EWING_PLAYBACK_SLIDER_SIZE;
	}
	else if (m_type == Shortcut)
	{
		m_values.resize(EWING_SHORTCUT_CHANNEL_COUNT);
	}
	else if (m_type == Program)
	{
		m_values.resize(EWING_PROGRAM_CHANNEL_COUNT);
	}

	/* Take initial values from the first received datagram packet.
	   The plugin hasn't yet connected to valueChanged() signal, so this
	   won't cause any input events. */
	parseData(data);
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
	if (data.size() < EWING_BYTE_FLAGS)
		return Unknown;

	unsigned char flags = data[EWING_BYTE_FLAGS];
	return EWing::Type(flags & EWING_FLAGS_MASK_TYPE);
}

unsigned char EWing::resolveFirmware(const QByteArray& data)
{
	/* Check, if there's enough bytes for wing flags */
	if (data.size() < EWING_BYTE_FIRMWARE)
		return 0;

	return data[EWING_BYTE_FIRMWARE];
}

QString EWing::name() const
{
	QString name;
	
	switch (m_type)
	{
	case Playback:
		name = "Playback";
		break;

	case Shortcut:
		name = "Shortcut";
		break;

	case Program:
		name = "Program";
		break;

	default:
		name = "Unknown";
		break;
	}

	name += tr(" at ");
	name += m_address.toString();

	return name;
}

QString EWing::infoText() const
{
	QString str;

	str  = QString("<B>%1</B><BR>").arg(name());
	str += tr("Firmware version %1").arg(int(m_firmware));
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
		qDebug() << channel << "=" << t_input_value(value);
		m_values[channel] = value;
		emit valueChanged(channel, t_input_value(value));
	}
}

void EWing::parsePlaybackData(const QByteArray& data)
{
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
			int key;

			/* Calculate the key number, which is 10-49, since
			   sliders are mapped to 0-9. */
			key = (size - byte - 1) * 8;
			key += bit;

			/* 0 = button down, 1 = button up */
			if ((data[byte] & (1 << bit)) == 0)
				value = 255;
			else
				value = 0;

			/* Get the correct channel number for each key. */
			setCacheValue(m_channelMap[key], value);
		}
	}

	/* Check that we can get all sliders from the packet */
	size = EWING_PLAYBACK_BYTE_SLIDER + EWING_PLAYBACK_SLIDER_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each slider. Each value takes all 8 bits. */
	for (int slider = 0; slider < EWING_PLAYBACK_SLIDER_SIZE; slider++)
	{
		value = data[EWING_PLAYBACK_BYTE_SLIDER + slider];

		/* Slider channels start from zero */
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
			   the extra 4 bits. */
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
	int channel;
	char value;
	int size;

	/* Check that we can get all channels from the packet */
	size = EWING_PROGRAM_BYTE_BUTTON + EWING_PROGRAM_BUTTON_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each button */
	for (int byte = size - 1; byte >= EWING_PROGRAM_BYTE_BUTTON; byte--)
	{
		/* Each byte has 8 button values as binary bits */
		for (int bit = 7; bit >= 0; bit--)
		{
			/* Calculate the channel number (0-72) */
			channel = (byte - EWING_PROGRAM_BYTE_BUTTON) * 8;
			channel += bit;

			/* There's only 64 buttons in a Program Wing, but
			   the data packet contains 72 values. So don't read
			   the extra 7 bytes. */
			if (channel > 64)
				break;

			/* 0 = button down, 1 = button up */
			if ((data[byte] & (1 << bit)) == 0)
				value = 255;
			else
				value = 0;

			setCacheValue(channel, value);
		}
	}

	size = EWING_PROGRAM_BYTE_ENCODER + EWING_PROGRAM_ENCODER_SIZE;
	Q_ASSERT(data.size() >= size);

	/* Read the state of each encoder */
	for (int encoder = 0; encoder < EWING_PROGRAM_ENCODER_SIZE; encoder++)
	{
		unsigned char dataValue;

		/* Encoder channels come after button channels */
		channel = (EWING_PROGRAM_BYTE_BUTTON * 8) + encoder;

		dataValue = data[EWING_PROGRAM_BYTE_ENCODER + encoder];
		
		/* Encoders are relative controls, so increment or decrement.
		   TODO: Now the values wrap, should they stop at both ends? */
		if (dataValue == 255)
		{
			value = m_values[channel];
			value++;
			setCacheValue(channel, value);
		}
		else if (dataValue == 1)
		{
			value = m_values[channel];
			value--;
			setCacheValue(channel, value);
		}
		else
		{
			/* Encoder value unchanged */
		}
	}
}
