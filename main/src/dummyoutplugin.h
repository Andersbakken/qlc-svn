/*
  Q Light Controller
  dummyoutplugin.h
  
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

#ifndef DUMMYOUTPLUGIN_H
#define DUMMYOUTPLUGIN_H

#include "../../libs/common/outputplugin.h"
#include "dmxchannel.h"

class DummyOutPlugin : public OutputPlugin
{
  Q_OBJECT
    
    friend class ConfigureDummyOutPlugin;
  
 public:
  DummyOutPlugin(int id);
  virtual ~DummyOutPlugin();
  
  virtual bool open();
  virtual bool close();
  virtual bool isOpen();
  virtual void configure();
  virtual QString infoText();
  virtual void contextMenu(QPoint pos);
  
  virtual void setConfigDirectory(QString dir);
  virtual void saveSettings();
  virtual void loadSettings();
  
  // OutputPlugin functions
  bool writeChannel(t_channel channel, t_value value);

  void activate();
  
 protected slots:
  void slotContextMenuCallback(int);
  
 private:
  bool m_open;
};

#endif

