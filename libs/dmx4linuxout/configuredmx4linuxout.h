/*
  Q Light Controller
  configuredmx4linuxout.h
  
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

#ifndef CONFIGUREDMX4LINUXOUT_H
#define CONFIGUREDMX4LINUXOUT_H

#include "uic_configuredmx4linuxout.h"

class DMX4LinuxOut;

class ConfigureDMX4LinuxOut : public UI_ConfigureDMX4LinuxOut
{
  Q_OBJECT

 public:
  ConfigureDMX4LinuxOut(DMX4LinuxOut* plugin);
  virtual ~ConfigureDMX4LinuxOut();

  void setDevice(QString deviceName);
  QString device();

  void updateStatus();

 private slots:
  void slotActivateClicked();

 private:
  DMX4LinuxOut* m_plugin;
};

#endif
