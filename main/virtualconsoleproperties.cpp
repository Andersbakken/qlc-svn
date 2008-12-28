/*
  Q Light Controller
  virtualconsoleproperties.cpp

  Copyright (c) Heikki Junnila

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

#include <QCheckBox>
#include <QSpinBox>

#include "common/qlcinputdevice.h"

#include "virtualconsoleproperties.h"
#include "selectinputchannel.h"
#include "virtualconsole.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

VirtualConsoleProperties::VirtualConsoleProperties(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	connect(m_chooseFadeInputButton, SIGNAL(clicked()),
		this, SLOT(slotChooseFadeInputClicked()));
	connect(m_chooseHoldInputButton, SIGNAL(clicked()),
		this, SLOT(slotChooseHoldInputClicked()));

#if defined(__APPLE__) || defined (WIN32)
	m_hardwareTimerRadio->setEnabled(false);
	m_softwareTimerRadio->setChecked(true);
	m_hardwareTimerRadio->setToolTip(tr("Not available"));
#endif
}

VirtualConsoleProperties::~VirtualConsoleProperties()
{
}

/*****************************************************************************
 * Grid
 *****************************************************************************/

void VirtualConsoleProperties::setGrid(bool enabled, int x, int y)
{
	m_gridGroup->setChecked(enabled);
	m_gridXSpin->setValue(x);
	m_gridYSpin->setValue(y);
}

bool VirtualConsoleProperties::isGridEnabled()
{
	return m_gridGroup->isChecked();
}

int VirtualConsoleProperties::gridX()
{
	return m_gridXSpin->value();
}

int VirtualConsoleProperties::gridY()
{
	return m_gridYSpin->value();
}

/*****************************************************************************
 * Key repeat
 *****************************************************************************/

void VirtualConsoleProperties::setKeyRepeatOff(bool set)
{
	m_keyRepeatOffCheck->setChecked(set);
}

bool VirtualConsoleProperties::isKeyRepeatOff()
{
	return m_keyRepeatOffCheck->isChecked();
}

/*****************************************************************************
 * Grab keyboard
 *****************************************************************************/

void VirtualConsoleProperties::setGrabKeyboard(bool set)
{
	m_grabKeyboardCheck->setChecked(set);
}

bool VirtualConsoleProperties::isGrabKeyboard()
{
	return m_grabKeyboardCheck->isChecked();
}

/*****************************************************************************
 * Slider value ranges
 *****************************************************************************/

void VirtualConsoleProperties::setFadeLimits(t_bus_value low, t_bus_value high)
{
	m_fadeLowSpin->setValue(low);
	m_fadeHighSpin->setValue(high);
}

t_bus_value VirtualConsoleProperties::fadeLowLimit()
{
	return m_fadeLowSpin->value();
}

t_bus_value VirtualConsoleProperties::fadeHighLimit()
{
	return m_fadeHighSpin->value();
}

void VirtualConsoleProperties::setHoldLimits(t_bus_value low, t_bus_value high)
{
	m_holdLowSpin->setValue(low);
	m_holdHighSpin->setValue(high);
}

t_bus_value VirtualConsoleProperties::holdLowLimit()
{
	return m_holdLowSpin->value();
}

t_bus_value VirtualConsoleProperties::holdHighLimit()
{
	return m_holdHighSpin->value();
}

/*****************************************************************************
 * Slider external input
 *****************************************************************************/

void VirtualConsoleProperties::setFadeInputSource(t_input_universe uni,
						  t_input_channel ch)
{
	m_fadeInputUniverse = uni;
	m_fadeInputChannel = ch;

	updateFadeInputSource();
}

t_input_universe VirtualConsoleProperties::fadeInputUniverse()
{
	return m_fadeInputUniverse;
}

t_input_channel VirtualConsoleProperties::fadeInputChannel()
{
	return m_fadeInputChannel;
}

void VirtualConsoleProperties::setHoldInputSource(t_input_universe uni,
						  t_input_channel ch)
{
	m_holdInputUniverse = uni;
	m_holdInputChannel = ch;

	updateHoldInputSource();
}

t_input_universe VirtualConsoleProperties::holdInputUniverse()
{
	return m_holdInputUniverse;
}

t_input_channel VirtualConsoleProperties::holdInputChannel()
{
	return m_holdInputChannel;
}

void VirtualConsoleProperties::slotChooseFadeInputClicked()
{
	SelectInputChannel sic(this);
	if (sic.exec() == QDialog::Accepted)
	{
		m_fadeInputUniverse = sic.universe();
		m_fadeInputChannel = sic.channel();

		updateFadeInputSource();
	}
}

void VirtualConsoleProperties::slotChooseHoldInputClicked()
{
	SelectInputChannel sic(this);
	if (sic.exec() == QDialog::Accepted)
	{
		m_holdInputUniverse = sic.universe();
		m_holdInputChannel = sic.channel();

		updateHoldInputSource();
	}
}

void VirtualConsoleProperties::updateFadeInputSource()
{
	QLCInputDevice* dev;
	InputPatch* patch;
	QString uniName;
	QString chName;

	if (m_fadeInputUniverse == KInputUniverseInvalid ||
	    m_fadeInputChannel == KInputChannelInvalid)
	{
		/* Nothing selected for input universe and/or channel */
		uniName = KInputNone;
		chName = KInputNone;
	}
	else
	{
		patch = _app->inputMap()->patch(m_fadeInputUniverse);
		if (patch == NULL || patch->plugin() == NULL)
		{
			/* There is no patch for the given universe */
			uniName = KInputNone;
			chName = KInputNone;
		}
		else
		{
			dev = patch->device();
			if (dev == NULL)
			{
				/* There is no device. Display plugin
				   name and channel number. Boring. */
				uniName = patch->plugin()->name();
				chName = tr("%1: Unknown")
						.arg(m_fadeInputChannel + 1);
			}
			else
			{
				QString name;

				/* Display device name for universe */
				uniName = QString("%1: %2")
						.arg(m_fadeInputUniverse + 1)
						.arg(dev->name());

				/* User can input the channel number by hand,
				   so put something rational to the channel
				   name in those cases as well. */
				name = dev->channelName(m_fadeInputChannel);
				if (name == QString::null)
					name = tr("Unknown");

				/* Display channel name */
				chName = QString("%1: %2")
					.arg(m_fadeInputChannel + 1).arg(name);
			}
		}
	}

	/* Display the gathered information */
	m_fadeInputUniverseEdit->setText(uniName);
	m_fadeInputChannelEdit->setText(chName);
}

void VirtualConsoleProperties::updateHoldInputSource()
{
	QLCInputDevice* dev;
	InputPatch* patch;
	QString uniName;
	QString chName;

	if (m_holdInputUniverse == KInputUniverseInvalid ||
	    m_holdInputChannel == KInputChannelInvalid)
	{
		/* Nothing selected for input universe and/or channel */
		uniName = KInputNone;
		chName = KInputNone;
	}
	else
	{
		patch = _app->inputMap()->patch(m_holdInputUniverse);
		if (patch == NULL || patch->plugin() == NULL)
		{
			/* There is no patch for the given universe */
			uniName = KInputNone;
			chName = KInputNone;
		}
		else
		{
			dev = patch->device();
			if (dev == NULL)
			{
				/* There is no device. Display plugin
				   name and channel number. Boring. */
				uniName = patch->plugin()->name();
				chName = tr("%1: Unknown")
						.arg(m_holdInputChannel + 1);
			}
			else
			{
				QString name;

				/* Display device name for universe */
				uniName = QString("%1: %2")
						.arg(m_holdInputUniverse + 1)
						.arg(dev->name());

				/* User can input the channel number by hand,
				   so put something rational to the channel
				   name in those cases as well. */
				name = dev->channelName(m_holdInputChannel);
				if (name == QString::null)
					name = tr("Unknown");

				/* Display channel name */
				chName = QString("%1: %2")
					.arg(m_holdInputChannel + 1).arg(name);
			}
		}
	}

	/* Display the gathered information */
	m_holdInputUniverseEdit->setText(uniName);
	m_holdInputChannelEdit->setText(chName);
}

/*****************************************************************************
 * Timer type
 *****************************************************************************/

void VirtualConsoleProperties::setTimerType(FunctionConsumer::TimerType type)
{
	if (type == FunctionConsumer::RTCTimer)
		m_hardwareTimerRadio->setChecked(true);
	else
		m_softwareTimerRadio->setChecked(true);
}

FunctionConsumer::TimerType VirtualConsoleProperties::timerType()
{
	if (m_hardwareTimerRadio->isChecked() == true)
		return FunctionConsumer::RTCTimer;
	else
		return FunctionConsumer::NanoSleepTimer;
}
