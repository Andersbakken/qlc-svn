/*
  Q Light Controller
  scene.h
  
  Copyright (C) 2004 Heikki Junnila
  
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

#ifndef SCENE_H
#define SCENE_H

#include "types.h"
#include "function.h"

class EventBuffer;
class Device;
class RunTimeData;
class SceneValue;

class Scene : public Function
{
 public:
  Scene(t_function_id id = 0);
  ~Scene();

  enum ValueType
    {
      Set   = 0, // Normal value
      Fade  = 1, // Fade value
      NoSet = 2  // Ignored value
    };

  bool copyFrom(Scene* sc);
  bool setDevice(Device* device);

  t_channel channels() { return m_channels; }

  bool set(t_channel ch, t_value value, ValueType type);
  SceneValue channelValue(t_channel ch);
  QString valueTypeString(t_channel ch);

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void speedChange(t_bus_value);
  void busValueChanged(t_bus_id, t_bus_value);
  void stop();
  void freeRunTimeData();

 protected:
  void init();
  void run();

 private:
  t_channel m_channels;

  SceneValue* m_values;

  RunTimeData* m_runTimeData;
  t_value* m_channelData;
  time_t m_elapsedTime;
};

class RunTimeData
{
 public:
  float start;
  float current;
  float increment;
};

class SceneValue
{
 public:
  Scene::ValueType type;
  t_value value;
};

#endif
