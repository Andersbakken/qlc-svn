/*
  Q Light Controller
  qlcphysical.cpp

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

#include <QString>
#include <QtXml>

#include "qlcphysical.h"

QLCPhysical::QLCPhysical()
{
	/* Initialize only integer values since QStrings are null by default */
	m_bulbLumens = 0;
	m_bulbColourTemperature = 0;

	m_weight = 0;
	m_width = 0;
	m_height = 0;
	m_depth = 0;

	m_lensName = "Other";
	m_lensDegreesMin = 0;
	m_lensDegreesMax = 0;

	m_focusType = "Fixed";
	m_focusPanMax = 0;
	m_focusTiltMax = 0;
}

QLCPhysical::~QLCPhysical()
{
}

QLCPhysical& QLCPhysical::operator=(const QLCPhysical& physical)
{
	if (this != &physical)
	{
		m_bulbType = physical.bulbType();
		m_bulbLumens = physical.bulbLumens();
		m_bulbColourTemperature = physical.bulbColourTemperature();

		m_weight = physical.weight();
		m_width = physical.width();
		m_height = physical.height();
		m_depth = physical.depth();

		m_lensName = physical.lensName();
		m_lensDegreesMin = physical.lensDegreesMin();
		m_lensDegreesMax = physical.lensDegreesMax();

		m_focusType = physical.focusType();
		m_focusPanMax = physical.focusPanMax();
		m_focusTiltMax = physical.focusTiltMax();
	}

	return *this;
}

bool QLCPhysical::loadXML(const QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	QString ch;

	if (root->tagName() != KXMLQLCPhysical)
		return false;

	/* Subtags */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCPhysicalBulb)
		{
			m_bulbType = tag.attribute(KXMLQLCPhysicalBulbType);
			m_bulbLumens = tag.attribute(KXMLQLCPhysicalBulbLumens).toInt();
			m_bulbColourTemperature = tag.attribute(KXMLQLCPhysicalBulbColourTemperature).toInt();
		}
		else if (tag.tagName() == KXMLQLCPhysicalDimensions)
		{
			m_weight = tag.attribute(KXMLQLCPhysicalDimensionsWeight).toInt();
			m_width = tag.attribute(KXMLQLCPhysicalDimensionsWidth).toInt();
			m_height = tag.attribute(KXMLQLCPhysicalDimensionsHeight).toInt();
			m_depth = tag.attribute(KXMLQLCPhysicalDimensionsDepth).toInt();
		}
		else if (tag.tagName() == KXMLQLCPhysicalLens)
		{
			m_lensName = tag.attribute(KXMLQLCPhysicalLensName);
			m_lensDegreesMin = tag.attribute(KXMLQLCPhysicalLensDegreesMin).toInt();
			m_lensDegreesMax = tag.attribute(KXMLQLCPhysicalLensDegreesMax).toInt();
		}
		else if (tag.tagName() == KXMLQLCPhysicalFocus)
		{
			m_focusType = tag.attribute(KXMLQLCPhysicalFocusType);
			m_focusPanMax = tag.attribute(KXMLQLCPhysicalFocusPanMax).toInt();
			m_focusTiltMax = tag.attribute(KXMLQLCPhysicalFocusTiltMax).toInt();
		}
		else
		{
			qDebug() << "Unknown Physical tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool QLCPhysical::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomElement subtag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* Physical entry */
	tag = doc->createElement(KXMLQLCPhysical);
	root->appendChild(tag);

	/* Bulb */
	subtag = doc->createElement(KXMLQLCPhysicalBulb);
	subtag.setAttribute(KXMLQLCPhysicalBulbType, m_bulbType);
	subtag.setAttribute(KXMLQLCPhysicalBulbLumens, m_bulbLumens);
	subtag.setAttribute(KXMLQLCPhysicalBulbColourTemperature, m_bulbColourTemperature);
	tag.appendChild(subtag);

	/* Dimensions */
	subtag = doc->createElement(KXMLQLCPhysicalDimensions);
	subtag.setAttribute(KXMLQLCPhysicalDimensionsWeight, m_weight);
	subtag.setAttribute(KXMLQLCPhysicalDimensionsWidth, m_width);
	subtag.setAttribute(KXMLQLCPhysicalDimensionsHeight, m_height);
	subtag.setAttribute(KXMLQLCPhysicalDimensionsDepth, m_depth);
	tag.appendChild(subtag);

	/* Lens */
	subtag = doc->createElement(KXMLQLCPhysicalLens);
	subtag.setAttribute(KXMLQLCPhysicalLensName, m_lensName);
	subtag.setAttribute(KXMLQLCPhysicalLensDegreesMin, m_lensDegreesMin);
	subtag.setAttribute(KXMLQLCPhysicalLensDegreesMax, m_lensDegreesMax);
	tag.appendChild(subtag);

	/* Focus */
	subtag = doc->createElement(KXMLQLCPhysicalFocus);
	subtag.setAttribute(KXMLQLCPhysicalFocusType, m_focusType);
	subtag.setAttribute(KXMLQLCPhysicalFocusPanMax, m_focusPanMax);
	subtag.setAttribute(KXMLQLCPhysicalFocusTiltMax, m_focusTiltMax);
	tag.appendChild(subtag);

	return true;
}
