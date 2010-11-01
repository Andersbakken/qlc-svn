/*
  Q Light Controller
  universearray.h

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

#ifndef UNIVERSEARRAY_H
#define UNIVERSEARRAY_H

#include <QByteArray>
#include <QSet>

#include "qlcchannel.h"

class UniverseArray : public QByteArray
{
public:
    /** Construct a new UniverseArray of given size */
    UniverseArray(int size);
    virtual ~UniverseArray();

    /************************************************************************
     * Grand Master
     ************************************************************************/
public:
    enum GrandMasterValueMode
    {
        GMLimit, /** Limit maximum values to current GM value */
        GMReduce /** Reduce channel values by a fraction (0-100%) */
    };

    enum GrandMasterChannelMode
    {
        GMIntensity,  /** GM applied only for Intensity channels */
        GMAllChannels /** GM applied for all channels */
    };

    /**
     * Set the way how Grand Master should treat its value. @See enum
     * GrandMasterValueMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGrandMasterValueMode(GrandMasterValueMode mode);

    /**
     * Get the Grand Master value mode.
     * @See setGrandMasterValueMode() and enum GrandMasterValueMode.
     *
     * @return Current value mode
     */
    GrandMasterValueMode grandMasterValueMode() const;

    /**
     * Set the way how Grand Master should treat channels. @See enum
     * GrandMasterChannelMode for more info on the modes.
     *
     * @param mode The mode to set
     */
    void setGrandMasterChannelMode(GrandMasterChannelMode mode);

    /**
     * Get the Grand Master channel mode.
     * @See setGrandMasterChannelMode() and enum GrandMasterChannelMode.
     *
     * @return Current channel mode
     */
    GrandMasterChannelMode grandMasterChannelMode() const;

    /**
     * Set the Grand Master value as a DMX value 0-255. This value is
     * converted to a fraction according to the current mode.
     */
    void setGrandMasterValue(uchar value);

    /**
     * Get the current Grand Master value as a DMX value (0 - 255)
     *
     * @return Current Grand Master value in DMX
     */
    uchar grandMasterValue() const;

    /**
     * Get the current Grand Master value as a fraction 0.0 - 1.0
     *
     * @return Current Grand Master value as a fraction
     */
    double grandMasterFraction() const;

    /**
     * Un-apply Grand Master to a channel. Basically just removes the channel
     * from a cache of Intensity-enabled channels. Useful when a fixture is
     * replaced by another (whose intensity channels might be elsewhere than
     * with the previous one).
     *
     * @param channel The DMX channel to reset
     */
    void gmResetChannel(int channel);

    /**
     * Get the current post-Grand-Master values (to be written to output HW)
     *
     * @return The current values
     */
    QByteArray postGMValues() const;

protected:
    /**
     * Apply Grand Master to the value.
     *
     * @param channel The channel to apply Grand Master to
     * @param value The value to write
     * @param group The channel's channel group
     * @return Value filtered through grand master (if applicable)
     */
    uchar grandMasterify(int channel, uchar value, QLCChannel::Group group);

protected:
    GrandMasterValueMode m_gmValueMode;
    GrandMasterChannelMode m_gmChannelMode;
    uchar m_gmValue;
    double m_gmFraction;
    QSet <int> m_gmChannels;
    QByteArray* m_postGMValues;

    /************************************************************************
     * Writing
     ************************************************************************/
public:
    /**
     * Write a value to a DMX channel, taking Grand Master into account, if
     * applicable.
     *
     * @param channel The channel number to write to
     * @param value The value to write
     * @param group The channel's channel group
     * @return true if successful, otherwise false
     */
    bool write(int channel, uchar value,
               QLCChannel::Group group = QLCChannel::NoGroup);
};

#endif
