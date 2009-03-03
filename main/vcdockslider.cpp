/*
  Q Light Controller
  vcdockslider.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QtXml>

#include "common/qlcfile.h"

#include "virtualconsole.h"
#include "vcproperties.h"
#include "vcdockslider.h"
#include "inputmap.h"
#include "app.h"
#include "doc.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCDockSlider::VCDockSlider(QWidget* parent, t_bus_id bus) : QFrame(parent)
{
	Q_ASSERT(bus == KBusIDDefaultFade || bus == KBusIDDefaultHold);

	setupUi(this);
	m_slider->setStyle(App::saneStyle());

	m_bus = bus;
	slotBusValueChanged(m_bus, Bus::value(m_bus));
	slotBusNameChanged(m_bus, Bus::name(m_bus));
	
	/* Bus connections */
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	connect(Bus::emitter(),	SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	/* External input connection */
	connect(_app->inputMap(), SIGNAL(inputValueChanged(t_input_universe,
							   t_input_channel,
							   t_input_value)),
		this, SLOT(slotInputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)));

	/* Slider connections */
	connect(m_slider, SIGNAL(valueChanged(int)),
		this, SLOT(slotSliderValueChanged(int)));

	/* Tap button clicks */
	connect(m_tapButton, SIGNAL(clicked()),
		this, SLOT(slotTapButtonClicked()));

	m_time.start();
}


VCDockSlider::~VCDockSlider()
{
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCDockSlider::refreshProperties()
{
	t_bus_value low = 0;
	t_bus_value high = 10;

	if (m_bus == KBusIDDefaultFade)
	{
		low = VirtualConsole::properties().fadeLowLimit();
		high = VirtualConsole::properties().fadeHighLimit();
	}
	else
	{
		low = VirtualConsole::properties().holdLowLimit();
		high = VirtualConsole::properties().holdHighLimit();
	}

	Q_ASSERT(m_slider != NULL);
	m_slider->setRange(low * KFrequency, high * KFrequency);
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void VCDockSlider::slotBusNameChanged(t_bus_id bus, const QString& name)
{
	if (bus == m_bus)
	{
		QString str(name);
		if (str.simplified().isEmpty() == true)
			str.sprintf("Bus %.2d", m_bus + 1);
		m_tapButton->setText(str);
	}
}

void VCDockSlider::slotBusValueChanged(t_bus_id bus, t_bus_value value)
{
	if (bus == m_bus && m_slider->isSliderDown() == false)
		m_slider->setValue(value);
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCDockSlider::slotSliderValueChanged(int value)
{
	t_input_universe uni;
	t_input_channel ch;
	QString num;

	/* Set changed value to bus */
	Bus::setValue(m_bus, m_slider->value());

	/* Set value to label */
	num.sprintf("%.2fs", float(value) / float(KFrequency));
	m_valueLabel->setText(num);

	/* Find out this slider's input universe & channel */
	if (m_bus == KBusIDDefaultFade)
	{
		uni = VirtualConsole::properties().fadeInputUniverse();
		ch = VirtualConsole::properties().fadeInputChannel();
	}
	else
	{
		uni = VirtualConsole::properties().holdInputUniverse();
		ch = VirtualConsole::properties().holdInputChannel();
	}

	qDebug() << "paska" << uni << ch;

	/* Send input feedback */
	if (uni != KInputUniverseInvalid && ch != KInputChannelInvalid)
	{
		if (m_slider->invertedAppearance() == true)
			value = m_slider->maximum() - value;

		float fb = SCALE(float(value), float(m_slider->minimum()),
				 float(m_slider->maximum()), float(0),
				 float(KInputValueMax));

		_app->inputMap()->feedBack(uni, ch, int(fb));
	}
}

/*****************************************************************************
 * Tap button
 *****************************************************************************/

void VCDockSlider::slotTapButtonClicked()
{
	int t = m_time.elapsed();
	m_slider->setValue(static_cast<int> (t * 0.001 * KFrequency));
	Bus::tap(m_bus);
	m_time.restart();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCDockSlider::slotInputValueChanged(t_input_universe universe,
					 t_input_channel channel,
					 t_input_value value)
{
	t_input_universe uni;
	t_input_channel ch;

	/* Find out this slider's input source */
	if (m_bus == KBusIDDefaultFade)
	{
		uni = VirtualConsole::properties().fadeInputUniverse();
		ch = VirtualConsole::properties().fadeInputChannel();
	}
	else
	{
		uni = VirtualConsole::properties().holdInputUniverse();
		ch = VirtualConsole::properties().holdInputChannel();
	}

	if (universe == uni && channel == ch)
	{
		/* Scale the from input value range to this slider's range */
		float val;
		val = SCALE((float) value, (float) 0, (float) KInputValueMax,
			    (float) m_slider->minimum(),
			    (float) m_slider->maximum());

		if (m_slider->invertedAppearance() == true)
			m_slider->setValue(m_slider->maximum() - (int) val);
		else
			m_slider->setValue((int) val);
	}
}
