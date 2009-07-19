/*
  Q Light Controller - Unit test
  scene_test.h

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

#ifndef SCENE_TEST_H
#define SCENE_TEST_H

#include <QObject>
#include <common/qlcfixturedefcache.h>
#include "../mastertimer.h"

class Scene_Test : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void initial();
	void values();
	void fixtureRemoval();
	void loadSuccess();
	void loadWrongType();
	void loadWrongRoot();
	void save();
	void copyFrom();
	void createCopy();

	void arm();
	void armMissingFixture();
	void armTooManyChannels();

	void flashUnflash();

	void writeBusZero();
	void writeBusOne();
	void writeBusTwo();
	void writeBusFiveChangeToZeroInTheMiddle();
	void writeNonZeroStartingValues();

private:
	QLCFixtureDefCache m_cache;
};

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

class MasterTimerStub : public MasterTimer
{
	Q_OBJECT

public:
	MasterTimerStub(QObject* parent);
	~MasterTimerStub();

	void startFunction(Function* function);
	void stopFunction(Function* function);

	QList <Function*> m_list;
};

#endif
