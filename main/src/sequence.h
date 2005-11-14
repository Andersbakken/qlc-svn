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

class Sequence : public Function
{
 public:
  Sequence();
  ~Sequence();

  enum RunOrder
    {
      Loop,
      SingleShot,
      PingPong
    };

  enum Direction
    {
      Forward,
      Backward
    };

  enum Advanced
    {
      SetZeroEnabled,
      SetZeroDisabled
    };

  bool copyFrom(Sequence* seq, t_device_id toDevice = KNoID);
  bool setDevice(t_device_id);

  void constructFromPointArray(const QPointArray& array,
			       t_channel horizontalChannel,
			       t_channel verticalChannel);
  
  t_channel channels() { return m_channels; }

  QPtrList <SceneValue> m_steps;

  void setRunOrder(RunOrder order) { m_runOrder = order; }
  RunOrder runOrder() const { return m_runOrder; }

  void setDirection(Direction dir) { m_direction = dir; }
  Direction direction() const { return m_direction; }
  
  void setAdvanced(Advanced adv) { m_advanced = adv; }
  Advanced advanced() const { return m_advanced; }

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void busValueChanged(t_bus_id, t_bus_value);

  void arm();
  void disarm();
  void cleanup();

 protected:
  void init();
  void run();

  void hold();

 private:
  RunOrder m_runOrder;
  Direction m_direction;
  Advanced m_advanced;

  t_value* m_channelData;
  SceneValue* m_runTimeValues;
  t_channel m_runTimeChannel;
  Direction m_runTimeDirection;

  t_bus_value m_holdTime;
  t_bus_value m_runTimeHold;
  t_value* m_holdNoSetData;
  
  // Use only at run time to prevent allocating a local variable
  t_channel m_address;
};

#endif
