/*
  Q Light Controller - Unit test
  efx_test.h

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

#ifndef EFX_TEST_H
#define EFX_TEST_H

#include <QObject>
#include "qlcfixturedefcache.h"

class Doc;

class EFX_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initial();
    void algorithmNames();
    void width();
    void height();
    void rotation();
    void xOffset();
    void yOffset();
    void xFrequency();
    void yFrequency();
    void xPhase();
    void yPhase();
    void fixtures();
    void propagationMode();
    void startStopScenes();

    void previewCircle();
    void previewEight();
    void previewLine();
    //void previewTriangle();
    void previewDiamond();
    void previewLissajous();

    void rotateAndScale();
    void widthHeightOffset();

    void copyFrom();
    void createCopy();

    void loadXAxis();
    void loadYAxis();
    void loadYAxisWrongRoot();
    void loadAxisNoXY();
    void loadSuccess();
    void loadWrongType();
    void loadWrongRoot();
    void loadDuplicateFixture();
    void save();

    void armSuccess();
    void armMissingStartScene();
    void armMissingStopScene();
    void armMissingFixture();
    void writeStartStopScenes();

private:
    Doc* m_doc;
    QLCFixtureDefCache m_cache;
};

#endif
