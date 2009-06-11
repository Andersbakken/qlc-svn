/*
  Q Light Controller - Unit test
  test_engine.cpp

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

#include <QApplication>
#include <QtTest>

#include "bus_test.h"

/* This file includes tests for QLC's ENGINE components. UI tests are done
   separately. */
int main(int argc, char** argv)
{
	QApplication qapp(argc, argv);

	Bus_Test bus;
	QTest::qExec(&bus, argc, argv);

	return 0;
}
