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

#include <qobject.h>
#include <qptrlist.h>
#include <limits.h>

#include "types.h"

class QFile;
class QString;
class QPoint;

class Bus : public QObject
{
  Q_OBJECT

 public:
  Bus(t_bus_id id = 0);
  ~Bus();

  Bus& operator=(Bus &b);

  QString infoText();

  t_bus_id id() { return m_id; }

  t_bus_value value() const { return m_value; }
  void setValue(t_bus_value value);

  QString name() { return m_name; }
  void setName(QString name);

  bool isStatic() const { return m_static; }

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  static Bus* defaultFadeBus() { return m_defaultFadeBus; }
  static Bus* defaultHoldBus() { return m_defaultHoldBus; }

 signals:
  void dataChanged(const Bus*);

 private:
  t_bus_id m_id;
  t_bus_value m_value;
  QString m_name;
  bool m_static;

  static Bus* m_defaultFadeBus;
  static Bus* m_defaultHoldBus;
  static t_bus_id m_nextBusID;
};

#endif
