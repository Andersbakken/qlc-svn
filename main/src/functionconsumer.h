/*
  Q Light Controller
  functionconsumer.h

  Copyright (C) 2000, 2001, 2002, 2003, 2004 Heikki Junnila

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

#ifndef FUNCTIONCONSUMER_H
#define FUNCTIONCONSUMER_H

#include <qthread.h>
#include <qptrlist.h>

#include "types.h"
#include "function.h"

const int KFrequency(64);

class FunctionConsumer : public QThread
{
 public:
  FunctionConsumer();
  virtual ~FunctionConsumer();

  // Number of running functions
  int runningFunctions();

  // Run a function
  void cue(Function* f);

  // Stop all functions
  void purge();

  // Get current timecode
  void timeCode(t_bus_value& timeCode);

  // Functions that control the consumer
  void stop();
  void init();

 protected:
  virtual void run();
  void event(time_t);

  void incrementTimeCode();

 protected:
  bool m_running;
  int m_fd;

  QPtrList <Function> m_functionList;
  QMutex m_functionListMutex;

  t_bus_value m_timeCode;
  QMutex m_timeCodeMutex;

  t_value* m_event;
  Function* m_function;
  t_channel m_channel;
};

#endif
