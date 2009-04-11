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

#define EWING_PROGRAM_BYTE_ENCODER 20
#define EWING_PROGRAM_ENCODER_SIZE 3

/** Should constitute up to  values (with the last 4 unused) */
#define EWING_PROGRAM_CHANNEL_COUNT 8 * EWING_PROGRAM_BUTTON_SIZE \
					+ EWING_PROGRAM_ENCODER_SIZE

/****************************************************************************
 * Initialization
 ****************************************************************************/

EProgramWing::EProgramWing(QObject* parent, const QHostAddress& address,
	const QByteArray& data) : EWing(parent, address, data)
{
	m_values.resize(EWING_PROGRAM_CHANNEL_COUNT);
	
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
}
