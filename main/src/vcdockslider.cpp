/*
  Q Light Controller
  vcslider.cpp

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

#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>

#include "vcdockslider.h"
#include "types.h"
#include "bus.h"
#include "app.h"
#include "doc.h"
#include "../../libs/common/outputplugin.h"

extern App* _app;

VCDockSlider::VCDockSlider(QWidget* parent, const char* name)
  : UI_VCDockSlider(parent, name)
{
  m_bus = NULL;
  m_slider->setTracking(false);
}

VCDockSlider::~VCDockSlider()
{
}

void VCDockSlider::slotSliderValueChanged(int value)
{
  QString num;
  num.sprintf("%.3d", KChannelValueMax - value);
  m_valueLabel->setText(num);

  switch (m_mode)
    {
    case Normal:
      break;

    case Speed:
      m_bus->setValue(KChannelValueMax - value);
      break;

    case Master:
      //_app->doc()->outputPlugin()->setMaster(KChannelValueMax - value);
      break;
    }
}


void VCDockSlider::slotBusDestroyed(t_bus_id)
{
  m_bus = Bus::defaultFadeBus();
}


void VCDockSlider::setMode(Mode mode)
{
  m_mode = mode;

  switch (mode)
    {
    default:
    case Normal:
      break;
      
    case Speed:
      {
	ASSERT(m_bus);
	if (m_bus == Bus::defaultFadeBus())
	  {
	    m_nameLabel->setText("Fade");
	    m_infoLabel->setText("Speed");
	  }
	else if (m_bus == Bus::defaultHoldBus())
	  {
	    m_nameLabel->setText("Hold");
	    m_infoLabel->setText("Speed");
	  }
      }
      break;

    case Master:
      m_nameLabel->setText("Master");
      m_infoLabel->setText("Level");
      break;
    }
}

void VCDockSlider::setBus(Bus* bus)
{
  if (m_bus)
    {
      // Disconnect old bus
      disconnect(m_bus, SIGNAL(destroyed(t_bus_id)),
		 this, SLOT(slotBusDestroyed(t_bus_id)));
    }

  // Take new bus
  m_bus = bus;

  if (bus)
    {
      // Connect new bus
      connect(m_bus, SIGNAL(destroyed(t_bus_id)),
	      this, SLOT(slotBusDestroyed(t_bus_id)));

      // Since a bus was set to this slider, we can
      // make it a speed slider
      setMode(Speed);
    }
}

