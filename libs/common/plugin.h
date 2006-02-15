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
#include <qmainwindow.h>
#include <limits.h>

typedef unsigned short t_plugin_id;

const t_plugin_id KPluginID    = 0;
const t_plugin_id KPluginIDMin = 1;
const t_plugin_id KPluginIDMax = USHRT_MAX;

class Plugin : public QObject
{
  Q_OBJECT

 public:
  Plugin(t_plugin_id id);
  virtual ~Plugin();

  enum PluginType
    {
      OutputType,
      InputType
    };

  // These functions have to be overwritten
  virtual int open() = 0; // Generic open function
  virtual int close() = 0; // Generic close function
  virtual bool isOpen() = 0; // Generic open-or-not? function
  virtual int configure() = 0; // Generic configure function
  virtual QString infoText() = 0; // Text that is displayed in device manager
  virtual void contextMenu(QPoint pos) = 0; // Invoke a context menu

  virtual int setConfigDirectory(QString dir) = 0; // Set the config file dir
  virtual int saveSettings() = 0; // Save settings to a file in config dir
  virtual int loadSettings() = 0; // Load settings from a file in config dir

  // Standard functions that don't have to be overwritten
  QString name() { return m_name; }
  t_plugin_id id() { return m_id; }
  unsigned long version() { return m_version; }
  PluginType type() { return m_type; }
  void setHandle(void* handle) { m_handle = handle; }
  void* handle() { return m_handle; }
  const void setParentApp(QObject* Parent){ m_parentApp =  Parent;}
  QObject* m_parentApp;

 signals:
  void activated(Plugin*);

 protected:
  QString m_name;
  PluginType m_type;
  unsigned long m_version;
  t_plugin_id m_id;
  void* m_handle;

};

#endif
