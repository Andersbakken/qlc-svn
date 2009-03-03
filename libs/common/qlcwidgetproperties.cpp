/*
  Q Light Controller
  qlcwidgetproperties.cpp

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

#include <QWidget>
#include <QtXml>

#include "qlcwidgetproperties.h"

QLCWidgetProperties::QLCWidgetProperties()
{
	m_state = Qt::WindowNoState;
	m_visible = false;
	m_x = 0;
	m_y = 0;
	m_width = 0;
	m_height = 0;
}

QLCWidgetProperties::QLCWidgetProperties(const QLCWidgetProperties& properties)
{
	*this = properties;
}

QLCWidgetProperties::~QLCWidgetProperties()
{
}

QLCWidgetProperties& QLCWidgetProperties::operator=(
					const QLCWidgetProperties& properties)
{
	if (this != &properties)
	{
		m_state = properties.m_state;
		m_visible = properties.m_visible;
		m_x = properties.m_x;
		m_y = properties.m_y;
		m_width = properties.m_width;
		m_height = properties.m_height;
	}

	return *this;
}

void QLCWidgetProperties::store(QWidget* widget)
{
	Q_ASSERT(widget != NULL);
	m_state = widget->windowState();
	m_visible = widget->isVisible();
	m_x = widget->x();
	m_y = widget->y();
	m_width = widget->width();
	m_height = widget->height();
}

bool QLCWidgetProperties::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomNode node;

	Q_UNUSED(doc);
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCWidgetProperties &&
	    root->tagName() != QString("Properties")) /* Legacy */
	{
		qWarning("Widget Properties node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCWidgetX)
			m_x = tag.text().toInt();
		else if (tag.tagName() == KXMLQLCWidgetY)
			m_y = tag.text().toInt();
		else if (tag.tagName() == KXMLQLCWidgetWidth)
			m_width = tag.text().toInt();
		else if (tag.tagName() == KXMLQLCWidgetHeight)
			m_height = tag.text().toInt();
		else if (tag.tagName() == KXMLQLCWidgetState)
			m_state = Qt::WindowState(tag.text().toInt());
		else if (tag.tagName() == KXMLQLCWidgetVisible)
			m_visible = bool(tag.text().toInt());
		else
			qDebug() << "Unknown widget tag:" << tag.tagName();

		node = node.nextSibling();
	}

	return true;
}

bool QLCWidgetProperties::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement prop_root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* Widget Properties entry */
	prop_root = doc->createElement(KXMLQLCWidgetProperties);
	root->appendChild(prop_root);

	/* X */
	tag = doc->createElement(KXMLQLCWidgetX);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_x));
	tag.appendChild(text);

	/* Y */
	tag = doc->createElement(KXMLQLCWidgetY);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_y));
	tag.appendChild(text);

	/* W */
	tag = doc->createElement(KXMLQLCWidgetWidth);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_width));
	tag.appendChild(text);

	/* H */
	tag = doc->createElement(KXMLQLCWidgetHeight);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_height));
	tag.appendChild(text);

	/* Window state */
	tag = doc->createElement(KXMLQLCWidgetState);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_state));
	tag.appendChild(text);

	/* Visible state */
	tag = doc->createElement(KXMLQLCWidgetVisible);
	prop_root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(m_visible));
	tag.appendChild(text);

	return true;
}

