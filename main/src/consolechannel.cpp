/*
  Q Light Controller
  consolechannel.cpp
  
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

#include <stdlib.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <assert.h>
#include <qevent.h>
#include <qpopupmenu.h>
#include <qptrlist.h>

#include "consolechannel.h"
#include "app.h"
#include "doc.h"
#include "settings.h"
#include "scene.h"
#include "device.h"
#include "logicalchannel.h"
#include "capability.h"
#include "configkeys.h"

extern App* _app;

const QColor KStatusButtonColorFade  (  80, 151, 222 );
const QColor KStatusButtonColorSet   ( 222,  80,  82 );
const QColor KStatusButtonColorOff   (  28,  52,  77 );

const int KMenuTitle          ( INT_MAX );

ConsoleChannel::ConsoleChannel(QWidget* parent, const char* name) 
  : UI_ConsoleChannel(parent, name),
    m_channel  (           0 ),
    m_value    (           0 ),
    m_status   ( Scene::Fade ),
    m_deviceID (       KNoID )
{
  m_statusButton->setBackgroundMode(FixedColor);

  connect(m_valueSlider, SIGNAL(valueChanged(int)), 
	  this, SLOT(slotValueChange(int)));

  connect(m_valueSlider, SIGNAL(sliderPressed()),
	  this, SLOT(slotSetFocus()));

  connect(m_statusButton, SIGNAL(clicked()),
	  this, SLOT(slotStatusButtonClicked()));
}

ConsoleChannel::~ConsoleChannel()
{
}

void ConsoleChannel::setDevice(t_device_id id)
{
  m_deviceID = id;
  assert(_app->doc()->device(id));
}

void ConsoleChannel::setChannel(t_channel channel)
{
  QString num;

  m_channel = channel;

  num.sprintf("%.3d", channel + 1);
  m_numberLabel->setText(num);

  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  QToolTip::add(this, device->deviceClass()->channels()->at(channel)->name());

  updateStatusButton();
}

void ConsoleChannel::setStatusButton(Scene::ValueType status)
{
  m_status = status;
  updateStatusButton();
}

void ConsoleChannel::slotStatusButtonClicked()
{
  m_status = (Scene::ValueType) ((m_status + 1) % 3);
  updateStatusButton();

  emit changed(m_channel, m_value, m_status);
}

void ConsoleChannel::updateStatusButton()
{
  QString dir;
  _app->settings()->get(KEY_SYSTEM_DIR, dir);
  dir += QString("/") + PIXMAPPATH;

  if (m_status == Scene::Fade)
    {
      m_statusButton->setPaletteBackgroundColor(KStatusButtonColorFade);
      m_statusButton->setText("F");
      QToolTip::add(m_statusButton,
		    "Fade; Scene will fade this channel's value");
    }
  else if (m_status == Scene::Set)
    {
      m_statusButton->setPaletteBackgroundColor(KStatusButtonColorSet);
      m_statusButton->setText("S");
      QToolTip::add(m_statusButton, 
		    "Set; Scene will set this channel's value instantly");
    }
  else
    {
      m_statusButton->setPaletteBackgroundColor(KStatusButtonColorOff);
      m_statusButton->setText("X");
      QToolTip::add(m_statusButton, 
		    "Off; Scene will not set this channel's value");
    }
}

void ConsoleChannel::slotSetFocus()
{
  t_value value = 0;

  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  // In case someone else has set the value for this channel, animate
  // the slider to the correct position
  _app->outputPlugin()->readChannel(device->address() + m_channel, value);
  slotAnimateValueChange(value);

  // Set focus to this slider
  m_valueSlider->setFocus();
}

void ConsoleChannel::update()
{
  t_value value = 0;

  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  _app->outputPlugin()->readChannel(device->address() + m_channel, value);

  m_valueLabel->setNum(value);
  slotAnimateValueChange(value);
}

void ConsoleChannel::slotValueChange(int value)
{
  value = KChannelValueMax - value;

  Device* device = _app->doc()->device(m_deviceID);
  assert(device);

  _app->outputPlugin()->writeChannel(device->address() + m_channel, 
				     (t_value) value);
  m_valueLabel->setNum(value);

  m_value = value;
  emit changed(m_channel, m_value, m_status);
}

int ConsoleChannel::getSliderValue()
{
  return KChannelValueMax - m_valueSlider->value();
}

// This slot emulates the user dragging the value slider
void ConsoleChannel::slotAnimateValueChange(t_value value)
{
  m_valueSlider->setValue(static_cast<int> (KChannelValueMax - value));
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
  QString s;
  QString t;
  Capability* c = NULL;

  Device* device = _app->doc()->device(m_deviceID);
  assert(device);
  
  LogicalChannel* ch = device->deviceClass()->channels()->at(m_channel);
  
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem(ch->name(), KMenuTitle);
  menu->setItemEnabled(KMenuTitle, false);
  menu->insertSeparator();

  QPtrListIterator<Capability> it(*ch->capabilities());

  QPopupMenu* valueMenu = NULL;

  while (it.current())
    {
      c = it.current();

      // Set the value range and name as menu item's name
      s.sprintf("%.3d - %.3d:", c->lo(), c->hi());
      s += c->name();

      // Create a submenu for ranges that contain more than one value
      if (c->hi() - c->lo() > 0)
	{
	  valueMenu = new QPopupMenu(menu);
	  connect(valueMenu, SIGNAL(activated(int)),
		  this, SLOT(slotContextMenuActivated(int)));

	  for (int i = c->lo(); i <= c->hi(); i++)
	    {
	      t.sprintf("%.3d", i);
	      valueMenu->insertItem(t, i);
	    }
	  menu->insertItem(s, valueMenu);
	}
      else
	{
	  // Just one value in this range, don't create a submenu
	  menu->insertItem(s, c->lo());
	}

      ++it;
    }
  
  connect(menu, SIGNAL(activated(int)),
	  this, SLOT(slotContextMenuActivated(int)));

  menu->exec(e->globalPos()); // Synchronous call

  delete menu; // QT deletes also submenus automatically

  e->accept();
}

void ConsoleChannel::slotContextMenuActivated(int value)
{
  if (value == KMenuTitle)
    {
      return;
    }
  else
    {
      // The menuitem contains a valid DMX value
      slotAnimateValueChange(value);
    }
}

void ConsoleChannel::slotSceneActivated(SceneValue* values,
					t_channel channels)
{
  assert(values);
  
  if (m_channel <= channels)
    {
      setStatusButton(values[m_channel].type);
      
      if (values[m_channel].type == Scene::Set || 
	  values[m_channel].type == Scene::Fade)
	{
	  slotAnimateValueChange(values[m_channel].value);
	}
    }
}
