/*
  Q Light Controller
  function.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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
#include <qlist.h>

#define VALUE_READY INT_MAX

class QFile;
class QString;

class DeviceClass;
class DMXDevice;
class Event;
class Feeder;
class LogicalChannel;

class Function : public QObject
{
  Q_OBJECT

 public:
  Function();
  virtual ~Function();

  enum Type { Undefined, Collection, Scene, Chaser, Sequence };

  QString name() const;
  void setName(QString name);

  DeviceClass* deviceClass() const;
  void setDeviceClass(DeviceClass* device);

  DMXDevice* device() const;
  void setDevice(DMXDevice* device);

  unsigned long id() { return m_id; }

  Type type() const;
  QString typeString() const;

  virtual Event* getEvent(Feeder* feeder) = 0;
  virtual void recalculateSpeed(Feeder* feeder) = 0;

  /* Signal the function that it is registered -> do some init */
  virtual bool registerFunction(Feeder*);

  /* Used to emit an unRegistered() signal from this function */
  virtual bool unRegisterFunction(Feeder*);

  /* Save this function to a file */
  virtual void saveToFile(QFile &file) = 0;

  /* Read this function's characteristics from a string list */
  virtual void createContents(QList<QString> &list) = 0;

 signals:
  void unRegistered(Function*);
  void destroyed();
  void destroyed(Function*);

 protected:
  QString m_name;
  Type m_type;
  DeviceClass* m_deviceClass;
  DMXDevice* m_device;

  bool m_running;

  unsigned long m_id;
  static unsigned long _nextFunctionId;
};

#endif
