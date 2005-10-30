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

#include <qobject.h>
#include <qthread.h>
#include <limits.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qevent.h>

#include "types.h"

const int KFunctionStopEvent        ( QEvent::User + 1 );

class Device;
class EventBuffer;
class QFile;
class VirtualController;
class Bus;
class Function;

//
// Weird namespace, wouldn't work without...
//
namespace FunctionNS
{
  //
  // A listener class that listens to bus value changes
  //
  class BusListener : public QObject
    {
      Q_OBJECT
	
     public:
      BusListener(t_function_id);
      ~BusListener();
      
     public slots:
      void slotBusValueChanged(t_bus_id id, t_bus_value value);

     protected:
      t_function_id m_functionID;
    };
};


class Function : public QThread
{
 public:
  // Possible function types
  enum Type
    {
      Undefined = 0x0,
      Collection = 0x1,
      Scene = 0x2,
      Chaser = 0x4,
      EFX = 0x5,
      Sequence = 0x8
    };

  // Constructor. See .cpp file for explanation on id's
  Function(Type);

  // Destructor
  virtual ~Function();

 public:
  // This function's unique ID
  t_function_id id() { return m_id; }

  // Set this function's ID
  void setID(t_function_id);

  // Return the type of this function (see the enum above)
  Function::Type type() { return m_type; }

  // Convert a type to string
  static QString typeToString(Function::Type);

  // Convert a string to type enum
  static Type stringToType(QString);

  // Create one function from list
  static Function* create(QPtrList <QString> &list);

  // Return the name of this function
  virtual QString name() { return m_name; }

  // Set a name for this function
  virtual bool setName(QString name);

  // Return the device that this function is associated to
  virtual t_device_id device() { return m_deviceID; }

  // Set the device that this function is associated to
  virtual bool setDevice(t_device_id);

  // The bus for speed setting
  t_bus_id busID() const { return m_busID; }
  virtual bool setBus(t_bus_id id);
  virtual void busValueChanged(t_bus_id, t_bus_value) {}

  // Number of channels that this function uses.
  // The actual inheriting function class should define means
  // for setting the channel count.
  virtual t_channel channels() const { return m_channels; }

  // Save this function to a file
  virtual void saveToFile(QFile &file) = 0;

  // Read this function's characteristics from a string list
  virtual void createContents(QPtrList <QString> &list) = 0;

  // When the mode is changed to Operate, this is called to make all mem
  // allocations so they are not done during run-time (and thus creating
  // huge overhead)
  virtual void arm() {};

  // When the mode is changed back to Design, this is called to free
  // any run-time allocations
  virtual void disarm() {};

  // Start this function (start() and run() were already taken... :)
  // Use only these functions, not QThread::start()!!!
  virtual bool engage(QObject* virtualController); // From vcbutton
  virtual bool engage(Function* parentFunction); // From chaser & collection

  // Stop this function
  virtual void stop();

  // If the buffer is empty and this is true, FunctionConsumer removes
  // this function from its list; this function has finished.
  bool removeAfterEmpty() { return m_removeAfterEmpty; }

  // After this function has been removed from FunctionConsumer's
  // list, it calls this function to do some post-run cleanup (but not delete)
  // This function also notifies the virtual controller or controller
  // function (whichever started this function) of finished operation.
  virtual void cleanup() = 0;

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
  t_function_id m_id;
  t_device_id m_deviceID;
  t_bus_id m_busID;
  t_channel m_channels;

  // Run-time data
  EventBuffer* m_eventBuffer;
  QObject* m_virtualController;
  Function* m_parentFunction;

  // This is used to determine if this function is running.
  // It is the very last variable that is set to false when this function
  // is definitely not running.
  bool m_running;

  // This can be used from the inside to signal that this function should
  // be stopped by setting it true.
  bool m_stopped;

  // Signals that removal of this function from consumer can be started
  bool m_removeAfterEmpty;

  // The respective mutex for m_running
  QMutex m_startMutex;

  // Bus listener
  FunctionNS::BusListener* m_listener;
};


//
// Function Stop Event
//
class FunctionStopEvent : public QCustomEvent
{
 public:
  FunctionStopEvent(t_function_id id)
    : QCustomEvent( KFunctionStopEvent )
    { m_functionID = id; }

  t_function_id functionID() { return m_functionID; }

 private:
  t_function_id m_functionID;
};

#endif
