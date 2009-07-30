/*
  Q Light Controller - Unit test
  function_test.cpp

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
#include <QtXml>

#include "function_test.h"
#include "../function.h"

void Function_Test::invalidId()
{
	QVERIFY(Function::invalidId() == -1);
}

void Function_Test::typeToString()
{
	QVERIFY(Function::typeToString(Function::Undefined) == "Undefined");
	QVERIFY(Function::typeToString(Function::Scene) == "Scene");
	QVERIFY(Function::typeToString(Function::Chaser) == "Chaser");
	QVERIFY(Function::typeToString(Function::EFX) == "EFX");
	QVERIFY(Function::typeToString(Function::Collection) == "Collection");

	QVERIFY(Function::typeToString(Function::Type(42)) == "Undefined");
	QVERIFY(Function::typeToString(Function::Type(31337)) == "Undefined");
}

void Function_Test::stringToType()
{
	QVERIFY(Function::stringToType("Undefined") == Function::Undefined);
	QVERIFY(Function::stringToType("Scene") == Function::Scene);
	QVERIFY(Function::stringToType("Chaser") == Function::Chaser);
	QVERIFY(Function::stringToType("EFX") == Function::EFX);
	QVERIFY(Function::stringToType("Collection") == Function::Collection);

	QVERIFY(Function::stringToType("Foobar") == Function::Undefined);
	QVERIFY(Function::stringToType("Xyzzy") == Function::Undefined);
}

void Function_Test::runOrderToString()
{
	QVERIFY(Function::runOrderToString(Function::Loop) == "Loop");
	QVERIFY(Function::runOrderToString(Function::SingleShot) == "SingleShot");
	QVERIFY(Function::runOrderToString(Function::PingPong) == "PingPong");

	QVERIFY(Function::runOrderToString(Function::RunOrder(42)) == "Loop");
	QVERIFY(Function::runOrderToString(Function::RunOrder(69)) == "Loop");
}

void Function_Test::stringToRunOrder()
{
	QVERIFY(Function::stringToRunOrder("Loop") == Function::Loop);
	QVERIFY(Function::stringToRunOrder("SingleShot") == Function::SingleShot);
	QVERIFY(Function::stringToRunOrder("PingPong") == Function::PingPong);

	QVERIFY(Function::stringToRunOrder("Foobar") == Function::Loop);
	QVERIFY(Function::stringToRunOrder("Xyzzy") == Function::Loop);
}

void Function_Test::directionToString()
{
	QVERIFY(Function::directionToString(Function::Forward) == "Forward");
	QVERIFY(Function::directionToString(Function::Backward) == "Backward");

	QVERIFY(Function::directionToString(Function::Direction(42)) == "Forward");
	QVERIFY(Function::directionToString(Function::Direction(69)) == "Forward");
}

void Function_Test::stringToDirection()
{
	QVERIFY(Function::stringToDirection("Forward") == Function::Forward);
	QVERIFY(Function::stringToDirection("Backward") == Function::Backward);

	QVERIFY(Function::stringToDirection("Foobar") == Function::Forward);
	QVERIFY(Function::stringToDirection("Xyzzy") == Function::Forward);
}

