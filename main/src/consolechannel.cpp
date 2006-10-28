/*
  Q Light Controller
  consolechannel.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Versio 2 as published by the Free Software Foundation.

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
#include "common/settings.h"
#include "scene.h"
#include "device.h"
#include "logicalchannel.h"
#include "capability.h"
#include "configkeys.h"

extern App* _app;

//
// Status button's colors
//
const QColor KStatusButtonColorFade  (  80, 151, 222 );
const QColor KStatusButtonColorSet   ( 222,  80,  82 );
const QColor KStatusButtonColorOff   (  28,  52,  77 );

//
// The preset menu title
//
const int KMenuTitle          ( INT_MAX );

//
// Constructor
//
ConsoleChannel::ConsoleChannel(QWidget* parent, t_device_id deviceID,
						t_channel channel)
	: UI_ConsoleChannel(parent, "ConsoleChannel"),

	m_channel    ( channel ),
	m_value      ( 0 ),
	m_status     ( Scene::Fade ),
	m_deviceID   ( deviceID ),
	m_fadeStatusEnabled ( true ),
	m_updateOnly ( false ),
	m_menu       ( NULL )
{
	assert(deviceID != KNoID);
	assert(channel != KChannelInvalid);
}

//
// Destructor
//
ConsoleChannel::~ConsoleChannel()
{
	if (m_menu)
	{
		delete m_menu;
	}
}

//
// Initialize the UI
//
void ConsoleChannel::init()
{
	Device* device = NULL;
	QString num;

	// Check that we have an actual device
	device = _app->doc()->device(m_deviceID);
	assert(device);
	
	// Check that the given channel is valid
	assert(m_channel < device->deviceClass()->channels()->count());
	
	// Set the channel's name as the tooltip
	QToolTip::add(this, 
		device->deviceClass()->channels()->at(m_channel)->name());

	// Set channel label
	num.sprintf("%.3d", m_channel + 1);
	m_numberLabel->setText(num);
	
	m_statusButton->setBackgroundMode(FixedColor);

	connect(m_valueSlider, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChange(int)));

	connect(m_valueSlider, SIGNAL(sliderPressed()),
		this, SLOT(slotSetFocus()));

	connect(m_statusButton, SIGNAL(clicked()),
		this, SLOT(slotStatusButtonClicked()));

	updateStatusButton();

	// Initialize the preset menu
	initMenu();
}

//
// Initialize the preset menu
//
void ConsoleChannel::initMenu()
{
	Capability* c = NULL;
	LogicalChannel* ch = NULL;
	Device* device = NULL;
	QPopupMenu* valueMenu = NULL;
	QString s;
	QString t;
	
	device = _app->doc()->device(m_deviceID);
	assert(device);

	ch = device->deviceClass()->channels()->at(m_channel);
	assert(ch);
	
	// Get rid of a possible previous menu
	if (m_menu)
	{
		delete m_menu;
	}
	
	// Create a popup menu and set the channel name as the title
	m_menu = new QPopupMenu();
	m_menu->insertItem(ch->name(), KMenuTitle);
	m_menu->setItemEnabled(KMenuTitle, false);
	m_menu->insertSeparator();

	QPtrListIterator<Capability> it(*ch->capabilities());

	while (it.current())
	{
		c = it.current();

		// Set the value range and name as menu item's name
		s.sprintf("%s: %.3d - %.3d", (const char*) c->name(), 
						c->lo(), c->hi());

		// Create submenu for ranges that contain more than one value
		if (c->hi() - c->lo() > 0)
		{
			valueMenu = new QPopupMenu(m_menu);
			
			connect(valueMenu, SIGNAL(activated(int)),
				this, SLOT(slotContextMenuActivated(int)));

			for (int i = c->lo(); i <= c->hi(); i++)
			{
				t.sprintf("%.3d", i);
				valueMenu->insertItem(t, i);
			}
			
			m_menu->insertItem(s, valueMenu);
		}
		else
		{
			// Just one value in this range, don't create submenu
			m_menu->insertItem(s, c->lo());
		}

		++it;
	}

	// Connect menu item activation signal to this
	connect(m_menu, SIGNAL(activated(int)),
		this, SLOT(slotContextMenuActivated(int)));
	
	// Set the menu also as the preset button's popup menu
	m_presetButton->setPopup(m_menu);
}

//
// Set the status button's.. well.. status
//
void ConsoleChannel::setStatusButton(Scene::ValueType status)
{
	m_status = status;
	updateStatusButton();
}

//
// Sequence editor doesn't like fade values
//
void ConsoleChannel::setFadeStatusEnabled(bool enable)
{
	m_fadeStatusEnabled = enable;
	if (m_status == Scene::Fade)
	{
		setStatusButton(Scene::Set);
	}

	updateStatusButton();
}

//
// Status button was clicked by the user
//
void ConsoleChannel::slotStatusButtonClicked()
{
	if (m_fadeStatusEnabled)
	{
		m_status = (Scene::ValueType) ((m_status + 1) % 3);
	}
	else
	{
		if (m_status == Scene::Set)
		{
			m_status = Scene::NoSet;
		}
		else
		{
			m_status = Scene::Set;
		}
	}

	updateStatusButton();

	emit changed(m_channel, m_value, m_status);
}

//
// Update the button's color, label and tooltip to reflect channel status
//
void ConsoleChannel::updateStatusButton()
{
	if (m_status == Scene::Fade)
	{
		m_statusButton->setPaletteBackgroundColor(
						KStatusButtonColorFade);
		m_statusButton->setText("F");
		QToolTip::add(m_statusButton, "Fade");
	}
	else if (m_status == Scene::Set)
	{
		m_statusButton->setPaletteBackgroundColor(
						KStatusButtonColorSet);
		m_statusButton->setText("S");
		QToolTip::add(m_statusButton, "Set");
	}
	else
	{
		m_statusButton->setPaletteBackgroundColor(
						KStatusButtonColorOff);
		m_statusButton->setText("X");
		QToolTip::add(m_statusButton, "Off");
	}
}

//
// Set channel's focus
//
void ConsoleChannel::slotSetFocus()
{
	t_value value = 0;

	Device* device = _app->doc()->device(m_deviceID);
	assert(device);

	// In case someone else has set the value for this channel, animate
	// the slider to the correct position
	value = _app->value(device->universeAddress() + m_channel);

	slotAnimateValueChange(value);

	// Set focus to this slider
	m_valueSlider->setFocus();
}

//
// Update the UI to match the channel's real status & value
//
void ConsoleChannel::update()
{
	t_value value = 0;
	
	Device* device = _app->doc()->device(m_deviceID);
	assert(device);
	
	value = _app->value(device->universeAddress() + m_channel);
	
	m_valueLabel->setNum(value);
	slotAnimateValueChange(value);
}

//
// Slider value was changed
//
void ConsoleChannel::slotValueChange(int value)
{
	value = KChannelValueMax - value;

	Device* device = _app->doc()->device(m_deviceID);
	assert(device);
	
	_app->setValue(device->universeAddress() + m_channel, (t_value) value);
	
	m_valueLabel->setNum(value);
	
	m_value = value;
	emit changed(m_channel, m_value, m_status);
}

//
// Get the channel's value
//
int ConsoleChannel::getSliderValue()
{
	return KChannelValueMax - m_valueSlider->value();
}

//
// Emulate the user dragging the value slider
//
void ConsoleChannel::slotAnimateValueChange(t_value value)
{
	m_valueSlider->setValue(static_cast<int> (KChannelValueMax - value));
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
	// Show the preset menu
	m_menu->exec(e->globalPos());
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

//
// Device console's scene editor activated a scene, which is reflected
// here. So set the value and status to UI.
//
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
			emit changed(m_channel, m_value, m_status);
		}
	}
}
