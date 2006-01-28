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

#ifndef CONFIGUREUSBDMXOUT_H
#define CONFIGUREUSBDMXOUT_H

#include "uic_configureusbdmxout.h"

class UsbDmxOut;

class ConfigureUsbDmxOut : public UI_ConfigureUsbDmxOut
{
  Q_OBJECT

 public:
  ConfigureUsbDmxOut(UsbDmxOut* plugin);
  virtual ~ConfigureUsbDmxOut();

  void setDevice(QString deviceName);
  QString device();

  void updateStatus();

 private slots:
  void slotActivateClicked();

 private:
  UsbDmxOut* m_plugin;
};

#endif
