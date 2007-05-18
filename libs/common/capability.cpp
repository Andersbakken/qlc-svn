/*
  Q Light Controller
  capability.cpp
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#include <qstring.h>
#include <qfile.h>
#include <qdom.h>

#include "common/types.h"
#include "capability.h"

Capability::Capability()
{
	m_lo = KChannelValueMin;
	m_hi = KChannelValueMax;
	m_name = QString::null;
}

Capability::Capability(Capability* cap)
{
	m_lo = cap->lo();
	m_hi = cap->hi();
	m_name = QString(cap->name());
}

Capability::~Capability()
{
}

void Capability::createInfo(QPtrList <QString> &list)
{
	// Parse one capability from string list
	// e.g. 0-255 = Foobar
	QString t;
	QString* s = list.next();
	int i = s->find(QString("-"));
	if (i > -1)
	{
		// Low limit
		setLo(s->left(i).toInt());
		// Hight limit
		setHi(s->mid(i+1).toInt());
	}
	else
	{
		// Just one value
		setLo(s->toInt());
		setHi(s->toInt());
	}
  
	// Capability name ("Foobar")
	setName(*(list.next()));
}

void Capability::saveToFile(QFile & file)
{
	QString s;
	QString t;

	t.setNum((int) m_lo);
	s = t + QString("-");

	t.setNum((int) m_hi);
	s += t + QString(" = ") + m_name + QString("\n");

	file.writeBlock((const char*) s, s.length());
}

int Capability::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomText text;
	QString str;

	/* Capability entry */
	tag = doc->createElement(KXMLCapability);
	root->appendChild(tag);

	/* Low limit attribute */
	str.setNum(m_lo);
	tag.setAttribute(KXMLCapabilityLow, str);

	/* High limit attribute */
	str.setNum(m_hi);
	tag.setAttribute(KXMLCapabilityHigh, str);

	/* Name value */
	text = doc->createTextNode(m_name);
	tag.appendChild(text);

	return 0;
}

bool Capability::loadXML(QDomElement* root)
{
	QString str;

	ASSERT(root != NULL);

	/* Get low limit attribute (critical) */
	str = root->attribute(KXMLCapabilityLow);
	if (str == QString::null)
	{
		qWarning("Capability has no low limit");
		return false;
	}
	else
	{
		setLo(str.toInt());
	}

	/* Get high limit attribute (critical) */
	str = root->attribute(KXMLCapabilityHigh);
	if (str == QString::null)
	{
		qWarning("Capability has no high limit");
		return false;
	}
	else
	{
		setHi(str.toInt());
	}

	/* Capability name */
	setName(root->text());

	return true;
}

Capability* Capability::createFromElement(QDomElement* root)
{
	Capability* cap = NULL;

	ASSERT(root != NULL);

	/* Check that the node is a Capability node */
	if (root->tagName() != KXMLCapability)
		return NULL;

	/* Create a new Capability and try to load its contents */
	cap = new Capability();
	if (cap->loadXML(root) == false)
	{
		delete cap;
		cap = NULL;
	}

	return cap;
}
