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

class QPoint;
class QString;

extern "C" OutputPlugin* create(int id);
extern "C" void destroy(OutputPlugin* object);

class DMX4LinuxOut : public OutputPlugin
{
 public:
  DMX4LinuxOut(int id);
  ~DMX4LinuxOut();

  virtual bool open();
  virtual bool close();
  virtual void configure();
  virtual QString infoText();
  virtual void contextMenu(QPoint pos);

  virtual QString deviceName() { return m_deviceName; }

  virtual bool writeChannel(unsigned short channel, unsigned char value);

 private:
  QString m_deviceName;
  int m_device;
};

#endif
