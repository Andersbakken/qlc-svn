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

#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QtXml>

#include "common/qlcfile.h"

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
	setupUi(this);
	m_slider->setStyle(App::sliderStyle());
	layout()->setMargin(0);
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	m_sliderPressed = false;

	m_busLowLimit = 0;
	m_busHighLimit = 5;
	m_inputUniverse = KInputUniverseInvalid;
	m_inputChannel = KInputChannelInvalid;

	setBus(bus);

	/* Bus connections */
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	connect(Bus::emitter(),	SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	/* Slider connections */
	connect(m_slider, SIGNAL(sliderPressed()),
		this, SLOT(slotSliderPressed()));
	connect(m_slider, SIGNAL(valueChanged(int)),
		this, SLOT(slotSliderValueChanged(int)));
	connect(m_slider, SIGNAL(sliderReleased()),
		this, SLOT(slotSliderReleased()));

	/* Tap button clicks */
	connect(m_tapButton, SIGNAL(clicked()),
		this, SLOT(slotTapButtonClicked()));

	m_time.start();
}


VCDockSlider::~VCDockSlider()
{
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void VCDockSlider::setBus(t_bus_id bus)
{
	QString name;

	Q_ASSERT(bus >= KBusIDMin && bus < KBusCount);

	m_slider->setRange(m_busLowLimit * KFrequency,
			   m_busHighLimit * KFrequency);

	// Set slider value
	m_bus = bus;
	slotBusValueChanged(m_bus, Bus::value(m_bus));

	// Set slider name to the tap button
	name = Bus::name(m_bus);
	if (name.simplified().isEmpty() == true)
		name.sprintf("Bus %.2d", m_bus + 1);

	m_tapButton->setText(name);
}

void VCDockSlider::setBusRange(t_bus_value lo, t_bus_value hi)
{
	m_busLowLimit = lo;
	m_busHighLimit = hi;

	m_slider->setRange(m_busLowLimit * KFrequency,
			   m_busHighLimit * KFrequency);
}

void VCDockSlider::slotBusNameChanged(t_bus_id bus, const QString& name)
{
	Q_UNUSED(name);

	if (bus == m_bus)
		setBus(bus);
}

void VCDockSlider::slotBusValueChanged(t_bus_id bus, t_bus_value value)
{
	if (bus == m_bus && m_sliderPressed == false)
		m_slider->setValue(value);
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCDockSlider::slotSliderPressed()
{
	m_sliderPressed = true;
}

void VCDockSlider::slotSliderValueChanged(int value)
{
	QString num;

	if (m_sliderPressed == true)
		Bus::setValue(m_bus, m_slider->value());

	/* Set value to label */
	num.sprintf("%.2fs", float(value) / float(KFrequency));
	m_valueLabel->setText(num);

	/* Send input feedback */
	if (m_inputUniverse != KInputUniverseInvalid &&
	    m_inputChannel != KInputChannelInvalid)
	{
		if (m_slider->invertedAppearance() == true)
			value = m_slider->maximum() - value;

		float fb = SCALE(float(value), float(m_slider->minimum()),
				 float(m_slider->maximum()), float(0),
				 float(KInputValueMax));

		_app->inputMap()->feedBack(m_inputUniverse, m_inputChannel,
								int(fb));
	}
}

void VCDockSlider::slotSliderReleased()
{
	m_sliderPressed = false;
}

/*****************************************************************************
 * Tap button
 *****************************************************************************/

void VCDockSlider::slotTapButtonClicked()
{
	int t = m_time.elapsed();
	m_sliderPressed = true;
	m_slider->setValue(static_cast<int> (t * 0.001 * KFrequency));
	Bus::tap(m_bus);
	m_sliderPressed = false;
	m_time.restart();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCDockSlider::setInputSource(t_input_universe uni, t_input_channel ch)
{
	if (uni == KInputUniverseInvalid || ch == KInputChannelInvalid)
	{
		/* If either one of the new values is invalid we end up here
		   to disconnect from inputmap and setting both of the values
		   invalid. */
		m_inputUniverse = KInputUniverseInvalid;
		m_inputChannel = KInputChannelInvalid;

		/* Even though we might not be connected, it is safe to do a
		   disconnect in any case. */
		disconnect(_app->inputMap(),
			   SIGNAL(inputValueChanged(t_input_universe,
						    t_input_channel,
						    t_input_value)),
			   this, SLOT(slotInputValueChanged(t_input_universe,
							    t_input_channel,
							    t_input_value)));
	}
	else if (m_inputUniverse == KInputUniverseInvalid ||
		 m_inputChannel == KInputChannelInvalid)
	{
		/* Execution comes here only if both of the new values
		   are valid and the existing values are invalid, in which
		   case a new connection must be made. */
		m_inputUniverse = uni;
		m_inputChannel = ch;

		connect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
	else
	{
		/* Execution comes here only if the current uni & channel are
		 * valid and the new ones are valid as well. So we don't do a
		 * new connection, which would end up in duplicate values.
		 * Just update the new values and get it over with. */
		 m_inputUniverse = uni;
		 m_inputChannel = ch;
	}
}

void VCDockSlider::slotInputValueChanged(t_input_universe universe,
					 t_input_channel channel,
					 t_input_value value)
{
	if (universe == m_inputUniverse && channel == m_inputChannel)
	{
		/* Scale the from input value range to this slider's range */
		float val;
		val = SCALE((float) value, (float) 0, (float) KInputValueMax,
			    (float) m_slider->minimum(),
			    (float) m_slider->maximum());

		m_sliderPressed = true;
		if (m_slider->invertedAppearance() == true)
			m_slider->setValue(m_slider->maximum() - (int) val);
		else
			m_slider->setValue((int) val);
		m_sliderPressed = false;
	}
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCDockSlider::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomNode node;

	t_input_universe uni;
	t_input_channel ch;
	t_bus_value lo;
	t_bus_value hi;
	t_bus_id bus;
	QString str;

	Q_UNUSED(doc);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCDockSlider)
	{
		qWarning("Dock slider node not found!");
		return false;
	}

	/* Accept only fade slider, hold is a thing of the past */
	if (root->attribute(KXMLQLCBusRole) != KXMLQLCBusFade)
		return false;

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCDockSliderBus)
		{
			/* Bus */
			bus = tag.text().toInt();

			/* Low limit */
			str = tag.attribute(KXMLQLCVCDockSliderBusLowLimit);
			lo = str.toInt();

			/* High limit */
			str = tag.attribute(KXMLQLCVCDockSliderBusHighLimit);
			hi = str.toInt();

			setBusRange(lo, hi);
			setBus(bus);
		}
		else if (tag.tagName() == KXMLQLCVCDockSliderInput)
		{
			str = tag.attribute(KXMLQLCVCDockSliderInputUniverse);
			uni = str.toInt();

			str = tag.attribute(KXMLQLCVCDockSliderInputChannel);
			ch = str.toInt();

			setInputSource(uni, ch);
		}
		else
		{
			qDebug() << "Unknown dock slider tag:"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool VCDockSlider::saveXML(QDomDocument* doc, QDomElement* da_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString role;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(da_root != NULL);

	/* Slider entry */
	root = doc->createElement(KXMLQLCVCDockSlider);
	da_root->appendChild(root);

	/* Role */
	root.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);

	/* Bus */
	tag = doc->createElement(KXMLQLCVCDockSliderBus);
	root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_bus));
	tag.appendChild(text);

	/* Bus low limit */
	tag.setAttribute(KXMLQLCVCDockSliderBusLowLimit,
			 QString("%1").arg(busLowLimit()));

	/* Bus high limit */
	tag.setAttribute(KXMLQLCVCDockSliderBusHighLimit,
			 QString("%1").arg(busHighLimit()));

	/* External input */
	if (m_inputUniverse != KInputUniverseInvalid &&
	    m_inputChannel != KInputChannelInvalid)
	{
		tag = doc->createElement(KXMLQLCVCDockSliderInput);
		root.appendChild(tag);
		tag.setAttribute(KXMLQLCVCDockSliderInputUniverse,
				 QString("%1").arg(inputUniverse()));
		tag.setAttribute(KXMLQLCVCDockSliderInputChannel,
				 QString("%1").arg(inputChannel()));
        }

	return true;
}
