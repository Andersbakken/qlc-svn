/*
  Q Light Controller - Unit test
  universearray_test.cpp

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
#include <sys/time.h>

#include "universearray_test.h"

#define protected public
#include "universearray.h"
#undef protected

void UniverseArray_Test::initial()
{
    UniverseArray ua(127);
    QCOMPARE(ua.size(), 127);
    QCOMPARE(ua.grandMasterChannelMode(), UniverseArray::GMIntensity);
    QCOMPARE(ua.grandMasterValueMode(), UniverseArray::GMReduce);
    QCOMPARE(ua.grandMasterValue(), uchar(255));
    QCOMPARE(ua.grandMasterFraction(), double(1.0));
}

void UniverseArray_Test::grandMasterChannelMode()
{
    UniverseArray ua(1);

    ua.setGrandMasterChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.grandMasterChannelMode(), UniverseArray::GMAllChannels);

    ua.setGrandMasterChannelMode(UniverseArray::GMIntensity);
    QCOMPARE(ua.grandMasterChannelMode(), UniverseArray::GMIntensity);
}

void UniverseArray_Test::grandMasterValueMode()
{
    UniverseArray ua(1);

    ua.setGrandMasterValueMode(UniverseArray::GMLimit);
    QCOMPARE(ua.grandMasterValueMode(), UniverseArray::GMLimit);

    ua.setGrandMasterValueMode(UniverseArray::GMReduce);
    QCOMPARE(ua.grandMasterValueMode(), UniverseArray::GMReduce);
}

void UniverseArray_Test::grandMasterValue()
{
    UniverseArray ua(1);

    for (int i = 0; i < UCHAR_MAX; i++)
    {
        ua.setGrandMasterValue(uchar(i));
        QCOMPARE(ua.grandMasterValue(), uchar(i));
        QCOMPARE(ua.grandMasterFraction(), (double(i) / double(UCHAR_MAX)));
    }

    ua.setGrandMasterValue(0);
    QCOMPARE(ua.grandMasterValue(), uchar(0));
    QCOMPARE(ua.grandMasterFraction(), double(0));

    ua.setGrandMasterValue(255);
    QCOMPARE(ua.grandMasterValue(), uchar(255));
    QCOMPARE(ua.grandMasterFraction(), double(1));
}

void UniverseArray_Test::grandMasterify()
{
    UniverseArray ua(1);

    QCOMPARE(ua.m_gmChannels.size(), 0);
    QCOMPARE(ua.grandMasterify(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Colour), uchar(200));

    ua.setGrandMasterValue(127);
    QCOMPARE(ua.grandMasterify(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Colour), uchar(200));

    ua.setGrandMasterValueMode(UniverseArray::GMLimit);
    QCOMPARE(ua.grandMasterify(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(ua.grandMasterify(0, 255, QLCChannel::Colour), uchar(255));

    ua.setGrandMasterChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.grandMasterify(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(ua.grandMasterify(0, 255, QLCChannel::Colour), uchar(127));

    ua.setGrandMasterValueMode(UniverseArray::GMReduce);
    QCOMPARE(ua.grandMasterify(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(ua.grandMasterify(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(ua.grandMasterify(0, 255, QLCChannel::Colour), uchar(127));

    QCOMPARE(ua.m_gmChannels.size(), 1);
}

void UniverseArray_Test::write()
{
    UniverseArray ua(10);

    QVERIFY(ua.write(10, 255, QLCChannel::Intensity) == false);
    QCOMPARE(ua.postGMValues().data()[9], char(0));
    QCOMPARE(ua.postGMValues().data()[4], char(0));
    QCOMPARE(ua.postGMValues().data()[0], char(0));

    QVERIFY(ua.write(9, 255, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues().data()[9], char(255));
    QCOMPARE(ua.postGMValues().data()[4], char(0));
    QCOMPARE(ua.postGMValues().data()[0], char(0));

    QVERIFY(ua.write(0, 255, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues().data()[9], char(255));
    QCOMPARE(ua.postGMValues().data()[4], char(0));
    QCOMPARE(ua.postGMValues().data()[0], char(255));

    ua.setGrandMasterValue(127);
    QCOMPARE(ua.postGMValues().data()[9], char(127));
    QCOMPARE(ua.postGMValues().data()[4], char(0));
    QCOMPARE(ua.postGMValues().data()[0], char(127));

    QVERIFY(ua.write(4, 200, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues().data()[9], char(127));
    QCOMPARE(ua.postGMValues().data()[4], char(100));
    QCOMPARE(ua.postGMValues().data()[0], char(127));
}

void UniverseArray_Test::setGrandMasterValueEfficiency()
{
    UniverseArray* ua = new UniverseArray(512 * KUniverseCount);
    int i;

    for (i = 0; i < int(512 * KUniverseCount); i++)
        ua->write(i, 200, QLCChannel::Intensity);

    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.37ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me..? */
    QBENCHMARK
    {
        // This is slower than plain write() because UA has to dig out each
        // Intensity-enabled channel from its internal QSet.
        ua->setGrandMasterValue(127);
    }

    for (i = 0; i < int(512 * KUniverseCount); i++)
        QCOMPARE(ua->postGMValues().data()[i], char(100));
}

void UniverseArray_Test::writeEfficiency()
{
    UniverseArray* ua = new UniverseArray(512 * KUniverseCount);
    ua->setGrandMasterValue(127);

    int i;
    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.15ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me..? */
    QBENCHMARK
    {
        for (i = 0; i < int(512 * KUniverseCount); i++)
            ua->write(i, 200, QLCChannel::Intensity);
    }

    for (i = 0; i < int(512 * KUniverseCount); i++)
        QCOMPARE(ua->postGMValues().data()[i], char(100));
}
