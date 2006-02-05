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

#ifndef CONFIGUREMIDIINOUT_H
#define CONFIGUREMIDIINOUT_H

#include "uic_configuremidi-inout.h"

class MidiInOut;

class ConfigureMidiInOut : public UI_ConfigureMidiInOut
{
  Q_OBJECT

 public:
  ConfigureMidiInOut(MidiInOut* plugin);
  virtual ~ConfigureMidiInOut();

  void setDevice(QString deviceName);
  QString device();

  void updateStatus();

 private slots:
  void slotActivateClicked();
  void slotDeactivateClicked();

 private:
  MidiInOut* m_plugin;
};

#endif
