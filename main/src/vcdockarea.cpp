/*
  Q Light Controller
  vcdockarea.cpp

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

#include <qlayout.h>
#include <qdom.h>

#include "common/filehandler.h"
#include "vcdockarea.h"
#include "vcdockslider.h"
#include "bus.h"
#include "app.h"

extern App* _app;

VCDockArea::VCDockArea(QWidget* parent) : QFrame(parent, "VCDockArea")
{
}

VCDockArea::~VCDockArea()
{
}

void VCDockArea::init()
{
	t_bus_value min = 0;
	t_bus_value max = 0;
	QString value;

	// Align widgets vertically in the area
	m_layout = new QVBoxLayout(this);

	// Default fade time slider
	m_defaultFadeSlider = new VCDockSlider(this);
	m_defaultFadeSlider->init();
	m_layout->addWidget(m_defaultFadeSlider);
	m_defaultFadeSlider->setBusID(KBusIDDefaultFade);
	m_defaultFadeSlider->setBusRange(0, 5);

	// Default hold time slider
	m_defaultHoldSlider = new VCDockSlider(this);
	m_defaultHoldSlider->init();
	m_layout->addWidget(m_defaultHoldSlider);
	m_defaultHoldSlider->setBusID(KBusIDDefaultHold);
	m_defaultHoldSlider->setBusRange(0, 5);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCDockArea::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	bool visible = false;
	t_bus_id bus = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCDockArea)
	{
		qWarning("Virtual Console Dock Area node not found!");
		return false;
	}
	
	/* Visibility */
	str = root->attribute(KXMLQLCVCDockAreaVisible);
	visible = static_cast<bool> (str.toInt());
	
	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCDockSlider)
		{
			/* Check, which slider is defined in this node */
			bus = tag.attribute(KXMLQLCVCDockSliderBus).toInt();

			/* Only accept buses 0 and 1 */
			if (bus == KBusIDDefaultFade)
				m_defaultFadeSlider->loadXML(doc, &tag);
			else if (bus == KBusIDDefaultHold)
				m_defaultHoldSlider->loadXML(doc, &tag);
			else
				qWarning("Not a default bus ID for a " \
					 "virtual console dock slider: %d",
					 bus);
		}
		else
		{
			qWarning("Unknown dock slider tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	if (visible == true)
		show();
	else
		hide();

	return true;
}

bool VCDockArea::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* Dock Area entry */
	root = doc->createElement(KXMLQLCVCDockArea);
	vc_root->appendChild(root);

	/* Visibility */
	str.setNum(static_cast<int> (isShown()));
	root.setAttribute(KXMLQLCVCDockAreaVisible, str);

	/* Slider entries */
	m_defaultFadeSlider->saveXML(doc, &root);
	m_defaultHoldSlider->saveXML(doc, &root);
}

/*****************************************************************************
 * Event Handlers
 *****************************************************************************/

void VCDockArea::showEvent(QShowEvent* event)
{
	emit visibilityChanged(true);
}

void VCDockArea::hideEvent(QHideEvent* event)
{
	emit visibilityChanged(false);
}
