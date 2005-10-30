/*
  Q Light Controller
  efx.h
  
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

#ifndef EFX_H
#define EFX_H

#include "function.h"
#include "scene.h"
#include "types.h"

class EFX : public Function
{
 public:
  EFX();
  ~EFX();

  enum Algorithm
    {
      Circle = 0,
      Eight,
      Line,
      Square,
      Triangle,
      Lissajous
    };

  bool copyFrom(EFX* efx, t_device_id toDevice = KNoID);
  bool setDevice(t_device_id);

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void busValueChanged(t_bus_id, t_bus_value);

  void arm();
  void disarm();
  void cleanup();

 protected:
  void init();
  void run();

 protected:
  void setPoint(int x, int y);
  void circle();
  void eight();
  void line();
  void square();
  void triangle();
  void lissajous();

 protected:
  double m_width;
  double m_height;
  double m_xOffset;
  double m_yOffset;
  double m_stepSize;
  double m_cycleDuration;
  double m_orientation;
  double m_omegax;
  double m_omegay;
  double m_phasex;
  double m_phasey;

 protected:
  EFX::Algorithm m_algorithm;
  t_value* m_channelData;
};

#endif
