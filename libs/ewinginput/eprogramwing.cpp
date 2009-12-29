/*
  Q Light Controller
  eprogramwing.cpp

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

#include "eprogramwing.h"

/****************************************************************************
 * Program wing specifics
 ****************************************************************************/

#define EWING_PROGRAM_BYTE_BUTTON 6
#define EWING_PROGRAM_BUTTON_SIZE 9

#define EWING_PROGRAM_BYTE_ENCODER 25
#define EWING_PROGRAM_ENCODER_SIZE 3

/** Should constitute up to 75 channels (with some unused ones in the middle) */
#define EWING_PROGRAM_CHANNEL_COUNT (8 * EWING_PROGRAM_BUTTON_SIZE) \
					+ EWING_PROGRAM_ENCODER_SIZE

/****************************************************************************
 * Initialization
 ****************************************************************************/

EProgramWing::EProgramWing(QObject* parent, const QHostAddress& address,
			   const QByteArray& data)
	: EWing(parent, address, data)
{
	m_values = QByteArray(EWING_PROGRAM_CHANNEL_COUNT, 0);

	m_channelMap[0] = 6;
	m_channelMap[1] = 5;
	m_channelMap[2] = EWING_INVALID_CHANNEL;
	m_channelMap[3] = 4;
	m_channelMap[4] = 3;
	m_channelMap[5] = 2;
	m_channelMap[6] = 1;
	m_channelMap[7] = 0;

	m_channelMap[8] = 14;
	m_channelMap[9] = 13;
	m_channelMap[10] = 12;
	m_channelMap[11] = 11;
	m_channelMap[12] = 10;
	m_channelMap[13] = 9;
	m_channelMap[14] = 8;
	m_channelMap[15] = 7;

	m_channelMap[16] = 17;
	m_channelMap[17] = 16;
	m_channelMap[18] = EWING_INVALID_CHANNEL;
	m_channelMap[19] = EWING_INVALID_CHANNEL;
	m_channelMap[20] = EWING_INVALID_CHANNEL;
	m_channelMap[21] = EWING_INVALID_CHANNEL;
	m_channelMap[22] = EWING_INVALID_CHANNEL;
	m_channelMap[23] = 15;

	m_channelMap[24] = 25;
	m_channelMap[25] = 24;
	m_channelMap[26] = 23;
	m_channelMap[27] = 22;
	m_channelMap[28] = 21;
	m_channelMap[29] = 20;
	m_channelMap[30] = 19;
	m_channelMap[31] = 18;

	m_channelMap[32] = 38;
	m_channelMap[33] = 37;
	m_channelMap[34] = 36;
	m_channelMap[35] = 30;
	m_channelMap[36] = 29;
	m_channelMap[37] = 28;
	m_channelMap[38] = 27;
	m_channelMap[39] = 26;

	m_channelMap[40] = 56;
	m_channelMap[41] = 50;
	m_channelMap[42] = 49;
	m_channelMap[43] = 48;
	m_channelMap[44] = 47;
	m_channelMap[45] = 46;
	m_channelMap[46] = 40;
	m_channelMap[47] = 39;

	m_channelMap[48] = 57;
	m_channelMap[49] = 35;
	m_channelMap[50] = 34;
	m_channelMap[51] = 45;
	m_channelMap[52] = 44;
	m_channelMap[53] = 55;
	m_channelMap[54] = 54;
	m_channelMap[55] = 58;

	m_channelMap[56] = 32;
	m_channelMap[57] = 31;
	m_channelMap[58] = 43;
	m_channelMap[59] = 42;
	m_channelMap[60] = 41;
	m_channelMap[61] = 53;
	m_channelMap[62] = 52;
	m_channelMap[63] = 51;

	m_channelMap[64] = EWING_INVALID_CHANNEL;
	m_channelMap[65] = 64;
	m_channelMap[66] = 63;
	m_channelMap[67] = 62;
	m_channelMap[68] = 61;
	m_channelMap[69] = 60;
	m_channelMap[70] = 59;
	m_channelMap[71] = 33;

	m_channelMap[72] = 65;
	m_channelMap[73] = 66;
	m_channelMap[74] = 67;

	/* Take initial values from the first received datagram packet.
	   The plugin hasn't yet connected to valueChanged() signal, so this
	   won't cause any input events. */
	parseData(data);
}

EProgramWing::~EProgramWing()
{
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QString EProgramWing::name() const
{
	QString name("Program");
	name += tr(" at ");
	name += m_address.toString();

	return name;
}

QString EProgramWing::infoText() const
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

void EProgramWing::parseData(const QByteArray& data)
{
	char value;
	int size;
	int byte;

	/* Check that we can get all buttons from the packet */
	size = EWING_PROGRAM_BYTE_BUTTON + EWING_PROGRAM_BUTTON_SIZE;
	if (data.size() < size)
	{
		qWarning() << "Expected at least" << size
			   << "bytes for buttons but got only" << data.size();
		return;
	}

	/* Read the state of each button */
	for (byte = size - 1; byte >= EWING_PROGRAM_BYTE_BUTTON; byte--)
	{
		/* Each byte has 8 button values as binary bits */
		for (int bit = 7; bit >= 0; bit--)
		{
			int key;

			/* Calculate the key number, which is 0-64 */
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
	size = EWING_PROGRAM_BYTE_ENCODER + EWING_PROGRAM_ENCODER_SIZE;
	if (data.size() < size)
	{
		qWarning() << "Expected at least" << size
			   << "bytes for sliders but got only" << data.size();
		return;
	}

	/* Read the direction of each encoder. 255 = CW, 1 = CCW, 0 = NOP. */
	for (int encoder = 0; encoder < EWING_PROGRAM_ENCODER_SIZE; encoder++)
	{
		int channel = (EWING_PROGRAM_CHANNEL_COUNT -
			      EWING_PROGRAM_ENCODER_SIZE) + encoder;
		unsigned char cvalue = cacheValue(m_channelMap[channel]);

		value = data[EWING_PROGRAM_BYTE_ENCODER + encoder];
		if (value == char(255))
			setCacheValue(m_channelMap[channel], ++cvalue);
		else if (value == char(1))
			setCacheValue(m_channelMap[channel], --cvalue);
	}
}
