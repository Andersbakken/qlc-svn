/*
  Q Light Controller
  sequence.h
  
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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "function.h"
#include "scene.h"
#include "types.h"

class EventBuffer;
class Device;
class RunTimeData;
class SequenceValue;

typedef float t_scene_acc;

class Sequence : public Function
{
 public:
  Sequence();
  ~Sequence();

  bool copyFrom(Sequence* seq, t_device_id toDevice = KNoID);
  bool setDevice(t_device_id);

  void constructFromPointArray(const QPointArray& array,
			       t_channel horizontalChannel,
			       t_channel verticalChannel);

  t_channel channels() { return m_channels; }

  QPtrList <SceneValue> m_values;

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void busValueChanged(t_bus_id, t_bus_value);
  void speedChange(t_bus_value);

  void arm();
  void disarm();
  void cleanup();

 protected:
  void init();
  void run();

 private:
  t_channel m_channels;

  t_bus_value m_timeSpan;
  t_bus_value m_elapsedTime;

  // This mutex is locked when the run time data is accessed
  QMutex m_dataMutex;
};

#endif
