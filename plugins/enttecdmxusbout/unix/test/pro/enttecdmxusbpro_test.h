/*
  Q Light Controller - Unit tests
  enttecdmxusbpro_test.h

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef ENTTECDMXUSBPRO_TEST_H
#define ENTTECDMXUSBPRO_TEST_H

#include <QObject>

class EnttecDMXUSBPro_Test : public QObject
{
	Q_OBJECT

private:
	void resetCounters();

private slots:
	void construction();
	void extractSerialFailedWrite();
	void extractSerialFailedRead();
	void extractSerialWrongReply();
	void closeAlreadyClosed();
	void closeFail();
	void open();
	void sendDMX();
};

#endif
