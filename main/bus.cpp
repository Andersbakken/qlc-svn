/*
  Q Light Controller
  bus.cpp

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
#include <QDebug>
#include <QFile>
#include <QtXml>

#include "function.h"
#include "bus.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/****************************************************************************
 * Initialization
 ****************************************************************************/

t_bus_id Bus::s_nextID ( KBusIDMin );
Bus* Bus::s_busArray ( NULL );
BusEmitter* Bus::s_busEmitter ( NULL );

Bus::Bus()
{
	m_id = s_nextID++;
	m_value = 0;
	m_name = QString::null;
}

Bus::~Bus()
{
}

void Bus::init()
{
	if (s_busArray != NULL)
		delete s_busArray;
	s_busArray = new Bus[KBusCount];

	s_busArray[KBusIDDefaultFade].m_name = "Fade";
	s_busArray[KBusIDDefaultHold].m_name = "Hold";

	if (s_busEmitter != NULL)
		delete s_busEmitter;
	s_busEmitter = new BusEmitter();
}

/****************************************************************************
 * Name
 ****************************************************************************/

const QString& Bus::name(t_bus_id id)
{
	static const QString null;
	if (id >= KBusIDMin && id < KBusCount)
		return s_busArray[id].m_name;
	else
		return null;
}

bool Bus::setName(t_bus_id id, const QString& name)
{
	if (id >= KBusIDMin && id < KBusCount)
	{
		s_busArray[id].m_name = name;
		s_busEmitter->emitNameChanged(id, name);
		_app->doc()->setModified();
		return true;
	}
	else
	{
		return false;
	}
}

/****************************************************************************
 * Value
 ****************************************************************************/

t_bus_value Bus::value(t_bus_id id)
{
	if (id >= KBusIDMin && id < KBusCount)
		return s_busArray[id].m_value;
	else
		return 0;
}

bool Bus::setValue(t_bus_id id, t_bus_value value)
{
	if (id >= KBusIDMin && id < KBusCount)
	{
		s_busArray[id].m_value = value;
		s_busEmitter->emitValueChanged(id, value);

		return true;
	}
	else
	{
		return false;
	}
}

/****************************************************************************
 * Tap
 ****************************************************************************/

bool Bus::tap(t_bus_id id)
{
	if (id >= KBusIDMin && id < KBusCount)
	{
		s_busEmitter->emitTapped(id);
		return true;
	}
	else
	{
		return false;
	}
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Bus::loadXML(QDomDocument*, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	t_bus_id id;
	QString name;
	t_bus_value value = 0;
	bool retval = false;

	Q_ASSERT(root != NULL);

	if (root->tagName() == KXMLQLCBus)
	{
		id = root->attribute(KXMLQLCBusID).toInt();

		node = root->firstChild();
		while (node.isNull() == false)
		{
			tag = node.toElement();

			if (tag.tagName() == KXMLQLCBusName)
				name = tag.text();
			else if (tag.tagName() == KXMLQLCBusValue)
				value = tag.text().toULong();
			else
				qDebug() << "Unknown Bus tag:" << tag.tagName();

			node = node.nextSibling();
		}

		Bus::setName(id, name);
		Bus::setValue(id, value);

		retval = true;
	}
	else
	{
		qDebug() << "Bus node not found in file!";
		retval = false;
	}

	return retval;
}

bool Bus::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	for (t_bus_id i = KBusIDMin; i < KBusCount; i++)
	{
		/* Bus entry */
		root = doc->createElement(KXMLQLCBus);
		root.setAttribute(KXMLQLCBusID, str.setNum(i));
		wksp_root->appendChild(root);

		/* Name */
		tag = doc->createElement(KXMLQLCBusName);
		root.appendChild(tag);
		text = doc->createTextNode(Bus::name(i));
		tag.appendChild(text);

		/* Value */
		tag = doc->createElement(KXMLQLCBusValue);
		root.appendChild(tag);
		text = doc->createTextNode(str.setNum(Bus::value(i)));
		tag.appendChild(text);
	}

	return true;
}
