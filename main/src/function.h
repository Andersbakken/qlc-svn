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
class VirtualController;

class Function : public QThread
{
 public:
  // Constructor. See .cpp file for explanation on id's
  Function(t_function_id id = KFunctionIDAuto);

  // Destructor
  virtual ~Function();

  // Possible function types
  enum Type
    {
      Undefined = 0x0,
      Collection = 0x1,
      Scene = 0x2,
      Chaser = 0x4,
      Sequence = 0x8
    };

 public:
  // Return the name of this function
  QString name() { return m_name; }

  // Set a name for this function
  virtual void setName(QString name);

  // Return the device that this function is associated to
  Device* device() { return m_device; }

  // Set the device that this function is associated to
  virtual void setDevice(Device* device);

  // This function's unique ID
  t_function_id id() { return m_id; }

  // Return the type of this function (see the enum above)
  Type type() { return m_type; }

  // Same thing, only this time as a string
  QString typeString() const;

  // The bus for speed setting
  void setBus(t_bus_id id);
  t_bus_id bus() const { return m_busID; }
  virtual void busValueChanged(t_bus_id, t_bus_value) {}

  // Save this function to a file
  virtual void saveToFile(QFile &file) = 0;

  // Read this function's characteristics from a string list
  virtual void createContents(QPtrList <QString> &list) = 0;

  // Start this function (start() and run() were already taken... :)
  // Use only these functions, not QThread::start()!!!
  virtual bool engage(QObject* virtualController); // From vcbutton
  virtual bool engage(Function* parentFunction); // From chaser & collection

  // Stop this function
  virtual void stop() = 0;

  // If the buffer is empty and this is true, FunctionConsumer removes
  // this function from its list; this function has finished.
  bool removeAfterEmpty() { return m_removeAfterEmpty; }

  // After this function has been removed from FunctionConsumer's
  // list, it calls this function to delete any run time pointers etc.
  // This function also notifies the virtual controller (if any) of
  // finished operation.
  virtual void freeRunTimeData() = 0;

  // This function is implemented only in such functions that can be
  // parents to another function (e.g. only in chasers & collections).
  // Those functions that can be children, will call this function from
  // their parents (if any)
  virtual void childFinished() {}

  // Return the eventbuffer object. Only for FunctionFonsumer's use.
  EventBuffer* eventBuffer() const { return m_eventBuffer; }

 protected:
  // Semi-permanent function data
  QString m_name;
  Type m_type;
  Device* m_device;
  t_function_id m_id;

  // Run-time data
  bool m_removeAfterEmpty;
  EventBuffer* m_eventBuffer;

  time_t m_timeSpan;

  QObject* m_virtualController;
  Function* m_parentFunction;

  // This is used to determine if this function is running.
  // It is the very last variable that is set to false when this function
  // is definitely not running.
  bool m_running;

  // The respective mutex for m_running
  QMutex m_startMutex;

  // This can be used from the inside to signal that this function should
  // be stopped by setting it true.
  bool m_stopped;
  QMutex m_stopMutex;

  // Bus for setting the speed
  t_bus_id m_busID;

 private:
  // Next ID that will be assigned to a new function
  static t_function_id _nextFunctionId;

 public:
  static void resetFunctionId() { _nextFunctionId = KFunctionIDMin; }
};


//
// Function Stop Event
//
#include <qevent.h>

const int KFunctionStopEvent        ( QEvent::User + 1 );
const int KFunctionSpeedEvent       ( QEvent::User + 2 );

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

class FunctionSpeedEvent : public QCustomEvent
{
 public:
  FunctionSpeedEvent(t_bus_value value) 
    : QCustomEvent( KFunctionSpeedEvent ),
    m_value(value) {}

  t_bus_value value() const { return m_value; }

 private:
  t_bus_value m_value;
};

#endif
