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

#include <qlist.h>
#include <qstring.h>

class ConfigureDMX4LinuxOut;
class QPoint;

extern "C" OutputPlugin* create(int id);
extern "C" void destroy(OutputPlugin* object);

class DMX4LinuxOut : public OutputPlugin
{
  Q_OBJECT

  friend class ConfigureDMX4LinuxOut;

 public:
  DMX4LinuxOut(int id);
  ~DMX4LinuxOut();

  // Plugin methods
  virtual bool open();
  virtual bool close();
  virtual bool isOpen();
  virtual void configure();
  virtual QString infoText();
  virtual void contextMenu(QPoint pos);

  virtual void setConfigDirectory(QString dir);
  virtual void saveSettings();
  virtual void loadSettings();

  // OutputPlugin methods
  virtual bool writeChannel(unsigned short channel, unsigned char value);

  // Own methods
  virtual QString deviceName() { return m_deviceName; }
  virtual void setDeviceName(QString name) { m_deviceName = name; }

 private slots:
  void slotContextMenuCallback(int item);

 private:
  void activate();
  void createContents(QList <QString> &list);

 private:
  QString m_deviceName;
  QString m_configDir;
  int m_device;
};

#endif
