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

#include <qtoolbutton.h>
#include <qlayout.h>

#include "vcdockarea.h"
#include "vcdockslider.h"
#include "bus.h"

VCDockArea::VCDockArea(QWidget* parent, const char* name)
  : QFrame(parent, name)
{
}

VCDockArea::~VCDockArea()
{
}

void VCDockArea::init()
{
  setFrameStyle(QFrame::Panel | QFrame::Raised);

  // Align widgets vertically in the area
  m_layout = new QVBoxLayout(this);
  m_layout->setMargin(2);

  // Default fade time slider
  m_defaultFadeSlider = new VCDockSlider(this, "Default Fade Slider");
  m_defaultFadeSlider->setBus(Bus::defaultFadeBus());
  m_layout->addWidget(m_defaultFadeSlider);

  // Default hold time slider
  m_defaultHoldSlider = new VCDockSlider(this, "Default Hold Slider");
  m_defaultHoldSlider->setBus(Bus::defaultHoldBus());
  m_layout->addWidget(m_defaultHoldSlider);

  // Default master slider
  m_masterSlider = new VCDockSlider(this, "Master Slider");
  m_masterSlider->setMode(VCDockSlider::Master);
  m_layout->addWidget(m_masterSlider);

  // Hide button
  m_hideButton = new QToolButton(this);
  m_hideButton->setText("Hide");
  m_layout->addWidget(m_hideButton);
  connect(m_hideButton, SIGNAL(clicked()), 
	  this, SLOT(slotHideButtonClicked()));
}

void VCDockArea::slotHideButtonClicked()
{
  hide();
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
