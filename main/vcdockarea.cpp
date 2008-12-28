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

#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "common/qlcfile.h"
#include "vcdockslider.h"
#include "vcdockarea.h"
#include "bus.h"
#include "app.h"

extern App* _app;

VCDockArea::VCDockArea(QWidget* parent) : QFrame(parent)
{
	// Align widgets vertically in the area
	new QVBoxLayout(this);
	layout()->setMargin(0);

	// Default fade time slider
	m_defaultFadeSlider = new VCDockSlider(this, KBusIDDefaultFade);
	layout()->addWidget(m_defaultFadeSlider);

	// Default hold time slider
	m_defaultHoldSlider = new VCDockSlider(this, KBusIDDefaultHold);
	layout()->addWidget(m_defaultHoldSlider);
}

VCDockArea::~VCDockArea()
{
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

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCDockArea)
	{
		qDebug() << "Virtual Console Dock Area node not found!";
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
			str = tag.attribute(KXMLQLCBusRole);

			/* Only accept buses 0 and 1 */
			if (str == KXMLQLCBusFade)
				m_defaultFadeSlider->loadXML(doc, &tag);
			else if (str == KXMLQLCBusHold)
				m_defaultHoldSlider->loadXML(doc, &tag);
			else
				qDebug() << "Cannot bind default sliders to"
				         << "other than Fade/Hold buses.";
		}
		else
		{
			qDebug() << "Unknown dock slider tag:" << tag.tagName();
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
	str.setNum(static_cast<int> (isVisible()));
	root.setAttribute(KXMLQLCVCDockAreaVisible, str);

	/* Slider entries */
	m_defaultFadeSlider->saveXML(doc, &root);
	m_defaultHoldSlider->saveXML(doc, &root);

	return true;
}

/*****************************************************************************
 * Event Handlers
 *****************************************************************************/

void VCDockArea::showEvent(QShowEvent*)
{
	emit visibilityChanged(true);
}

void VCDockArea::hideEvent(QHideEvent*)
{
	emit visibilityChanged(false);
}
