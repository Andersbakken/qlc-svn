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
#include "functionconsumer.h"

extern App* _app;

VCDockSlider::VCDockSlider(QWidget* parent, const char* name)
  : UI_VCDockSlider(parent, name)
{
  m_busID = KBusIDInvalid;
}

VCDockSlider::~VCDockSlider()
{
}

void VCDockSlider::slotSliderValueChanged(int value)
{
  QString num;

  switch (m_mode)
    {
    case Normal:
      break;

    case Speed:
      num.sprintf("%.2fs", ((float) value / (float) KFrequency));
      m_valueLabel->setText(num);
      Bus::setValue(m_busID, m_slider->value());
      break;

    case Master:
      //_app->doc()->outputPlugin()->setMaster(KChannelValueMax - value);
      break;
    }
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
	if (m_busID == KBusIDDefaultFade)
	  {
	    m_nameLabel->setText("Fade");
	    m_infoLabel->setText("Speed");
	  }
	else if (m_busID == KBusIDDefaultHold)
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

void VCDockSlider::setBusID(t_bus_id id)
{
  m_busID = id;
  setMode(Speed);

  t_bus_value value;
  Bus::value(m_busID, value);
  m_slider->setValue(value);
}

