/*
  Q Light Controller
  function.h
  
  Copyright (C) 2004 Heikki Junnila
  
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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <qthread.h>
#include <limits.h>
#include <qptrlist.h>
#include <qstring.h>

#include "types.h"

class Device;
class EventBuffer;
class QFile;

class Function : public QThread
{
 public:
  Function(t_function_id id = KFunctionIDAuto);
  virtual ~Function();

 public:
  enum Type
    {
      Undefined = 0x0,
      Collection = 0x1,
      Scene = 0x2,
      Chaser = 0x4,
      Sequence = 0x8
    };

  QString name() { return m_name; }
  virtual void setName(QString name);

  Device* device() { return m_device; }
  virtual void setDevice(Device* device);

  t_function_id id() { return m_id; }

  Type type() { return m_type; }
  QString typeString() const;

  // Save this function to a file
  virtual void saveToFile(QFile &file) = 0;

  // Read this function's characteristics from a string list
  virtual void createContents(QPtrList <QString> &list) = 0;

  // New producer - consumer - style functions
  virtual void speedChange(unsigned long newTimeSpan) = 0;
  virtual void stop() = 0;

  time_t timeSpan() { return m_timeSpan; }
  time_t startTime() { return m_startTime; }

  void setVirtualController(QObject* virtualController) { m_virtualController = virtualController; }
  
  EventBuffer* eventBuffer() const { return m_eventBuffer; }

  bool removeAfterEmpty() { return m_removeAfterEmpty; }

  virtual void freeRunTimeData() = 0;

 protected:
  QString m_name;
  Type m_type;
  Device* m_device;
  QObject* m_virtualController;

  bool m_running;

  t_function_id m_id;
  static t_function_id _nextFunctionId;

  EventBuffer* m_eventBuffer;

  time_t m_timeSpan;
  time_t m_startTime;
  bool m_removeAfterEmpty;

 public:
  static void resetFunctionId() { _nextFunctionId = KFunctionIDMin; }
};

#include <qevent.h>

const int KFunctionStopEvent        ( QEvent::User + 1 );

class FunctionStopEvent : public QCustomEvent
{
 public:
  FunctionStopEvent(Function* stoppedFunction) 
    : QCustomEvent( KFunctionStopEvent ),
    m_function(stoppedFunction) {}

  Function* function() { return m_function; }

 private:
  Function* m_function;
};

#endif
