/*
  Q Light Controller
  vcdocksliderproperties.cpp

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

#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <assert.h>

#include "vcdocksliderproperties.h"
#include "vcdockslider.h"
#include "types.h"
#include "bus.h"
#include "app.h"

extern App* _app;

VCDockSliderProperties::VCDockSliderProperties(VCDockSlider* parent,
					       const char* name)
  : UI_VCDockSliderProperties(_app, name, true)
{
  assert(parent);
  m_slider = parent;
}

VCDockSliderProperties::~VCDockSliderProperties()
{
}

void VCDockSliderProperties::init()
{
  //
  // Mode
  //
  m_behaviourGroup->setButton(m_slider->mode());
  slotBehaviourSelected(m_slider->mode());

  //
  // Bus elements
  //
  t_bus_value buslo, bushi;
  m_slider->busRange(buslo, bushi);
  m_lowBusValueSpin->setValue(buslo);
  m_highBusValueSpin->setValue(bushi);
  
  fillBusCombo();
}

void VCDockSliderProperties::fillBusCombo()
{
  QString s;

  m_busCombo->clear();

  for (t_bus_id i = 0; i < KBusCount; i++)
    {
      s.sprintf("%.2d:", i+1);
      s += Bus::name(i);
      m_busCombo->insertItem(s);
    }

  m_busCombo->setCurrentItem(m_slider->busID());
}

void VCDockSliderProperties::slotBehaviourSelected(int id)
{
  switch (id)
    {
    default:
    case VCDockSlider::Speed:
      m_busGroup->setEnabled(true);
      m_channelGroup->setEnabled(false);
      break;
      
    case VCDockSlider::Level:
      m_busGroup->setEnabled(false);
      m_channelGroup->setEnabled(true);

      m_lowChannelValueSpin->setEnabled(true);
      m_highChannelValueSpin->setEnabled(true);
      break;

    case VCDockSlider::Submaster:
      m_busGroup->setEnabled(false);
      m_channelGroup->setEnabled(true);

      m_lowChannelValueSpin->setEnabled(false);
      m_highChannelValueSpin->setEnabled(false);
      break;
    }

  m_mode = static_cast<VCDockSlider::Mode> (id);
}

void VCDockSliderProperties::slotAddChannelClicked()
{
}

void VCDockSliderProperties::slotRemoveChannelClicked()
{
}

void VCDockSliderProperties::slotClearAllChannelsClicked()
{
}

void VCDockSliderProperties::slotOKClicked()
{
  if (m_mode == VCDockSlider::Speed)
    {
      m_slider->setBusRange(m_lowBusValueSpin->value(), 
			    m_highBusValueSpin->value());
      m_slider->setBusID(m_busCombo->currentItem());
    }

  accept();
}

void VCDockSliderProperties::slotCancelClicked()
{
  reject();
}

