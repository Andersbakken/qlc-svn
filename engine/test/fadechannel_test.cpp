/*
  Q Light Controller - Unit test
  fadechannel_test.cpp

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

#include <QtTest>
#include "qlcchannel.h"
#include "fadechannel_test.h"

#define private public
#include "fadechannel.h"
#undef private

void FadeChannel_Test::address()
{
    FadeChannel fch;
    QCOMPARE(fch.address(), quint32(0));

    for (quint32 i = 0; i <= (32 * 512); i++)
    {
        fch.setAddress(i);
        QCOMPARE(fch.address(), quint32(i));
    }
}

void FadeChannel_Test::group()
{
    FadeChannel fch;
    QCOMPARE(fch.group(), QLCChannel::NoGroup);

    fch.setGroup(QLCChannel::Intensity);
    QCOMPARE(fch.group(), QLCChannel::Intensity);

    fch.setGroup(QLCChannel::Colour);
    QCOMPARE(fch.group(), QLCChannel::Colour);
}

void FadeChannel_Test::start()
{
    FadeChannel fch;
    QCOMPARE(fch.start(), uchar(0));

    for (uchar i = 0; i < 255; i++)
    {
        fch.setStart(i);
        QCOMPARE(fch.start(), i);
    }
}

void FadeChannel_Test::target()
{
    FadeChannel fch;
    QCOMPARE(fch.target(), uchar(0));

    for (uchar i = 0; i < 255; i++)
    {
        fch.setTarget(i);
        QCOMPARE(fch.target(), i);
    }
}

void FadeChannel_Test::current()
{
    FadeChannel fch;
    QCOMPARE(fch.current(), uchar(0));

    for (uchar i = 0; i < 255; i++)
    {
        fch.setCurrent(i);
        QCOMPARE(fch.current(), i);
    }
}

void FadeChannel_Test::calculateCurrent()
{
    FadeChannel fch;
    fch.setStart(0);
    fch.setTarget(255);

    // Simple: 255 ticks to fade from 0 to 255
    for (quint32 time = 0; time <= 255; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(time));
    // Same thing, but the value should stay at 255 same after 255 ticks
    for (quint32 time = 0; time <= 512; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(MIN(time, 255)));

    // Simple reverse: 255 ticks to fade from 255 to 0
    fch.setStart(255);
    fch.setTarget(0);
    for (quint32 time = 0; time <= 255; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(255 - time));

    // A bit more complex involving decimals that don't produce round integers
    fch.setStart(3);
    fch.setTarget(147);
    QCOMPARE(fch.calculateCurrent(13, 0), uchar(13));
    QCOMPARE(fch.calculateCurrent(13, 1), uchar(23));
    QCOMPARE(fch.calculateCurrent(13, 2), uchar(33));
    QCOMPARE(fch.calculateCurrent(13, 3), uchar(44));
    QCOMPARE(fch.calculateCurrent(13, 4), uchar(54));
    QCOMPARE(fch.calculateCurrent(13, 5), uchar(64));
    QCOMPARE(fch.calculateCurrent(13, 6), uchar(75));
    QCOMPARE(fch.calculateCurrent(13, 7), uchar(85));
    QCOMPARE(fch.calculateCurrent(13, 8), uchar(95));
    QCOMPARE(fch.calculateCurrent(13, 9), uchar(105));
    QCOMPARE(fch.calculateCurrent(13, 10), uchar(116));
    QCOMPARE(fch.calculateCurrent(13, 11), uchar(126));
    QCOMPARE(fch.calculateCurrent(13, 12), uchar(136));
    QCOMPARE(fch.calculateCurrent(13, 13), uchar(147));

    // One more to check slower operation (200 ticks for 144 steps)
    fch.setStart(245);
    fch.setTarget(101);
    QCOMPARE(fch.calculateCurrent(200, 0), uchar(245));
    QCOMPARE(fch.calculateCurrent(200, 1), uchar(244));
    QCOMPARE(fch.calculateCurrent(200, 2), uchar(243));
    QCOMPARE(fch.calculateCurrent(200, 3), uchar(243));
    QCOMPARE(fch.calculateCurrent(200, 4), uchar(242));
    QCOMPARE(fch.calculateCurrent(200, 5), uchar(241));
    QCOMPARE(fch.calculateCurrent(200, 6), uchar(240));
    QCOMPARE(fch.calculateCurrent(200, 7), uchar(240));
    QCOMPARE(fch.calculateCurrent(200, 8), uchar(239));
    QCOMPARE(fch.calculateCurrent(200, 9), uchar(238));
    QCOMPARE(fch.calculateCurrent(200, 10), uchar(238));
    QCOMPARE(fch.calculateCurrent(200, 11), uchar(237));
    // Skip...
    QCOMPARE(fch.calculateCurrent(200, 100), uchar(173));
    QCOMPARE(fch.calculateCurrent(200, 101), uchar(172));
    QCOMPARE(fch.calculateCurrent(200, 102), uchar(172));
    // Skip...
    QCOMPARE(fch.calculateCurrent(200, 198), uchar(103));
    QCOMPARE(fch.calculateCurrent(200, 199), uchar(102));
    QCOMPARE(fch.calculateCurrent(200, 200), uchar(101));
}
