/*
  Q Light Controller
  vcdockslider.h

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include "uic_vcdockslider.h"
#include "types.h"

class QMouseEvent;
class QPopupMenu;

class VCDockSlider : public UI_VCDockSlider
{
  Q_OBJECT
    
 public:
  VCDockSlider(QWidget* parent, const char* name = NULL);
  ~VCDockSlider();
  
  enum Mode
    {
      Normal = 0,
      Speed  = 1,
      Master = 2
    };
      

  bool setBusID(t_bus_id id);

 private slots:
  void slotSliderValueChanged(int);
  void slotBusMenuActivated(int id);

  void slotMoveButtonPressed();
  void slotMoveButtonReleased();

 private:
  Mode m_mode;
  t_bus_id m_busID;

  QPopupMenu* m_busMenu;
};

#endif

