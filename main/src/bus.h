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
#include <qlist.h>

class QFile;
class QString;
class QPoint;

class Bus : public QObject
{
  Q_OBJECT

 public:
  Bus();
  ~Bus();

  enum Type { Generic = 0, Speed = 1 };

  unsigned long id() { return m_id; }

  unsigned long value() const { return m_value; }
  void setValue(unsigned long value);

  Type type() { return m_type; }
  void setType(Type type) { m_type = type; }

  QString name() { return m_name; }
  void setName(QString name) { m_name = QString(name); }

  QString infoText();

  void saveToFile(QFile &file);
  void createContents(QList <QString> &list);

  Bus& operator=(Bus &b);

 signals:
  void dataChanged(const Bus*);

 private:
  unsigned long m_id;
  unsigned long m_value;
  Type m_type;
  QString m_name;
};

#endif
