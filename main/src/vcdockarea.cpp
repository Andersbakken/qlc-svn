/*
  Q Light Controller
  vcdockarea.cpp

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

#include <qpushbutton.h>
#include <qlayout.h>

#include "vcdockarea.h"
#include "vcdockslider.h"
#include "bus.h"
#include "app.h"
#include "settings.h"

extern App* _app;

VCDockArea::VCDockArea(QWidget* parent, const char* name)
  : QFrame(parent, name)
{
}

VCDockArea::~VCDockArea()
{
}

void VCDockArea::init()
{
  t_bus_value min, max;
  QString value;

  // Align widgets vertically in the area
  m_layout = new QVBoxLayout(this);

  // Default fade time slider
  m_defaultFadeSlider = new VCDockSlider(this, true, "Default Fade Slider");
  m_defaultFadeSlider->init();
  m_defaultFadeSlider->setBusID(KBusIDDefaultFade);

  // Get value range
  if (_app->settings()->get(KEY_DEFAULT_FADE_MIN, value) != -1
	&& value != "")
    {
      min = value.toInt();
    }
  else
    {
      min = 0;
    }

  if (_app->settings()->get(KEY_DEFAULT_FADE_MAX, value) != -1
	&& value != "")
    {
      max = value.toInt();
    }
  else
    {
      max = 5;
    }

  // If there are bogus values, default to 0-5
  if (min >= max)
    {
      min = 0;
      max = 5;
    }

  // Set value range
  m_defaultFadeSlider->setBusRange(min, max);

  // Mode and add widget
  m_defaultFadeSlider->setMode(VCDockSlider::Speed);
  m_layout->addWidget(m_defaultFadeSlider);

  //
  // Default hold time slider
  //
  m_defaultHoldSlider = new VCDockSlider(this, true, "Default Hold Slider");
  m_defaultHoldSlider->init();
  m_defaultHoldSlider->setBusID(KBusIDDefaultHold);

  // Get value range
  if (_app->settings()->get(KEY_DEFAULT_HOLD_MIN, value) != -1
	&& value != "")
    {
      min = value.toInt();
    }
  else
    {
      min = 0;
    }

  if (_app->settings()->get(KEY_DEFAULT_HOLD_MAX, value) != -1
	&& value != "")
    {
      max = value.toInt();
    }
  else
    {
      max = 5;
    }

  // If there are bogus values, default to 0-5
  if (min >= max)
    {
      min = 0;
      max = 5;
    }

  // Set value range
  m_defaultHoldSlider->setBusRange(min, max);

  // Mode and add widget
  m_defaultHoldSlider->setMode(VCDockSlider::Speed);
  m_layout->addWidget(m_defaultHoldSlider);
}

void VCDockArea::hide()
{
  emit areaHidden(true);

  QFrame::hide();
}

void VCDockArea::show()
{
  emit areaHidden(false);

  QFrame::show();
}
