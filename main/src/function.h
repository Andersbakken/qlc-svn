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

typedef unsigned short int t_function_id;

#include <qobject.h>
#include <qlist.h>

#include "dmxchannel.h"

class QFile;
class QString;

class DMXDevice;
class Event;
class Feeder;
class LogicalChannel;

const t_value KReadyValue = USHRT_MAX;

const t_function_id KFunctionIDMin = 1;
const t_function_id KFunctionIDMax = USHRT_MAX;

class Function : public QObject
{
  Q_OBJECT

 public:
  Function(t_function_id = 0);
  virtual ~Function();

  enum Type
    {
      Undefined = 0x0,
      Collection = 0x1,
      Scene = 0x2,
      Chaser = 0x4,
      Sequence = 0x8
    };

  QString name() const;
  void setName(QString name);

  DMXDevice* device() const;
  void setDevice(DMXDevice* device);

  t_function_id id() { return m_id; }

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
  void destroyed(t_function_id id);

 protected:
  QString m_name;
  Type m_type;
  DMXDevice* m_device;

  bool m_running;

  t_function_id m_id;
  static t_function_id _nextFunctionId;

 public:
  static void resetFunctionId() { _nextFunctionId = KFunctionIDMin; }
};

#endif
