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
#include <qevent.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>

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
  m_busMenu = NULL;
}

VCDockSlider::~VCDockSlider()
{
}

void VCDockSlider::slotSliderValueChanged(int value)
{
  if (Bus::setValue(m_busID, m_slider->value()))
    {
      QString num;
      num.sprintf("%.2fs", ((float) value / (float) KFrequency));
      m_valueLabel->setText(num);
    }
  else
    {
      m_valueLabel->setText("ERROR");
    }
}


bool VCDockSlider::setBusID(t_bus_id id)
{
  if (m_busMenu) delete m_busMenu;

  t_bus_value value;
  if (Bus::value(id, value))
    {
      m_slider->setValue(value);
      m_busID = id;

      //
      // Create bus menu to bus tool button
      //
      QString name;
      m_busMenu = new QPopupMenu();
      for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
	{
	  name.sprintf("%.2d: ", i);
	  name += Bus::name(i);
	  
	  m_busMenu->insertItem(name, i);
	  if (m_busID == i)
	    {
	      m_busMenu->setItemChecked(i, true);
	    }
	}
      
      m_busButton->setPopup(m_busMenu);
      connect(m_busMenu, SIGNAL(activated(int)),
	      this, SLOT(slotBusMenuActivated(int)));

      //
      // Set name label
      //
      name = Bus::name(m_busID);
      if (name == QString::null)
	{
	  name.sprintf("%.2d", id);
	}
      m_nameLabel->setText(name);

      //
      // Enable/disable some buttons
      //
      m_functionButton->setEnabled(false);
      m_busButton->setEnabled(true);

      return true;
    }
  else
    {
      m_functionButton->setEnabled(false);
      m_busButton->setEnabled(false);

      return false;
    }
}


void VCDockSlider::slotBusMenuActivated(int id)
{
  setBusID(id);
}


void VCDockSlider::slotMoveButtonPressed()
{
}

void VCDockSlider::slotMoveButtonReleased()
{
}

