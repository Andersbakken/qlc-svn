/*
  Q Light Controller
  plugin.h

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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>

class Plugin : public QObject
{
  Q_OBJECT

 public:
  Plugin(int id);
  virtual ~Plugin();

  enum PluginType
    {
      OutputType,
      InputType
    };

  // These functions have to be overwritten
  virtual bool open() = 0; // Generic open function
  virtual bool close() = 0; // Generic close function
  virtual void configure() = 0; // Generic configure function
  virtual QString infoText() = 0; // Text that is displayed in device manager
  virtual void contextMenu(QPoint pos) = 0; // Invoke a context menu from device manager

  // Standard functions that don't have to be overwritten
  QString name() { return m_name; }
  int id() { return m_id; }
  unsigned long version() { return m_version; }
  PluginType type() { return m_type; }

 protected:
  QString m_name;
  PluginType m_type;
  unsigned long m_version;
  int m_id;
};

#endif
