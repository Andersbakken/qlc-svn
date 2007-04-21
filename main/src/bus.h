/*
  Q Light Controller
  bus.h
  
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

#ifndef BUS_H
#define BUS_H

#include <qptrlist.h>
#include <qobject.h>

#include "common/types.h"

class QFile;
class QString;
class Function;
class BusEmitter;

class Bus
{
 private:
  Bus();

 public:
  ~Bus();

 public:
  static void init();

  static const BusEmitter* emitter() { return s_busEmitter; }

  static const bool value(t_bus_id, t_bus_value&);
  static bool setValue(t_bus_id, t_bus_value);

  static const QString name(t_bus_id);
  static bool setName(t_bus_id, QString);

  static bool addListener(t_bus_id, Function*);
  static bool removeListener(t_bus_id, Function*);

  static void createContents(QPtrList <QString> &list);
  static void saveToFile(QFile &file);

 private:
  t_bus_id m_id;
  t_bus_value m_value;
  QString m_name;

  QPtrList <Function> m_listeners;

 private:
  static Bus* s_busArray;
  static t_bus_id s_nextID;
  static BusEmitter* s_busEmitter;
};

//
// Bus Emitter class to handle signal emission
//
class BusEmitter : public QObject
{
  Q_OBJECT

  friend class Bus;

 public:
  BusEmitter() {};
  ~BusEmitter() {};

 protected:
  void emitValueChanged(t_bus_id id, t_bus_value value)
    { emit valueChanged(id, value); }

  void emitNameChanged(t_bus_id id, QString name)
    { emit nameChanged(id, name); }

 signals:
  void valueChanged(t_bus_id, t_bus_value);
  void nameChanged(t_bus_id, const QString&);
};

#endif
