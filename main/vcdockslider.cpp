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
#include "app.h"
#include "doc.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCDockSlider::VCDockSlider(QWidget* parent, t_bus_id bus) : QFrame(parent)
{
	m_updateOnly = false;
	m_busLowLimit = 0;
	m_busHighLimit = 5;

	setupUi(this);
	layout()->setMargin(0);

	setBusID(bus);

	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	// Receive bus name change signals
	connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
	
	// Receive bus value change signals
	connect(Bus::emitter(),	SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	/* Slider dragging */
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
 * Bus
 *****************************************************************************/

void VCDockSlider::setBusID(t_bus_id id)
{
	QString name;
	
	Q_ASSERT(id >= KBusIDMin && id < KBusCount);

	m_slider->setRange(m_busLowLimit * KFrequency,
			   m_busHighLimit * KFrequency);

	// Set slider value
	m_busID = id;
	slotBusValueChanged(m_busID, Bus::value(m_busID));

	// Set slider name to the tap button
	name = Bus::name(m_busID);
	if (name.simplified().isEmpty() == true)
		name.sprintf("Bus %.2d", m_busID + 1);

	m_tapButton->setText(name);
}

void VCDockSlider::setBusRange(t_bus_value lo, t_bus_value hi)
{
	m_busLowLimit = lo;
	m_busHighLimit = hi;

	m_slider->setRange(m_busLowLimit * KFrequency,
			   m_busHighLimit * KFrequency);
}

void VCDockSlider::busRange(t_bus_value &lo, t_bus_value &hi)
{
	lo = m_busLowLimit;
	hi = m_busHighLimit;
}

void VCDockSlider::slotBusNameChanged(t_bus_id id, const QString&)
{
	if (id == m_busID)
		setBusID(id);
}

void VCDockSlider::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	m_updateOnly = true;
	if (id == m_busID)
		m_slider->setValue(value);
	m_updateOnly = false;
}

/*****************************************************************************
 * Slider dragging & tap button clicks
 *****************************************************************************/

void VCDockSlider::slotSliderValueChanged(int value)
{
	QString num;

	if (m_updateOnly == false)
	{
		if (Bus::setValue(m_busID, m_slider->value()) == false)
		{
			m_valueLabel->setText("ERROR");
			return;
		}
	}

	num.sprintf("%.2fs", ((float) value / (float) KFrequency));
	m_valueLabel->setText(num);
	
	// int range = m_busHighLimit - m_busLowLimit;
	// float f = ((float) value / (float) KFrequency);
	// _app->inputPlugin()->feedBack(1, m_inputChannel,
	//		      127 - int((f * 127) / range));
}

void VCDockSlider::slotTapButtonClicked()
{
	int t = m_time.elapsed();
	m_slider->setValue(static_cast<int> (t * 0.001 * KFrequency));
	m_time.restart();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCDockSlider::loadXML(QDomDocument*, QDomElement* root)
{
	t_bus_value lo = 0;
	t_bus_value hi = 0;
	t_bus_id id = 0;
	QString str;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCDockSlider)
	{
		qWarning("Dock slider node not found!");
		return false;
	}

	/* Bus ID */
	str = root->attribute(KXMLQLCVCDockSliderBus);
	id = str.toInt();

	/* Bus low limit */
	str = root->attribute(KXMLQLCVCDockSliderBusLowLimit);
	lo = str.toInt();

	/* Bus high limit */
	str = root->attribute(KXMLQLCVCDockSliderBusHighLimit);
	hi = str.toInt();

	setBusRange(lo, hi);
	setBusID(id);

	return true;
}

bool VCDockSlider::saveXML(QDomDocument* doc, QDomElement* da_root)
{
	QDomElement root;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(da_root != NULL);

	/* Slider entry */
	root = doc->createElement(KXMLQLCVCDockSlider);
	da_root->appendChild(root);

	/* Bus ID */
	str.setNum(busID());
	root.setAttribute(KXMLQLCVCDockSliderBus, str);

	/* Bus low limit */
	str.setNum(m_busLowLimit);
	root.setAttribute(KXMLQLCVCDockSliderBusLowLimit, str);

	/* Bus high limit */
	str.setNum(m_busHighLimit);
	root.setAttribute(KXMLQLCVCDockSliderBusHighLimit, str);

	return true;
}
