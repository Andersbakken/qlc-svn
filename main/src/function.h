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

#include <qstring.h>
#include <pthread.h>
#include <qlist.h>
#include <qfile.h>
#include "sequenceprovider.h"
#include "classes.h"

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

  Device* device() const;
  void setDevice(Device* device);

  Type type() const;
  QString typeString() const;

  virtual Event* getEvent(Feeder* feeder) = 0;
  virtual void recalculateSpeed(Feeder* feeder) = 0;

  /* Used to emit an unRegistered() signal from this function */
  virtual void unRegister();

  /* Save this function to a file */
  virtual void saveToFile(QFile &file) = 0;

  /* Read this function's characteristics from a string list */
  virtual void createContents(QList<QString> &list) = 0;

 signals:
  void unRegistered(Function*);
  void destroyed();

 protected:
  QString m_name;
  Type m_type;
  DeviceClass* m_deviceClass;
  Device* m_device;

  bool m_running;
};

#endif
