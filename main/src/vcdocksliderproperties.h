/*
  Q Light Controller
  vcdocksliderproperties.h

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

#ifndef VCDOCKSLIDERPROPERTIES_H
#define VCDOCKSLIDERPROPERTIES_H

#include "uic_vcdocksliderproperties.h"
#include "vcdockslider.h"

class VCDockSliderProperties : public UI_VCDockSliderProperties
{
  Q_OBJECT

    public:
  VCDockSliderProperties(VCDockSlider* parent, const char* name = NULL);
  ~VCDockSliderProperties();

  void init();

 protected:
  void fillBusCombo();
  void fillChannelList();

  void extractChannels();

 public slots:
  void slotBehaviourSelected(int);
  void slotAllChannelsClicked();
  void slotInvertChannelsClicked();
  void slotClearChannelsClicked();
  void slotDeviceChannelsClicked();
  void slotRoleChannelsClicked();

  void slotOKClicked();
  void slotCancelClicked();

 protected:
  VCDockSlider* m_slider;
  VCDockSlider::Mode m_mode;
};

#endif
