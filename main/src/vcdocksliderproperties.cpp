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
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qlineedit.h>
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
#include "sliderkeybind.h"
#include "assignsliderhotkey.h"

extern App* _app;

const int KColumnDevice         ( 0 );
const int KColumnDeviceChannel  ( 1 );
const int KColumnDMXChannel     ( 2 );

VCDockSliderProperties::VCDockSliderProperties(VCDockSlider* parent,
					       const char* name)
  : UI_VCDockSliderProperties(_app, name, true)
{
  assert(parent);
  m_slider = parent;
  m_sliderKeyBind = new SliderKeyBind(parent->sliderKeyBind());
}

VCDockSliderProperties::~VCDockSliderProperties()
{
  delete m_sliderKeyBind;
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
  // Level stuff
  //
  t_value levello, levelhi;
  m_slider->levelRange(levello, levelhi);
  m_lowChannelValueSpin->setValue(levello);
  m_highChannelValueSpin->setValue(levelhi);

  //
  // Mode
  //
  m_behaviourGroup->setButton(m_slider->mode());
  slotBehaviourSelected(m_slider->mode());

  //
  // Slider key bind
  //
  QString keyStringUp;
  QString keyStringDown;

  m_sliderKeyBind->keyStringUp(keyStringUp);
  m_sliderKeyBind->keyStringDown(keyStringDown);
  m_keyUpEdit->setText(keyStringUp);
  m_keyDownEdit->setText(keyStringDown);


  //
  // Midi stuff
  //
  m_channelSpinBox->setValue(m_slider->channel());

  //
  // Pixmaps
  //
  m_attachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/key_bindings.png")));
  m_detachKey->setPixmap(QPixmap(QString(PIXMAPS) + QString("/keyboard.png")));

}


//
// Fill bus combo with available buses
//
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

  //
  // Select the bus that has been assigned to the slider
  //
  m_busCombo->setCurrentItem(m_slider->busID());
}


//
// Initialize the channel list view for level & submaster modes
//
void VCDockSliderProperties::fillChannelList()
{
  QString s;
  t_channel ch;
  t_channel channels;
  QCheckListItem* item = NULL;

  //
  // Fill the list view with available channels. Put only
  // those channels that are occupied by existing devices.
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
		// Device name
		item = new QCheckListItem(m_channelList, d->name(),
		    QCheckListItem::CheckBox);

		// Device channel
		s.sprintf("%.3d:" +
		   d->deviceClass()->channels()->at(ch)->name(), ch + 1);
		item->setText(KColumnDeviceChannel, s);

		// DMX & Universe Channel
		s.sprintf("%d", d->universeAddress() + ch);
		item->setText(KColumnDMXChannel, s);
	    }
	}
    }

  //
  // Check those channels that are found from slider's channel list
  //
  QValueList <t_channel>::iterator it;
  for (it = m_slider->channels()->begin();
       it != m_slider->channels()->end(); ++it)
    {
      s.sprintf("%d", (*it));
      item = static_cast<QCheckListItem*> (m_channelList
				   ->findItem(s, KColumnDMXChannel));
      if (item)
	{
	  item->setOn(true);
	}
    }
}


//
// Bus, Level or Submaster radio has been clicked
//
void VCDockSliderProperties::slotBehaviourSelected(int id)
{
  switch (id)
	{
	default:
	case VCDockSlider::Speed:
		m_busGroup->setEnabled(true);
		m_channelGroup->setEnabled(false);

		m_allChannels->setEnabled(false);
		m_invertChannels->setEnabled(false);
		m_clearChannels->setEnabled(false);
		m_deviceChannels->setEnabled(false);
		m_roleChannels->setEnabled(false);

		m_lowChannelValueSpin->setEnabled(false);
		m_highChannelValueSpin->setEnabled(false);
	break;

	case VCDockSlider::Level:
		m_busGroup->setEnabled(false);
		m_channelGroup->setEnabled(true);

		m_allChannels->setEnabled(true);
		m_invertChannels->setEnabled(true);
		m_clearChannels->setEnabled(true);
		m_deviceChannels->setEnabled(true);
		m_roleChannels->setEnabled(true);

		m_lowChannelValueSpin->setEnabled(true);
		m_highChannelValueSpin->setEnabled(true);
	break;

	case VCDockSlider::Submaster:
		m_busGroup->setEnabled(false);
		m_channelGroup->setEnabled(true);

		m_allChannels->setEnabled(true);
		m_invertChannels->setEnabled(true);
		m_clearChannels->setEnabled(true);
		m_deviceChannels->setEnabled(true);
		m_roleChannels->setEnabled(true);

		m_lowChannelValueSpin->setEnabled(false);
		m_highChannelValueSpin->setEnabled(false);
	break;
    }

  m_mode = static_cast<VCDockSlider::Mode> (id);
}

/**
 * Select all channels
 */
void VCDockSliderProperties::slotAllChannelsClicked()
{
	QListViewItemIterator it(m_channelList);
	while (it.current())
	{
		static_cast<QCheckListItem*> (it.current())->setOn(true);
		it++;
	}
}

/**
 * Invert selection
 */
void VCDockSliderProperties::slotInvertChannelsClicked()
{
	QListViewItemIterator it(m_channelList);
	while (it.current())
	{
		static_cast<QCheckListItem*> (it.current())->setOn(
		    !(static_cast<QCheckListItem*> (it.current())->isOn()));
		it++;
	}
}

/**
 * Clear selection
 */
void VCDockSliderProperties::slotClearChannelsClicked()
{
	QListViewItemIterator it(m_channelList);
	while (it.current())
	{
		static_cast<QCheckListItem*> (it.current())->setOn(false);
		it++;
	}
}

/**
 * Select all channels from the selected device
 */
void VCDockSliderProperties::slotDeviceChannelsClicked()
{
	QCheckListItem* item;
	QListViewItemIterator it(m_channelList->currentItem());
	QString name;

	if (it.current())
	{
		name = it.current()->text(KColumnDevice);
	}

	while (it.current() && it.current()->text(KColumnDevice) == name)
	{
		item = static_cast<QCheckListItem*> (it.current());
		item->setOn(!item->isOn());
		it++;
	}
}

/**
 * Select channels whose roles (names) match with the selected channel
 */
void VCDockSliderProperties::slotRoleChannelsClicked()
{
	QCheckListItem* item;
	QListViewItemIterator it(m_channelList->currentItem());
	QString role;
	QString itrole;

	if (it.current())
	{
		role = it.current()->text(KColumnDeviceChannel);
		role = role.right(role.length() - role.find(':', 0, FALSE) - 1);
		qDebug(role);
	}

	while (it.current())
	{
		item = static_cast<QCheckListItem*> (it.current());
		itrole = item->text(KColumnDeviceChannel);
		itrole = itrole.right(itrole.length() - itrole.find(':', 0, FALSE) - 1);

		if (itrole == role)
		{
			item->setOn(!item->isOn());
		}

		it++;
	}
}

void VCDockSliderProperties::slotAttachKeyClicked()
{
  QString keyStringUp;
  QString keyStringDown;

  AssignSliderHotKey* a = new AssignSliderHotKey(this);

  if (a->exec() == QDialog::Accepted)
    {
      assert(a->sliderKeyBind());

      if (m_sliderKeyBind)
        {
	  delete m_sliderKeyBind;
	}

      m_sliderKeyBind = new SliderKeyBind(a->sliderKeyBind());
      m_sliderKeyBind->keyStringUp(keyStringUp);
      m_keyUpEdit->setText(keyStringUp);
      m_sliderKeyBind->keyStringDown(keyStringDown);
      m_keyDownEdit->setText(keyStringDown);
    }

  delete a;
}

void VCDockSliderProperties::slotDetachKeyClicked()
{
  QString keyStringUp;
  QString keyStringDown;

  m_sliderKeyBind->setKeyUp(Key_unknown);
  m_sliderKeyBind->setModUp(NoButton);
  m_sliderKeyBind->setKeyDown(Key_unknown);
  m_sliderKeyBind->setModDown(NoButton);

  m_sliderKeyBind->keyStringUp(keyStringUp);
  m_keyUpEdit->setText(keyStringUp);
  m_sliderKeyBind->keyStringDown(keyStringDown);
  m_keyDownEdit->setText(keyStringDown);
}

//
// Accept changes
//
void VCDockSliderProperties::slotOKClicked()
{
  //
  // Set the keyBinds
  //
  m_slider->setSliderKeyBind(m_sliderKeyBind);
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
      //
      // Extract selected channels from channel list
      //
     extractChannels();

     //
     // Set range
     //
     m_slider->setLevelRange(m_lowChannelValueSpin->value(),
			     m_highChannelValueSpin->value());
    }
  else if (m_mode == VCDockSlider::Submaster)
    {
      //
      // Extract selected channels from channel list
      //
      extractChannels();

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

  m_slider->setChannel(m_channelSpinBox->value());

  accept();
}


//
// Get selected channels from channel list view and set them
// to the slider's channel list. Used for both level & submaster modes.
//
void VCDockSliderProperties::extractChannels()
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
	(item->text(KColumnDMXChannel).toInt());

      if (item->isOn() &&
	  m_slider->channels()->find(ch) == m_slider->channels()->end())
	{
	  m_slider->channels()->append(ch);
	}

      ++it;
    }
}


//
// Don't save changes
//
void VCDockSliderProperties::slotCancelClicked()
{
  reject();
}

