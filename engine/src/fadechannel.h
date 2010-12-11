/*
  Q Light Controller
  fadechannel.h

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

#ifndef FADECHANNEL_H
#define FADECHANNEL_H

#include <QtGlobal>
#include "qlcchannel.h"

/**
 * FadeChannel is a helper class used to store individual RUNTIME values for
 * channels as they are operated by various functions during Operate mode.
 *
 * A FadeChannel consists of an absolute DMX address, starting channel value,
 * target value and the current value. FadeChannels are used only during
 * Operate mode, when fixture addresses cannot change anymore, so it is
 * slightly more efficient to store absolute DMX addresses than relative
 * channel numbers. When a Scene starts, it takes the current values of all of
 * its channels and stores them into their respective FadeChannels' $start
 * variable. The Scene then calculates how much time it still has until the
 * values specified in $target need to be set for each involved channel and
 * adjusts the step size accordingly. The more time there's left, the smoother
 * the fade effect will be. Each gradual step is written to the channels' DMX
 * address and also stored to FadeChannels' $current variable for the next
 * round.
 *
 * Although uchar is an UNSIGNED char (0-255) and represents the actual value
 * range of DMX channels, these variables must be SIGNED and able to store
 * bigger values because the floating-point calculations done by functions do
 * not necessarily stop exactly at 0.0 and 255.0, but might go slightly
 * over or under. If these variables were uchars, an overflow might occur and
 * the the functions might never be able to stop.
 */
class FadeChannel
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** Create a new FadeChannel with the given values */
    FadeChannel(quint32 address = 0, QLCChannel::Group grp = QLCChannel::NoGroup,
                uchar start = 0, uchar target = 0, uchar current = 0);

    /** Copy constructor */
    FadeChannel(const FadeChannel& ch);

    /** Destructor */
    virtual ~FadeChannel();

    /************************************************************************
     * Values
     ************************************************************************/
public:
    /** Set the absolute DMX address */
    void setAddress(quint32 addr);

    /** Get the absolute DMX address */
    quint32 address() const;

    /** Set channel group */
    void setGroup(QLCChannel::Group grp);

    /** Get channel group */
    QLCChannel::Group group() const;

    /** Set starting value */
    void setStart(uchar value);

    /** Get starting value */
    uchar start() const;

    /** Set target value */
    void setTarget(uchar value);

    /** Get target value */
    uchar target() const;

    /** Set the current value */
    void setCurrent(uchar value);

    /** Get the current value */
    uchar current() const;

    /** Mark this channel as ready (useful for writing LTP values only once) */
    void setReady(bool rdy);

    /** Check if this channel is ready. Default is false. */
    bool isReady() const;

    /**
     * Calculate current value based on fadeTime and elapsedTime. Basically:
     * "what m_current should be, if you were given $fadeTime ticks to fade
     * from m_start to m_target when $elapsedTime ticks have already passed."
     *
     * Also, if a channel has been marked ready (isReady() == true), this method
     * returns the target value.
     *
     * @param fadeTime Number of ticks to fade from start to target
     * @param elapsedTime Number of ticks already spent
     * @return New current value
     */
    uchar calculateCurrent(quint32 fadeTime, quint32 elapsedTime);

private:
    quint32 m_address;
    QLCChannel::Group m_group;
    qint32 m_start;
    qint32 m_target;
    qint32 m_current;
    bool m_ready;
};

#endif
