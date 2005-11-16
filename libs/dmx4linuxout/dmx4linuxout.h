/*
  Q Light Controller
  dmx4linuxout.h
  
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

#ifndef DMX4LINUXOUT_H
#define DMX4LINUXOUT_H

#include "../common/outputplugin.h"
#include "../../main/src/types.h"

#include <qptrlist.h>
#include <qstring.h>

class ConfigureDMX4LinuxOut;
class QPoint;

extern "C" OutputPlugin* create(t_plugin_id id);
extern "C" void destroy(OutputPlugin* object);

class DMX4LinuxOut : public OutputPlugin
{
  Q_OBJECT

  friend class ConfigureDMX4LinuxOut;

 public:
  DMX4LinuxOut(t_plugin_id id);
  ~DMX4LinuxOut();

  // Plugin methods
  int open();
  int close();
  bool isOpen();
  int configure();
  QString infoText();
  void contextMenu(QPoint pos);

  int setConfigDirectory(QString dir);
  int saveSettings();
  int loadSettings();

  // OutputPlugin methods
  int writeChannel(t_channel channel, t_value value);
  int writeRange(t_channel address, t_value* values, t_channel num);

  int readChannel(t_channel channel, t_value &value);
  int readRange(t_channel address, t_value* values, t_channel num);

  // Own methods
  QString deviceName() { return m_deviceName; }
  void setDeviceName(QString name) { m_deviceName = name; }

 private slots:
  void slotContextMenuCallback(int item);

 private:
  void activate();
  void createContents(QPtrList <QString> &list);

 private:
  QString m_deviceName;
  QString m_configDir;
  int m_device;
  t_value m_values[KChannelMax];
};

#endif
