/*
  Q Light Controller
  settings.h

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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <qobject.h>
#include <qptrlist.h>

const QString KConfigFile ( "qlc.conf" );
const QString KQLCUserDir (     ".qlc" );

class QString;
class ConfigItem;

class Settings : public QObject
{
  Q_OBJECT

 public:
  Settings();
  ~Settings();

  static QString trueValue() { return QString("True"); }
  static QString falseValue() { return QString("False"); }

  QPtrList <ConfigItem>* items() { return &m_items; }

 public:
  int set(QString key, QString text);
  int set(QString key, int value);
  int get(QString key, QString &text);
  int remove(QString key);

  //
  // Read settings file to a list
  //
  void load();
  void save();
  void createContents(QPtrList <QString> &list);

 signals:
  void outputPluginChanged(const QString&);

 protected:
  QPtrList <ConfigItem> m_items;
};

#endif
