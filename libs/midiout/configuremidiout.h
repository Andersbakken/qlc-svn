/*
  Q Light Controller
  configuremidiout.h
  
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

#ifndef CONFIGUREMIDIOUT_H
#define CONFIGUREMIDIOUT_H

#include "uic_configuremidiout.h"

class MidiOut;

class ConfigureMidiOut : public UI_ConfigureMidiOut
{
  Q_OBJECT

 public:
  ConfigureMidiOut(MidiOut* plugin);
  virtual ~ConfigureMidiOut();

  void setDevice(QString deviceName);
  QString device();

  unsigned char midiChannel();
  unsigned char firstNote();

  void updateStatus();

 private slots:
  void slotActivateClicked();

 private:
  MidiOut* m_plugin;
};

#endif
