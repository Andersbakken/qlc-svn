/*
  Q Light Controller
  vcframeproperties.cpp
  
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

#include <assert.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "vcframe.h"
#include "vcframeproperties.h"

VCFrameProperties::VCFrameProperties(QWidget* parent, const char* name)
  : UI_VCFrameProperties(parent, name, true)
{
  assert(parent);
  m_frame = static_cast<VCFrame*> (parent);
}

VCFrameProperties::~VCFrameProperties()
{
}

void VCFrameProperties::init()
{
  if (m_frame->buttonBehaviour() == VCFrame::Exclusive)
    {
      m_behaviourGroup->setButton(1);
    }
  else
    {
      m_behaviourGroup->setButton(0);
    }
}

void VCFrameProperties::slotOKClicked()
{
  if (m_behaviourGroup->selected() == static_cast<QButton*> (m_exclusive))
    {
      m_frame->setButtonBehaviour(VCFrame::Exclusive);
    }
  else
    {
      m_frame->setButtonBehaviour(VCFrame::Normal);
    }

  accept();
}

void VCFrameProperties::slotCancelClicked()
{
  reject();
}
