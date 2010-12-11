/*
  Q Light Controller
  fadechannel.cpp

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

#include <QDebug>

#include "fadechannel.h"

FadeChannel::FadeChannel(quint32 address, QLCChannel::Group grp,
                         uchar start, uchar target, uchar current)
    : m_address(address)
    , m_group(grp)
    , m_start(start)
    , m_target(target)
    , m_current(current)
    , m_ready(false)
{
}

FadeChannel::FadeChannel(const FadeChannel& ch)
    : m_address(ch.m_address)
    , m_group(ch.m_group)
    , m_start(ch.m_start)
    , m_target(ch.m_target)
    , m_current(ch.m_current)
    , m_ready(ch.m_ready)
{
}

FadeChannel::~FadeChannel()
{
}

void FadeChannel::setAddress(quint32 addr)
{
    m_address = addr;
}

quint32 FadeChannel::address() const
{
    return m_address;
}

void FadeChannel::setGroup(QLCChannel::Group grp)
{
    m_group = grp;
}

QLCChannel::Group FadeChannel::group() const
{
    return m_group;
}

void FadeChannel::setStart(uchar value)
{
    m_start = value;
}

uchar FadeChannel::start() const
{
    return m_start;
}

void FadeChannel::setTarget(uchar value)
{
    m_target = value;
}

uchar FadeChannel::target() const
{
    return m_target;
}

void FadeChannel::setCurrent(uchar value)
{
    m_current = value;
}

uchar FadeChannel::current() const
{
    return m_current;
}

void FadeChannel::setReady(bool rdy)
{
    m_ready = rdy;
}

bool FadeChannel::isReady() const
{
    return m_ready;
}

uchar FadeChannel::calculateCurrent(quint32 fadeTime, quint32 elapsedTime)
{
    // Return the target value if all time has been consumed or the channel
    // has been marked ready.
    if (elapsedTime >= fadeTime || m_ready == true)
    {
        m_current = m_target;
        return m_current;
    }

    // Time scale is basically a percentage (0.0 - 1.0) of remaining time.
    // Add 1.0 to both to get correct scale (fadeTime==1 means two steps)
    qreal timeScale = qreal(elapsedTime + 1.0) / qreal(fadeTime + 1.0);

    m_current = m_target - m_start;
    m_current = qint32(qreal(m_current) * timeScale);
    m_current += m_start;

    return static_cast<uchar>(m_current);
}
