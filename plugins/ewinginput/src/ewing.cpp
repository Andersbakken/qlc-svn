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

/** ENTTEC wings send data thru UDP port 3330 */
const int EWing::UDPPort = 3330;

/****************************************************************************
 * Initialization
 ****************************************************************************/

EWing::EWing(QObject* parent, const QHostAddress& address, const QByteArray& data)
    : QObject(parent)
{
    m_address = address;
    m_type = resolveType(data);
    m_firmware = resolveFirmware(data);
    m_page = EWING_PAGE_MIN;
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

QString EWing::infoText() const
{
    QString str;
    str  = QString("<B>%1</B>").arg(name());
    str += QString("<P>");
    str += tr("Firmware version %1").arg(int(m_firmware));
    str += QString("<BR>");
    str += tr("Device is operating correctly.");
    str += QString("</P>");
    return str;
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

QHostAddress EWing::address() const
{
    return m_address;
}

EWing::Type EWing::type() const
{
    return m_type;
}

uchar EWing::firmware() const
{
    return m_firmware;
}

EWing::Type EWing::resolveType(const QByteArray& data)
{
    /* Check, if there's enough bytes for wing flags */
    if (data.size() < EWING_BYTE_FLAGS)
    {
        qWarning() << Q_FUNC_INFO
                   << "Unable to determine wing type."
                   << "Expected at least" << EWING_BYTE_FLAGS
                   << "bytes but got only" << data.size();
        return Unknown;
    }

    unsigned char flags = data[EWING_BYTE_FLAGS];
    return EWing::Type(flags & EWING_FLAGS_MASK_TYPE);
}

unsigned char EWing::resolveFirmware(const QByteArray& data)
{
    /* Check, if there's enough bytes for wing flags */
    if (data.size() < EWING_BYTE_FIRMWARE)
    {
        qWarning() << Q_FUNC_INFO
                   << "Unable to determine firmware version."
                   << "Expected at least" << EWING_BYTE_FIRMWARE
                   << "bytes but got only" << data.size();
        return 0;
    }

    return data[EWING_BYTE_FIRMWARE];
}

/****************************************************************************
 * Page
 ****************************************************************************/

void EWing::nextPage()
{
    if (m_page == EWING_PAGE_MAX)
        m_page = EWING_PAGE_MIN;
    else
        m_page++;
}

void EWing::previousPage()
{
    if (m_page == EWING_PAGE_MIN)
        m_page = EWING_PAGE_MAX;
    else
        m_page--;
}

/****************************************************************************
 * Input data
 ****************************************************************************/

uchar EWing::cacheValue(int channel)
{
    if (channel >= m_values.size())
        return 0;
    else
        return m_values[channel];
}

void EWing::setCacheValue(int channel, uchar value)
{
    if (channel >= m_values.size())
        return;

    if (channel != EWING_INVALID_CHANNEL && m_values[channel] != char(value))
    {
        m_values[channel] = char(value);
        emit valueChanged(channel, value);
    }
}

void EWing::feedBack(quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

uchar EWing::toBCD(uchar number)
{
    uchar bcd = ((number / 10) & 0x0F) << 4;
    bcd |= (number % 10) & 0x0F;
    return bcd;
}
