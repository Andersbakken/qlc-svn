/*
  Q Light Controller
  consolechannel.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef CONSOLECHANNEL_H
#define CONSOLECHANNEL_H

#include "uic_consolechannel.h"
#include "scene.h"
#include "types.h"

class QSlider;
class QLabel;
class QPushButton;
class QContextMenuEvent;
class Device;

class ConsoleChannel : public UI_ConsoleChannel
{
  Q_OBJECT

 public:
  ConsoleChannel(QWidget *parent, const char* name = NULL);
  ~ConsoleChannel();

  void setDevice(t_device_id);
  void setChannel(t_channel);

  int getSliderValue(void);
  void update(void);

  void setStatusButton(Scene::ValueType);

  Scene::ValueType status() const { return m_status; }

  // For sequence editor doesn't like fade values
  void setFadeStatusEnabled(bool enable); 
  
 signals:
  void changed(t_channel, t_value, Scene::ValueType);

 public slots:
  void slotValueChange(int);
  void slotAnimateValueChange(t_value);

  void slotSceneActivated(SceneValue* values, t_channel channels);

 private slots:
  void slotStatusButtonClicked();
  void slotSetFocus();
  void slotContextMenuActivated(int);

 protected:
  void contextMenuEvent(QContextMenuEvent*);

 private:
  void initMenu(void);
  void updateStatusButton();

  t_channel m_channel;
  t_value m_value;
  Scene::ValueType m_status;

  t_device_id m_deviceID;

  bool m_fadeStatusEnabled;

  bool m_updateOnly;
};

#endif
