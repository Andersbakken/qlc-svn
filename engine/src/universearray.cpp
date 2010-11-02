/*
  Q Light Controller
  universearray.cpp

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

#include <math.h>
#include "universearray.h"
#include "qlctypes.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

UniverseArray::UniverseArray(int size)
{
    m_preGMValues = new QByteArray(size, char(0));
    m_postGMValues = new QByteArray(size, char(0));
    m_gmValueMode = GMReduce;
    m_gmChannelMode = GMIntensity;
    m_gmValue = 255;
    m_gmFraction = 1.0;
}

UniverseArray::~UniverseArray()
{
    delete m_preGMValues;
    delete m_postGMValues;
}

int UniverseArray::size() const
{
    return m_preGMValues->size();
}

/****************************************************************************
 * Grand Master
 ****************************************************************************/

void UniverseArray::setGrandMasterValueMode(UniverseArray::GrandMasterValueMode mode)
{
    m_gmValueMode = mode;
}

UniverseArray::GrandMasterValueMode UniverseArray::grandMasterValueMode() const
{
    return m_gmValueMode;
}

void UniverseArray::setGrandMasterChannelMode(UniverseArray::GrandMasterChannelMode mode)
{
    m_gmChannelMode = mode;
}

UniverseArray::GrandMasterChannelMode UniverseArray::grandMasterChannelMode() const
{
    return m_gmChannelMode;
}

void UniverseArray::setGrandMasterValue(uchar value)
{
    m_gmValue = value;
    m_gmFraction = CLAMP(double(value) / double(UCHAR_MAX), 0.0, 1.0);

    QSetIterator <int> it(m_gmChannels);
    while (it.hasNext() == true)
    {
        int channel(it.next());
        char chValue(m_preGMValues->data()[channel]);
        write(channel, chValue, QLCChannel::Intensity);
    }
}

uchar UniverseArray::grandMasterValue() const
{
    return m_gmValue;
}

double UniverseArray::grandMasterFraction() const
{
    return m_gmFraction;
}

void UniverseArray::gmReset()
{
    m_gmChannels.clear();
}

const QByteArray UniverseArray::postGMValues() const
{
    return *m_postGMValues;
}

const QByteArray UniverseArray::preGMValues() const
{
    return *m_preGMValues;
}

uchar UniverseArray::grandMasterify(int channel, uchar value, QLCChannel::Group group)
{
    if (value == 0)
    {
        m_gmChannels.remove(channel);
        return value;
    }

    if ((grandMasterChannelMode() == GMIntensity && group == QLCChannel::Intensity) ||
        (grandMasterChannelMode() == GMAllChannels))
    {
        if (grandMasterValueMode() == GMLimit)
            value = MIN(value, grandMasterValue());
        else
            value = char(floor((double(value) * grandMasterFraction()) + 0.5));

        m_gmChannels << channel;
    }

    return value;
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool UniverseArray::write(int channel, uchar value, QLCChannel::Group group)
{
    if (channel >= size())
        return false;

    m_preGMValues->data()[channel] = char(value);

    value = grandMasterify(channel, value, group);
    m_postGMValues->data()[channel] = char(value);

    return true;
}
