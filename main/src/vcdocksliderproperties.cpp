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
#include <qlistview.h>
#include <qspinbox.h>
#include <assert.h>

#include "vcdocksliderproperties.h"
#include "vcdockslider.h"
#include "device.h"
#include "deviceclass.h"
#include "logicalchannel.h"
#include "types.h"
#include "bus.h"
#include "app.h"
#include "doc.h"

extern App* _app;

const int KColumnDMXChannel     ( 0 );
const int KColumnDevice         ( 1 );
const int KColumnDeviceChannel  ( 2 );

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
  // Fill elements
  //
  fillBusCombo();
  fillChannelList();

  //
  // Bus stuff
  //
  t_bus_value buslo, bushi;
  m_slider->busRange(buslo, bushi);
  m_lowBusValueSpin->setValue(buslo);
  m_highBusValueSpin->setValue(bushi);
  
  //
  // Mode
  //
  m_behaviourGroup->setButton(m_slider->mode());
  slotBehaviourSelected(m_slider->mode());

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

void VCDockSliderProperties::fillChannelList()
{
  QString s;
  t_channel ch;
  t_channel channels;
  QCheckListItem* item = NULL;

  //
  // Fill the list view
  //
  for (t_device_id i = 0; i < KDeviceArraySize; i++)
    {
      Device* d = _app->doc()->device(i);
      if (!d)
	{
	  continue;
	}
      else
	{
	  channels = (t_channel) d->deviceClass()->channels()->count();

	  for (ch = 0; ch < channels; ch++)
	    {
	      // DMX Channel
	      s.sprintf("%.3d", d->address() + ch + 1);
	      item = new QCheckListItem(m_channelList, s, 
					QCheckListItem::CheckBoxController);
	      
	      // Device name
	      item->setText(KColumnDevice, d->name());

	      // Device channel
	      s.sprintf("%.3d:" + 
			d->deviceClass()->channels()->at(ch)->name(), ch + 1);
	      item->setText(KColumnDeviceChannel, s);
	    }
	}
    }

  //
  // Check selected items
  //
  QValueList <t_channel>::iterator it;
  for (it = m_slider->channels()->begin();
       it != m_slider->channels()->end(); ++it)
    {
      s.sprintf("%.3d", (*it) + 1);
      item = static_cast<QCheckListItem*> (m_channelList
					   ->findItem(s, KColumnDMXChannel));
      if (item)
	{
	  item->setState(QCheckListItem::On);
	}
    }
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

void VCDockSliderProperties::slotOKClicked()
{
  //
  // Resign previous submasters, if any
  //
  if (m_slider->mode() == VCDockSlider::Submaster)
    {
      m_slider->assignSubmasters(false);
    }

  //
  // Check new mode
  //
  if (m_mode == VCDockSlider::Speed)
    {
      m_slider->setBusRange(m_lowBusValueSpin->value(), 
			    m_highBusValueSpin->value());
      m_slider->setBusID(m_busCombo->currentItem());
    }
  else if (m_mode == VCDockSlider::Level)
    {
      QCheckListItem* item = NULL;
      t_channel ch = 0;

      //
      // Clear channel list
      //
      m_slider->channels()->clear();
      
      //
      // Then, add the new level channels
      //
      QListViewItemIterator it(m_channelList);
      while (it.current())
	{
	  item = static_cast<QCheckListItem*> (it.current());

	  ch = static_cast<t_channel> 
	    (item->text(KColumnDMXChannel).toInt() - 1);

	  if (item->isOn() && 
	      m_slider->channels()->find(ch) == m_slider->channels()->end())
	    {
	      m_slider->channels()->append(ch);
	    }

	  ++it;
	}
    }
  else if (m_mode == VCDockSlider::Submaster)
    {
      QCheckListItem* item = NULL;
      t_channel ch = 0;

      //
      // Clear channel list
      //
      m_slider->channels()->clear();

      //
      // Then, add the new submaster channels
      //
      QListViewItemIterator it(m_channelList);
      while (it.current())
	{
	  item = static_cast<QCheckListItem*> (it.current());

	  ch = static_cast<t_channel> 
	    (item->text(KColumnDMXChannel).toInt() - 1);

	  if (item->isOn() && 
	      m_slider->channels()->find(ch) == m_slider->channels()->end())
	    {
	      m_slider->channels()->append(ch);
	    }

	  ++it;
	}
      
      //
      // Assign submasters
      //
      m_slider->assignSubmasters(true);
    }
  
  //
  // Set the actual mode last
  //
  m_slider->setMode(m_mode);

  //
  // Reset all non-assigned submaster channels back to 100%
  //
  _app->resetSubmasters();
  
  accept();
}

void VCDockSliderProperties::slotCancelClicked()
{
  reject();
}

